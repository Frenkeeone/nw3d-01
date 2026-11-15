#include "bambu.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "mbedtls/base64.h"

namespace bambu {

static const char* api(uint8_t region) { return region ? "https://api.bambulab.cn" : "https://api.bambulab.com"; }
static const char* web(uint8_t region) { return region ? "https://bambulab.cn" : "https://bambulab.com"; }

struct Resp { int code = 0; String body; String cookie; };

static Resp request(const char* method, const String& url, const String& body, const String& token) {
  Resp r;
  WiFiClientSecure c;
  c.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  http.setReuse(false);
  if (!http.begin(c, url)) { r.code = -100; return r; }
  http.setUserAgent("bambu_network_agent/01.09.05.01");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.addHeader("X-BBL-Client-Type", "slicer");
  http.addHeader("X-BBL-Client-Name", "OrcaSlicer");
  http.addHeader("X-BBL-Client-Version", "01.09.05.51");
  http.addHeader("X-BBL-Language", "en-US");
  http.addHeader("X-BBL-OS-Type", "linux");
  if (token.length()) http.addHeader("Authorization", "Bearer " + token);
  const char* keep[] = {"Set-Cookie"};
  http.collectHeaders(keep, 1);
  r.code = strcmp(method, "POST") == 0 ? http.POST(body) : http.GET();
  if (r.code > 0) r.body = http.getString();
  r.cookie = http.header("Set-Cookie");
  http.end();
  return r;
}

static bool looksLikeHtml(const String& b) {
  String t = b;
  t.trim();
  return t.startsWith("<") || b.indexOf("cloudflare") >= 0;
}

static Login parseLogin(const Resp& r) {
  Login l;
  if (r.code <= 0) { l.error = "network error " + String(r.code); return l; }
  if (looksLikeHtml(r.body)) { l.error = "server blocked the request (HTTP " + String(r.code) + "), try again later"; return l; }
  JsonDocument d;
  if (deserializeJson(d, r.body)) { l.error = "unexpected reply (HTTP " + String(r.code) + ")"; return l; }
  String tok = d["accessToken"] | "";
  if (tok.length()) { l.kind = Login::Ok; l.token = tok; return l; }
  String type = d["loginType"] | "";
  if (type == "verifyCode") { l.kind = Login::NeedCode; return l; }
  if (type == "tfa") { l.kind = Login::NeedTfa; l.tfaKey = d["tfaKey"] | ""; return l; }
  String err = d["error"] | "";
  if (err.isEmpty()) err = d["message"] | "";
  if (err.isEmpty()) err = "login failed (HTTP " + String(r.code) + ")";
  l.error = err;
  return l;
}

Login login(const String& email, const String& password, uint8_t region) {
  JsonDocument d;
  d["account"] = email;
  d["password"] = password;
  d["apiError"] = "";
  String body;
  serializeJson(d, body);
  return parseLogin(request("POST", String(api(region)) + "/v1/user-service/user/login", body, ""));
}

bool sendCode(const String& email, uint8_t region) {
  JsonDocument d;
  d["email"] = email;
  d["type"] = "codeLogin";
  String body;
  serializeJson(d, body);
  Resp r = request("POST", String(api(region)) + "/v1/user-service/user/sendemail/code", body, "");
  return r.code >= 200 && r.code < 300;
}

Login loginCode(const String& email, const String& code, uint8_t region) {
  JsonDocument d;
  d["account"] = email;
  d["code"] = code;
  String body;
  serializeJson(d, body);
  return parseLogin(request("POST", String(api(region)) + "/v1/user-service/user/login", body, ""));
}

Login loginTfa(const String& tfaKey, const String& code, uint8_t region) {
  JsonDocument d;
  d["tfaKey"] = tfaKey;
  d["tfaCode"] = code;
  String body;
  serializeJson(d, body);
  Resp r = request("POST", String(web(region)) + "/api/sign-in/tfa", body, "");
  // token prichazi v cookie "token=..."
  int i = r.cookie.indexOf("token=");
  if (i >= 0) {
    int e = r.cookie.indexOf(';', i);
    Login l;
    l.kind = Login::Ok;
    l.token = r.cookie.substring(i + 6, e < 0 ? r.cookie.length() : e);
    if (l.token.length() > 20) return l;
  }
  return parseLogin(r);
}

// Token byva JWT - v jeho prostredni casti je "username": "u_123..."
static String userFromJwt(const String& token) {
  int a = token.indexOf('.');
  int b = a < 0 ? -1 : token.indexOf('.', a + 1);
  if (a < 0 || b < 0) return "";
  String p = token.substring(a + 1, b);
  p.replace('-', '+');
  p.replace('_', '/');
  while (p.length() % 4) p += '=';
  size_t outLen = 0;
  size_t cap = p.length() * 3 / 4 + 4;
  uint8_t* out = (uint8_t*)malloc(cap);
  if (!out) return "";
  String res;
  if (mbedtls_base64_decode(out, cap, &outLen, (const uint8_t*)p.c_str(), p.length()) == 0) {
    JsonDocument d;
    if (!deserializeJson(d, (const char*)out, outLen)) {
      String u = d["username"] | "";
      if (u.startsWith("u_")) res = u;
      else {
        String uid = d["uid"].is<const char*>() ? String(d["uid"].as<const char*>()) : (d["uid"].is<long long>() ? String((long long)d["uid"].as<long long>()) : String());
        if (uid.length()) res = "u_" + uid;
      }
    }
  }
  free(out);
  return res;
}

String userFor(const String& token, uint8_t region, int* httpCode) {
  String u = userFromJwt(token);
  if (u.length()) return u;
  const char* paths[] = {"/v1/design-user-service/my/preference", "/v1/user-service/my/profile"};
  for (const char* p : paths) {
    Resp r = request("GET", String(api(region)) + p, "", token);
    if (httpCode) *httpCode = r.code;
    if (r.code == 401 || r.code == 403) return "";
    if (r.code != 200 || looksLikeHtml(r.body)) continue;
    JsonDocument d;
    if (deserializeJson(d, r.body)) continue;
    String s = d["uidStr"] | "";
    if (s.isEmpty() && !d["uid"].isNull()) s = String((long long)d["uid"].as<long long>());
    if (s.length() && s != "0") return "u_" + s;
  }
  return "";
}

int devices(const String& token, uint8_t region, Device* out, int max) {
  Resp r = request("GET", String(api(region)) + "/v1/iot-service/api/user/bind", "", token);
  if (r.code != 200 || looksLikeHtml(r.body)) return r.code > 0 ? -r.code : -1;
  JsonDocument f;
  JsonObject fd = f["devices"][0].to<JsonObject>();
  for (const char* k : {"dev_id", "name", "online", "dev_product_name", "dev_model_name", "dev_access_code"}) fd[k] = true;
  JsonDocument d;
  if (deserializeJson(d, r.body, DeserializationOption::Filter(f))) return -1;
  int n = 0;
  for (JsonObject o : d["devices"].as<JsonArray>()) {
    if (n >= max) break;
    out[n].sn = o["dev_id"] | "";
    out[n].name = o["name"] | "";
    String m = o["dev_product_name"] | "";
    if (m.isEmpty()) m = o["dev_model_name"] | "";
    out[n].model = m;
    out[n].acode = o["dev_access_code"] | "";
    out[n].online = o["online"] | false;
    if (out[n].sn.length()) n++;
  }
  return n;
}

bool lastTask(const String& token, uint8_t region, const String& sn, String& title, String& coverUrl) {
  Resp r = request("GET", String(api(region)) + "/v1/user-service/my/tasks?deviceId=" + sn + "&limit=1", "", token);
  if (r.code != 200 || looksLikeHtml(r.body)) return false;
  JsonDocument f;
  JsonObject fh = f["hits"][0].to<JsonObject>();
  fh["title"] = true; fh["designTitle"] = true; fh["cover"] = true;
  JsonDocument d;
  if (deserializeJson(d, r.body, DeserializationOption::Filter(f))) return false;
  JsonObject h = d["hits"][0];
  if (h.isNull()) return false;
  title = h["title"] | "";
  if (title.isEmpty()) title = h["designTitle"] | "";
  coverUrl = h["cover"] | "";
  return coverUrl.length() > 0;
}

}  // namespace bambu

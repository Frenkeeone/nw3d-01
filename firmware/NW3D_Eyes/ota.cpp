#include "ota.h"
#include "version.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>

namespace ota {

static String s_latest, s_url, s_status = "-";
static volatile bool s_check = true, s_install = false;
static volatile int s_progress = -1;
static uint32_t s_lastCheck = 0;

String latest() { return s_latest; }
bool updateAvailable() { return s_latest.length() && s_url.length() && newer(s_latest.c_str(), FW_VERSION); }
int progress() { return s_progress; }
String status() { return s_status; }
void requestCheck() { s_check = true; }
void requestInstall() { s_install = true; }

// "v2.1.3" > "2.0"?
bool newer(const char* a, const char* b) {
  const char* a0 = a;
  const char* b0 = b;
  while (*a && !isdigit((unsigned char)*a)) a++;
  while (*b && !isdigit((unsigned char)*b)) b++;
  for (int i = 0; i < 4; i++) {
    long x = strtol(a, (char**)&a, 10), y = strtol(b, (char**)&b, 10);
    if (x != y) return x > y;
    if (*a == '.') a++;
    if (*b == '.') b++;
    if (!*a && !*b) break;
  }
  // stejna cisla: vydani bez pripony je novejsi nez testovaci (2.0.0 > 2.0.0-beta.1)
  return strchr(b0, '-') && !strchr(a0, '-');
}

static void check() {
  s_lastCheck = millis();
  WiFiClientSecure c;
  c.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  http.setUserAgent("NW3D-Robot/" FW_VERSION);
  if (!http.begin(c, "https://api.github.com/repos/" FW_REPO "/releases/latest")) { s_status = "no connection"; return; }
  http.addHeader("Accept", "application/vnd.github+json");
  int code = http.GET();
  if (code != 200) { s_status = "GitHub HTTP " + String(code); http.end(); return; }
  JsonDocument f;
  f["tag_name"] = true;
  f["assets"][0]["name"] = true;
  f["assets"][0]["browser_download_url"] = true;
  JsonDocument d;
  DeserializationError e = deserializeJson(d, http.getStream(), DeserializationOption::Filter(f));
  http.end();
  if (e) { s_status = "bad reply"; return; }
  s_latest = d["tag_name"] | "";
  s_url = "";
  for (JsonObject a : d["assets"].as<JsonArray>()) {
    String n = a["name"] | "";
    if (n == FW_OTA_ASSET) s_url = a["browser_download_url"] | "";
  }
  s_status = s_url.length() ? "ok" : "release without firmware";
  Serial.printf("[ota] nejnovejsi %s (mam %s)\n", s_latest.c_str(), FW_VERSION);
}

static void install() {
  if (!s_url.length()) { s_status = "nothing to install"; return; }
  s_progress = 0;
  WiFiClientSecure c;
  c.setInsecure();
  HTTPClient http;
  http.setTimeout(20000);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.setUserAgent("NW3D-Robot/" FW_VERSION);
  if (!http.begin(c, s_url)) { s_status = "no connection"; s_progress = -1; return; }
  int code = http.GET();
  int len = http.getSize();
  if (code != 200 || len <= 0) { s_status = "download HTTP " + String(code); http.end(); s_progress = -1; return; }
  if (!Update.begin(len)) { s_status = "no space for update"; http.end(); s_progress = -1; return; }
  WiFiClient* st = http.getStreamPtr();
  uint8_t buf[2048];
  int got = 0;
  uint32_t lastData = millis();
  while (got < len && millis() - lastData < 20000) {
    size_t a = st->available();
    if (!a) { delay(2); continue; }
    int r = st->readBytes(buf, min(a, sizeof buf));
    if (r <= 0) continue;
    if (Update.write(buf, r) != (size_t)r) break;
    got += r;
    lastData = millis();
    s_progress = (int)((int64_t)got * 100 / len);
  }
  http.end();
  if (got == len && Update.end(true)) {
    s_status = "installed, restarting";
    s_progress = 100;
    Serial.println("[ota] hotovo, restart");
    delay(1500);
    ESP.restart();
  }
  Update.abort();
  s_status = "update failed (" + String(got) + "/" + String(len) + ")";
  s_progress = -1;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (s_install) { s_install = false; install(); return; }
  // jednou za 12 hodin, prvne az minutu po startu
  if (s_check && millis() > 60000) { s_check = false; check(); return; }
  if (s_lastCheck && millis() - s_lastCheck > 12UL * 3600 * 1000) s_check = true;
}

}  // namespace ota

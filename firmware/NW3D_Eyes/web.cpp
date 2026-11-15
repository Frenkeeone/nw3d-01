#include "web.h"
#include "web_page.h"
#include "version.h"
#include "settings.h"
#include "cfg.h"
#include "printer.h"
#include "weather.h"
#include "quotes.h"
#include "wifimgr.h"
#include "bambu.h"
#include "cover.h"
#include "ota.h"
#include "i18n.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Update.h>

namespace web {

static WebServer s_srv(80);
static char s_screen[96] = "";
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;
static volatile bool s_test = false, s_sound = false;
static const uint8_t* s_fb[2] = {nullptr, nullptr};

// tiskarny z posledniho nacteni uctu (access code zustava jen v robotovi)
static bambu::Device s_devs[8];
static int s_devN = 0;
static String s_uploadErr;

void setScreenName(const char* n) {
  portENTER_CRITICAL(&s_mux);
  strncpy(s_screen, n, sizeof(s_screen) - 1);
  s_screen[sizeof(s_screen) - 1] = 0;
  portEXIT_CRITICAL(&s_mux);
}

void setFrameSource(const void* l, const void* r) {
  s_fb[0] = (const uint8_t*)l;
  s_fb[1] = (const uint8_t*)r;
}

bool takeTestRequest() { if (!s_test) return false; s_test = false; return true; }
bool takeSoundRequest() { if (!s_sound) return false; s_sound = false; return true; }

static void sendJson(JsonDocument& d, int code = 200) {
  String out;
  serializeJson(d, out);
  s_srv.sendHeader("Cache-Control", "no-store");
  s_srv.send(code, "application/json", out);
}

static void sendOk(const char* msg = "ok") {
  JsonDocument d;
  d["ok"] = true;
  d["msg"] = msg;
  sendJson(d);
}

static void sendErr(const String& msg, int code = 200) {
  JsonDocument d;
  d["ok"] = false;
  d["msg"] = msg;
  sendJson(d, code);
}

// Nahled oka jako BMP 240x240 (24 bit)
static void sendFrame() {
  int e = s_srv.hasArg("e") ? (s_srv.arg("e").toInt() & 1) : 0;
  const uint8_t* fb = s_fb[e];
  if (!fb) { s_srv.send(503, "text/plain", "no frame"); return; }
  const int W = 240, H = 240, row = W * 3;
  const uint32_t size = 54 + row * H;
  uint8_t hdr[54] = {'B', 'M'};
  auto put32 = [&](int o, uint32_t v) { for (int i = 0; i < 4; i++) hdr[o + i] = (v >> (8 * i)) & 0xFF; };
  put32(2, size); put32(10, 54); put32(14, 40); put32(18, W); put32(22, H);
  hdr[26] = 1; hdr[28] = 24; put32(34, row * H);
  s_srv.setContentLength(size);
  s_srv.sendHeader("Cache-Control", "no-store");
  s_srv.send(200, "image/bmp", "");
  s_srv.sendContent((const char*)hdr, sizeof hdr);
  static uint8_t buf[240 * 3 * 8];
  for (int y = H - 1; y >= 0;) {          // BMP je ulozene odspodu
    int n = 0;
    for (int k = 0; k < 8 && y >= 0; k++, y--) {
      const uint8_t* src = fb + (size_t)y * W * 2;
      for (int x = 0; x < W; x++) {
        uint16_t v = (src[2 * x] << 8) | src[2 * x + 1];
        uint8_t r = (v >> 11) & 31, g = (v >> 5) & 63, b = v & 31;
        buf[n++] = (b << 3) | (b >> 2);
        buf[n++] = (g << 2) | (g >> 4);
        buf[n++] = (r << 3) | (r >> 2);
      }
    }
    s_srv.sendContent((const char*)buf, n);
  }
}

static const char* phaseName(Phase p) {
  switch (p) {
    case Phase::Idle: return "Idle";
    case Phase::Preparing: return "Preparing";
    case Phase::Printing: return "Printing";
    case Phase::Paused: return "Paused";
    case Phase::Finished: return "Finished";
    case Phase::Failed: return "Failed";
    default: return "Unknown";
  }
}

static void sendState() {
  Settings S = settings::get();
  DevCfg C = cfg::get();
  PrintInfo P = printer::snapshot();
  WeatherInfo W = weather::snapshot();
  JsonDocument d;
  d["v"] = FW_VERSION;
  d["ip"] = wifimgr::ip();
  d["rssi"] = WiFi.RSSI();
  d["up"] = (uint32_t)(millis() / 1000);
  d["portal"] = wifimgr::state() == wifimgr::State::Portal;
  d["online"] = wifimgr::connected();
  d["ap"] = wifimgr::apName();
  char scr[96];
  portENTER_CRITICAL(&s_mux);
  memcpy(scr, s_screen, sizeof scr);
  portEXIT_CRITICAL(&s_mux);
  d["screen"] = scr;

  JsonObject s = d["set"].to<JsonObject>();
  s["pl"] = S.printW[0];     s["pr"] = S.printW[1];
  s["il"] = S.idleW[0];      s["ir"] = S.idleW[1];
  s["eyes"] = S.showEyes;    s["clock"] = S.showClock;     s["weather"] = S.showWeather;
  s["sec"] = S.screenSec;    s["fin"] = S.finishedMin;
  s["bright"] = S.bright;    s["nbright"] = S.nightBright;
  s["nfrom"] = S.nightFrom;  s["nto"] = S.nightTo;
  s["rot"] = S.rotation;     s["swap"] = S.swapEyes;
  s["quotes"] = S.showQuotes; s["tk"] = S.tickers;
  s["lang"] = S.lang == 0xFF ? -1 : S.lang;
  s["vol"] = S.volume;       s["sfin"] = S.soundFinish;    s["serr"] = S.soundError;
  s["done"] = S.setupDone;

  // pripojeni (bez hesel a tokenu!)
  JsonObject c = d["cfg"].to<JsonObject>();
  c["ssid"] = C.ssid;
  c["mode"] = C.mode;
  c["sn"] = C.sn;
  c["pname"] = C.pname;
  c["pmodel"] = C.pmodel;
  c["ip"] = C.ip;
  c["email"] = C.email;
  c["region"] = C.region;
  c["logged"] = C.token.length() > 20;
  c["city"] = C.city;
  c["tzname"] = C.tzName;
  if (C.hasLocation()) { c["lat"] = C.lat; c["lon"] = C.lon; }

  JsonObject p = d["pr"].to<JsonObject>();
  p["conf"] = P.configured;  p["online"] = P.online;       p["data"] = P.hasData;
  p["phase"] = phaseName(P.phase);
  p["pct"] = P.percent;      p["rem"] = P.remainingMin;
  p["layer"] = P.layer;      p["layers"] = P.totalLayers;
  p["noz"] = P.nozzle;       p["bed"] = P.bed;
  p["job"] = P.job;          p["err"] = P.error;
  String st = P.status;
  st.replace("|", " – ");
  p["status"] = st;
  p["cover"] = cover::status();

  JsonObject w = d["w"].to<JsonObject>();
  w["valid"] = W.valid; w["temp"] = W.temp; w["tmin"] = W.tmin; w["tmax"] = W.tmax;
  w["code"] = W.code; w["rain"] = W.rainProb; w["status"] = W.status;

  JsonArray qa = d["q"].to<JsonArray>();
  for (int i = 0; i < quotes::count(); i++) {
    QuoteInfo q = quotes::get(i);
    JsonObject o = qa.add<JsonObject>();
    o["s"] = q.sym; o["n"] = q.name; o["u"] = q.unit; o["ok"] = q.valid;
    if (q.valid) { o["p"] = q.price; o["pc"] = q.prev; }
    o["st"] = q.status;
  }

  JsonObject o = d["ota"].to<JsonObject>();
  o["latest"] = ota::latest();
  o["avail"] = ota::updateAvailable();
  o["prog"] = ota::progress();
  o["status"] = ota::status();
  sendJson(d);
}

static long argInt(const char* k, long lo, long hi, long def) {
  if (!s_srv.hasArg(k)) return def;
  long v = s_srv.arg(k).toInt();
  return v < lo ? lo : (v > hi ? hi : v);
}

static void handleSet() {
  Settings S = settings::get();
  S.printW[0]     = argInt("pl", 0, W_COUNT - 1, S.printW[0]);
  S.printW[1]     = argInt("pr", 0, W_COUNT - 1, S.printW[1]);
  S.idleW[0]      = argInt("il", 0, W_COUNT - 1, S.idleW[0]);
  S.idleW[1]      = argInt("ir", 0, W_COUNT - 1, S.idleW[1]);
  S.showEyes      = argInt("eyes", 0, 1, S.showEyes);
  S.showClock     = argInt("clock", 0, 1, S.showClock);
  S.showWeather   = argInt("weather", 0, 1, S.showWeather);
  S.screenSec     = argInt("sec", 3, 600, S.screenSec);
  S.finishedMin   = argInt("fin", 1, 1440, S.finishedMin);
  S.bright        = argInt("bright", 1, 100, S.bright);
  S.nightBright   = argInt("nbright", 0, 100, S.nightBright);
  S.nightFrom     = argInt("nfrom", 0, 23, S.nightFrom);
  S.nightTo       = argInt("nto", 0, 23, S.nightTo);
  S.rotation      = argInt("rot", 0, 3, S.rotation);
  S.swapEyes      = argInt("swap", 0, 1, S.swapEyes);
  S.showQuotes    = argInt("quotes", 0, 1, S.showQuotes);
  S.volume        = argInt("vol", 0, 100, S.volume);
  S.soundFinish   = argInt("sfin", 0, 1, S.soundFinish);
  S.soundError    = argInt("serr", 0, 1, S.soundError);
  S.setupDone     = argInt("done", 0, 1, S.setupDone);
  if (s_srv.hasArg("lang")) S.lang = argInt("lang", 0, LANG_COUNT - 1, 0);
  if (s_srv.hasArg("tk")) {
    // jen bezpecne znaky symbolu, max. 4 polozky
    String in = s_srv.arg("tk");
    char out[sizeof(S.tickers)];
    size_t j = 0;
    int items = 0, len = 0;
    for (size_t i = 0; i <= in.length() && j + 1 < sizeof out; i++) {
      char c = i < in.length() ? in[i] : ',';
      if (c == ',' || c == ' ' || c == ';') {
        if (len > 0) { items++; len = 0; if (items >= quotes::MAX) break; out[j++] = ','; }
        continue;
      }
      if (isalnum((unsigned char)c) || strchr(".=^-_", c)) {
        if (len < 15) { out[j++] = toupper((unsigned char)c); len++; }
      }
    }
    while (j > 0 && out[j - 1] == ',') j--;
    out[j] = 0;
    memcpy(S.tickers, out, sizeof out);
  }
  settings::set(S);
  if (S.lang != 0xFF) i18n::setLang(S.lang);
  sendState();
}

// ---------------- Wi-Fi ----------------
static void handleScan() {
  String j = wifimgr::scanJson();
  s_srv.sendHeader("Cache-Control", "no-store");
  s_srv.send(200, "application/json", j);
}

static void handleWifi() {
  String ssid = s_srv.arg("ssid"), pass = s_srv.arg("pass");
  if (ssid.isEmpty() || ssid.length() > 32 || pass.length() > 64) { sendErr("bad ssid"); return; }
  sendOk();
  delay(300);                       // at odpoved stihne odejit
  wifimgr::connectTo(ssid, pass, 0);
}

// ---------------- Bambu ucet ----------------
static void loginResult(const bambu::Login& l, const String& email, uint8_t region) {
  JsonDocument d;
  switch (l.kind) {
    case bambu::Login::Ok: {
      DevCfg c = cfg::get();
      c.token = l.token;
      c.region = region;
      c.email = email;
      c.user = "";
      cfg::set(c);
      d["r"] = "ok";
      break;
    }
    case bambu::Login::NeedCode: d["r"] = "code"; break;
    case bambu::Login::NeedTfa:  d["r"] = "tfa"; d["key"] = l.tfaKey; break;
    default:                     d["r"] = "err"; d["msg"] = l.error; break;
  }
  sendJson(d);
}

static uint8_t regionArg() { return argInt("region", 0, 1, 0); }

static void handleLogin() {
  String email = s_srv.arg("email"); email.trim();
  uint8_t region = regionArg();
  bambu::Login l = bambu::login(email, s_srv.arg("pass"), region);
  if (l.kind == bambu::Login::NeedCode) bambu::sendCode(email, region);
  loginResult(l, email, region);
}

static void handleCode() {
  String email = s_srv.arg("email"); email.trim();
  uint8_t region = regionArg();
  if (s_srv.hasArg("resend")) { bool ok = bambu::sendCode(email, region); ok ? sendOk() : sendErr("send failed"); return; }
  loginResult(bambu::loginCode(email, s_srv.arg("code"), region), email, region);
}

static void handleTfa() {
  String email = s_srv.arg("email"); email.trim();
  uint8_t region = regionArg();
  loginResult(bambu::loginTfa(s_srv.arg("key"), s_srv.arg("code"), region), email, region);
}

static void handleToken() {
  String t = s_srv.arg("token");
  t.trim();
  if (t.startsWith("Bearer ")) t = t.substring(7);
  if (t.length() < 20) { sendErr("token too short"); return; }
  DevCfg c = cfg::get();
  c.token = t;
  c.region = regionArg();
  c.user = "";
  cfg::set(c);
  sendOk();
}

static void handleLogout() {
  DevCfg c = cfg::get();
  c.token = ""; c.user = ""; c.email = "";
  if (c.mode == PM_CLOUD) c.mode = PM_NONE;
  cfg::set(c);
  s_devN = 0;
  sendOk();
}

static void handleDevices() {
  DevCfg c = cfg::get();
  if (c.token.length() < 20) { sendErr("not logged in"); return; }
  int n = bambu::devices(c.token, c.region, s_devs, 8);
  JsonDocument d;
  if (n < 0) {
    s_devN = 0;
    d["ok"] = false;
    d["msg"] = (n == -401 || n == -403) ? String("login expired") : String("HTTP ") + String(-n);
    sendJson(d);
    return;
  }
  s_devN = n;
  d["ok"] = true;
  JsonArray a = d["list"].to<JsonArray>();
  for (int i = 0; i < n; i++) {
    JsonObject o = a.add<JsonObject>();
    o["i"] = i;
    o["sn"] = s_devs[i].sn;
    o["name"] = s_devs[i].name;
    o["model"] = s_devs[i].model;
    o["online"] = s_devs[i].online;
  }
  sendJson(d);
}

// Vyber tiskarny: z uctu (i = poradi v seznamu) nebo rucne (sn, ip, acode)
static void handlePrinter() {
  DevCfg c = cfg::get();
  String conn = s_srv.arg("conn");          // "cloud", "lan" nebo "none"
  if (conn == "none") { c.mode = PM_NONE; cfg::set(c); sendOk(); return; }
  if (s_srv.hasArg("i")) {
    int i = s_srv.arg("i").toInt();
    if (i < 0 || i >= s_devN) { sendErr("reload the printer list"); return; }
    c.sn = s_devs[i].sn;
    c.pname = s_devs[i].name;
    c.pmodel = s_devs[i].model;
    if (s_devs[i].acode.length()) c.acode = s_devs[i].acode;
  } else if (s_srv.hasArg("sn")) {
    c.sn = s_srv.arg("sn"); c.sn.trim(); c.sn.toUpperCase();
    c.pname = c.sn;
    c.pmodel = "";
  }
  if (s_srv.hasArg("acode") && s_srv.arg("acode").length()) { c.acode = s_srv.arg("acode"); c.acode.trim(); }
  if (conn == "lan") {
    c.mode = PM_LAN;
    c.ip = s_srv.arg("ip"); c.ip.trim();
    if (c.ip.length() < 7) { sendErr("missing printer IP"); return; }
    if (c.acode.length() < 6) { sendErr("missing access code"); return; }
  } else {
    if (c.token.length() < 20) { sendErr("not logged in"); return; }
    c.mode = PM_CLOUD;
  }
  if (c.sn.length() < 6) { sendErr("missing serial number"); return; }
  cfg::set(c);
  sendOk();
}

static void handleLocation() {
  DevCfg c = cfg::get();
  if (s_srv.hasArg("clear")) { c.lat = c.lon = NAN; c.city = ""; cfg::set(c); sendOk(); return; }
  float lat = s_srv.arg("lat").toFloat(), lon = s_srv.arg("lon").toFloat();
  if (s_srv.hasArg("lat")) {
    if (lat < -90 || lat > 90 || lon < -180 || lon > 180) { sendErr("bad coordinates"); return; }
    c.lat = lat; c.lon = lon;
    c.city = s_srv.arg("city").substring(0, 60);
  }
  if (s_srv.hasArg("tz") && s_srv.arg("tz").length() > 2 && s_srv.arg("tz").length() < 64) {
    c.tz = s_srv.arg("tz");
    c.tzName = s_srv.arg("tzname").substring(0, 40);
  }
  cfg::set(c);
  sendOk();
}

// ---------------- Aktualizace ----------------
static void handleUploadDone() {
  if (Update.hasError() || s_uploadErr.length()) {
    sendErr(s_uploadErr.length() ? s_uploadErr : String("update failed"));
    return;
  }
  sendOk("restart");
  delay(800);
  ESP.restart();
}

static void handleUpload() {
  HTTPUpload& u = s_srv.upload();
  if (u.status == UPLOAD_FILE_START) {
    s_uploadErr = "";
    if (!u.filename.endsWith(".bin")) { s_uploadErr = "not a .bin file"; return; }
    Serial.printf("[ota] nahravam %s\n", u.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) s_uploadErr = "cannot start update";
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (s_uploadErr.isEmpty() && Update.write(u.buf, u.currentSize) != u.currentSize) s_uploadErr = "write error";
  } else if (u.status == UPLOAD_FILE_END) {
    if (s_uploadErr.isEmpty() && !Update.end(true)) s_uploadErr = "image invalid (merged/factory .bin? use the OTA file)";
  } else if (u.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    s_uploadErr = "aborted";
  }
}

static void handleFactory() {
  sendOk("restart");
  delay(500);
  cfg::factoryReset();
  settings::wipe();
  delay(300);
  ESP.restart();
}

// Nastavovaci sit: vsechny cizi adresy presmerovat na stranku robota (captive portal)
static bool captive() {
  if (wifimgr::state() != wifimgr::State::Portal) return false;
  String host = s_srv.hostHeader();
  if (host.length() == 0 || host == WiFi.softAPIP().toString() || host.startsWith("nw3d-robot")) return false;
  s_srv.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
  s_srv.send(302, "text/plain", "");
  return true;
}

static void task(void*) {
  // pockat, az bude nejaka sit (domaci nebo nastavovaci)
  while (wifimgr::ip().isEmpty()) vTaskDelay(pdMS_TO_TICKS(300));
  if (MDNS.begin("nw3d-robot")) MDNS.addService("http", "tcp", 80);
  s_srv.on("/", HTTP_GET, [] {
    if (captive()) return;
    s_srv.sendHeader("Cache-Control", "no-store");
    s_srv.send_P(200, "text/html; charset=utf-8", WEB_PAGE);
  });
  s_srv.on("/api/state", HTTP_GET, sendState);
  s_srv.on("/api/set", HTTP_POST, handleSet);
  s_srv.on("/api/frame", HTTP_GET, sendFrame);
  s_srv.on("/api/test", HTTP_POST, [] { s_test = true; sendOk(); });
  s_srv.on("/api/sound", HTTP_POST, [] { s_sound = true; sendOk(); });
  s_srv.on("/api/reset", HTTP_POST, [] { settings::reset(); sendState(); });
  s_srv.on("/api/factory", HTTP_POST, handleFactory);
  s_srv.on("/api/scan", HTTP_GET, handleScan);
  s_srv.on("/api/wifi", HTTP_POST, handleWifi);
  s_srv.on("/api/bambu/login", HTTP_POST, handleLogin);
  s_srv.on("/api/bambu/code", HTTP_POST, handleCode);
  s_srv.on("/api/bambu/tfa", HTTP_POST, handleTfa);
  s_srv.on("/api/bambu/token", HTTP_POST, handleToken);
  s_srv.on("/api/bambu/logout", HTTP_POST, handleLogout);
  s_srv.on("/api/bambu/devices", HTTP_GET, handleDevices);
  s_srv.on("/api/printer", HTTP_POST, handlePrinter);
  s_srv.on("/api/location", HTTP_POST, handleLocation);
  s_srv.on("/api/ota/check", HTTP_POST, [] { ota::requestCheck(); sendOk(); });
  s_srv.on("/api/ota/install", HTTP_POST, [] { ota::requestInstall(); sendOk(); });
  s_srv.on("/api/ota/upload", HTTP_POST, handleUploadDone, handleUpload);
  s_srv.onNotFound([] {
    if (captive()) return;
    s_srv.send(404, "text/plain", "not found");
  });
  s_srv.begin();
  Serial.printf("[web] http://nw3d-robot.local  nebo  http://%s\n", wifimgr::ip().c_str());
  bool mdnsOnSta = wifimgr::connected();
  for (;;) {
    s_srv.handleClient();
    // mDNS spusteny v nastavovaci siti znovu pro domaci sit
    if (!mdnsOnSta && wifimgr::connected()) {
      mdnsOnSta = true;
      MDNS.end();
      if (MDNS.begin("nw3d-robot")) MDNS.addService("http", "tcp", 80);
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void begin() { xTaskCreatePinnedToCore(task, "web", 16384, nullptr, 1, nullptr, 0); }

}  // namespace web

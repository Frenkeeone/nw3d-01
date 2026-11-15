#include "wifimgr.h"
#include "cfg.h"
#include "version.h"
#include <WiFi.h>
#include <DNSServer.h>
#include "esp_mac.h"

namespace wifimgr {

static State s_state = State::Connecting;
static DNSServer s_dns;
static bool s_portal = false;
static uint32_t s_since = 0;          // od kdy se pripojujeme / jsme pripojeni
static uint32_t s_connectedAt = 0;
static uint32_t s_lastKick = 0;
static String s_ap;
static SemaphoreHandle_t s_scanLock;
static uint32_t s_scanAt = 0;
static int s_scanN = -1;
struct Net { String ssid; int rssi; bool locked; };
static Net s_nets[20];

static const IPAddress AP_IP(192, 168, 4, 1);

String apName() { return s_ap; }
bool connected() { return WiFi.status() == WL_CONNECTED; }
State state() { return s_state; }
String ip() {
  if (connected()) return WiFi.localIP().toString();
  if (s_portal) return AP_IP.toString();
  return "";
}

static void startPortal() {
  if (s_portal) return;
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(s_ap.c_str());
  s_dns.setErrorReplyCode(DNSReplyCode::NoError);
  s_dns.start(53, "*", AP_IP);
  s_portal = true;
  Serial.printf("[wifi] nastavovaci sit %s, http://%s\n", s_ap.c_str(), AP_IP.toString().c_str());
}

static void stopPortal() {
  if (!s_portal) return;
  s_dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  s_portal = false;
  Serial.println("[wifi] nastavovaci sit vypnuta");
}

void begin() {
  s_scanLock = xSemaphoreCreateMutex();
  uint8_t mac[6] = {0};
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char buf[16];
  snprintf(buf, sizeof buf, "NW3D-%02X%02X", mac[4], mac[5]);
  s_ap = buf;
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("nw3d-robot");
  WiFi.setAutoReconnect(true);
  DevCfg c = cfg::get();
  s_since = millis();
  if (c.ssid.length()) {
    WiFi.begin(c.ssid.c_str(), c.pass.c_str());
  } else {
    startPortal();
  }
}

void loop() {
  uint32_t now = millis();
  bool up = connected();
  if (up) {
    if (s_state != State::Connected) {
      s_state = State::Connected;
      s_connectedAt = now;
      Serial.printf("[wifi] pripojeno, IP %s\n", WiFi.localIP().toString().c_str());
    }
    // nastavovaci sit nechat jeste 2 minuty, at uzivatel uvidi novou adresu
    if (s_portal && now - s_connectedAt > 120000UL && WiFi.softAPgetStationNum() == 0) stopPortal();
  } else {
    if (s_state == State::Connected) { s_state = State::Connecting; s_since = now; }
    DevCfg c = cfg::get();
    if (!s_portal && (c.ssid.isEmpty() || now - s_since > 60000UL)) startPortal();
    // kdyz je nekdo pripojeny k nastavovaci siti, neprepinat kanaly zkousenim domaci site
    bool guests = s_portal && WiFi.softAPgetStationNum() > 0;
    if (c.ssid.length() && !guests && now - s_lastKick > (s_portal ? 45000UL : 20000UL)) {
      s_lastKick = now;
      WiFi.disconnect();
      WiFi.begin(c.ssid.c_str(), c.pass.c_str());
    }
    if (s_portal) s_state = State::Portal;
  }
  if (s_portal) s_dns.processNextRequest();
  improvLoop();
}

bool connectTo(const String& ssid, const String& pass, uint32_t waitMs) {
  DevCfg c = cfg::get();
  c.ssid = ssid;
  c.pass = pass;
  cfg::set(c);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid.c_str(), pass.c_str());
  s_since = s_lastKick = millis();
  if (!waitMs) return false;
  uint32_t t0 = millis();
  while (millis() - t0 < waitMs) {
    if (connected()) return true;
    if (s_portal) s_dns.processNextRequest();
    delay(100);
  }
  return connected();
}

static void doScan() {
  xSemaphoreTake(s_scanLock, portMAX_DELAY);
  if (s_scanN < 0 || millis() - s_scanAt > 20000UL) {
    int n = WiFi.scanNetworks(false, false);
    int k = 0;
    for (int i = 0; i < n && k < 20; i++) {
      String s = WiFi.SSID(i);
      if (s.isEmpty()) continue;
      bool dup = false;
      for (int j = 0; j < k; j++) if (s_nets[j].ssid == s) { dup = true; break; }
      if (dup) continue;
      s_nets[k++] = {s, WiFi.RSSI(i), WiFi.encryptionType(i) != WIFI_AUTH_OPEN};
    }
    WiFi.scanDelete();
    s_scanN = k;
    s_scanAt = millis();
  }
  xSemaphoreGive(s_scanLock);
}

int scanCount() { doScan(); return s_scanN; }

bool scanItem(int i, String& ssid, int& rssi, bool& locked) {
  if (i < 0 || i >= s_scanN) return false;
  ssid = s_nets[i].ssid; rssi = s_nets[i].rssi; locked = s_nets[i].locked;
  return true;
}

String scanJson() {
  doScan();
  String out = "[";
  for (int i = 0; i < s_scanN; i++) {
    if (i) out += ",";
    String s = s_nets[i].ssid;
    s.replace("\\", "\\\\");
    s.replace("\"", "\\\"");
    out += "{\"s\":\"" + s + "\",\"r\":" + String(s_nets[i].rssi) + ",\"l\":" + (s_nets[i].locked ? "1" : "0") + "}";
  }
  return out + "]";
}

// --------------------------------------------------------------------
//  Improv Wi-Fi pres USB (https://www.improv-wifi.com/serial/)
//  Webovy instalator po nahrani firmwaru pomoci nej zada Wi-Fi.
// --------------------------------------------------------------------
enum : uint8_t { IMP_CUR_STATE = 1, IMP_ERROR = 2, IMP_RPC = 3, IMP_RPC_RESULT = 4 };
enum : uint8_t { IMP_ST_READY = 2, IMP_ST_PROVISIONING = 3, IMP_ST_PROVISIONED = 4 };
enum : uint8_t { IMP_ERR_NONE = 0, IMP_ERR_INVALID = 1, IMP_ERR_UNKNOWN_CMD = 2, IMP_ERR_CONNECT = 3 };
enum : uint8_t { CMD_WIFI = 1, CMD_STATE = 2, CMD_INFO = 3, CMD_SCAN = 4 };

static uint8_t s_buf[256];
static size_t s_len = 0;

static void impSend(uint8_t type, const uint8_t* data, uint8_t len) {
  uint8_t pkt[6 + 3 + 255 + 1];
  memcpy(pkt, "IMPROV", 6);
  pkt[6] = 1;
  pkt[7] = type;
  pkt[8] = len;
  memcpy(pkt + 9, data, len);
  uint8_t sum = 0;
  for (int i = 0; i < 9 + len; i++) sum += pkt[i];
  pkt[9 + len] = sum;
  Serial.write('\n');          // paket musi zacinat na novem radku (mezi logy)
  Serial.write(pkt, 10 + len);
  Serial.write('\n');
  Serial.flush();
}

static void impState(uint8_t st) { impSend(IMP_CUR_STATE, &st, 1); }
static void impError(uint8_t e) { impSend(IMP_ERROR, &e, 1); }

static void impResult(uint8_t cmd, const char* const* strs, int n) {
  uint8_t d[250];
  size_t p = 2;
  for (int i = 0; i < n; i++) {
    size_t l = strlen(strs[i]);
    if (p + 1 + l > sizeof d) break;
    d[p++] = (uint8_t)l;
    memcpy(d + p, strs[i], l);
    p += l;
  }
  d[0] = cmd;
  d[1] = (uint8_t)(p - 2);
  impSend(IMP_RPC_RESULT, d, (uint8_t)p);
}

static void impUrl(uint8_t cmd) {
  String url = "http://" + WiFi.localIP().toString() + "/";
  const char* s[] = {url.c_str()};
  impResult(cmd, s, 1);
}

static void impHandle(const uint8_t* d, uint8_t len) {
  if (len < 2) { impError(IMP_ERR_INVALID); return; }
  uint8_t cmd = d[0];
  switch (cmd) {
    case CMD_WIFI: {
      if (len < 3) { impError(IMP_ERR_INVALID); return; }
      uint8_t sl = d[2];
      if (3 + sl > len) { impError(IMP_ERR_INVALID); return; }
      String ssid((const char*)d + 3, sl);
      uint8_t pl = (3 + sl < len) ? d[3 + sl] : 0;
      if (4 + sl + pl > len + (pl ? 0 : 1)) { impError(IMP_ERR_INVALID); return; }
      String pass = pl ? String((const char*)d + 4 + sl, pl) : String();
      impState(IMP_ST_PROVISIONING);
      Serial.printf("[improv] pripojuji k %s\n", ssid.c_str());
      if (connectTo(ssid, pass, 20000)) {
        impState(IMP_ST_PROVISIONED);
        impUrl(CMD_WIFI);
      } else {
        impError(IMP_ERR_CONNECT);
        impState(IMP_ST_READY);
      }
      break;
    }
    case CMD_STATE:
      if (connected()) { impState(IMP_ST_PROVISIONED); impUrl(CMD_STATE); }
      else impState(IMP_ST_READY);
      break;
    case CMD_INFO: {
      const char* s[] = {"NW3D-01", FW_VERSION, "ESP32-S3", "NW3D robot"};
      impResult(CMD_INFO, s, 4);
      break;
    }
    case CMD_SCAN: {
      int n = scanCount();
      for (int i = 0; i < n; i++) {
        String ssid; int rssi; bool locked;
        if (!scanItem(i, ssid, rssi, locked)) continue;
        String r = String(rssi);
        const char* s[] = {ssid.c_str(), r.c_str(), locked ? "YES" : "NO"};
        impResult(CMD_SCAN, s, 3);
      }
      impResult(CMD_SCAN, nullptr, 0);
      break;
    }
    default:
      impError(IMP_ERR_UNKNOWN_CMD);
  }
}

void improvLoop() {
  while (Serial.available()) {
    int c = Serial.read();
    if (c < 0) break;
    if (s_len >= sizeof s_buf) s_len = 0;
    s_buf[s_len++] = (uint8_t)c;
    // hlavicka "IMPROV" - jinak zahodit
    static const char H[] = "IMPROV";
    if (s_len <= 6) {
      if (s_buf[s_len - 1] != (uint8_t)H[s_len - 1]) {
        s_len = (c == 'I') ? 1 : 0;
        if (s_len) s_buf[0] = 'I';
      }
      continue;
    }
    if (s_len < 9) continue;
    uint8_t len = s_buf[8];
    if (s_len < (size_t)(9 + len + 1)) continue;
    // cely paket
    uint8_t sum = 0;
    for (int i = 0; i < 9 + len; i++) sum += s_buf[i];
    uint8_t type = s_buf[7];
    bool ok = s_buf[6] == 1 && sum == s_buf[9 + len];
    uint8_t data[256];
    memcpy(data, s_buf + 9, len);
    s_len = 0;
    if (!ok) { impError(IMP_ERR_INVALID); continue; }
    if (type == IMP_RPC) impHandle(data, len);
  }
}

}  // namespace wifimgr

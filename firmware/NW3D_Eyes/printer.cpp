// MQTT spojeni s tiskarnou Bambu Lab (LAN nebo Bambu cloud).
#include "printer.h"
#include "cfg.h"
#include "bambu.h"
#include "i18n.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

namespace printer {

static WiFiClientSecure s_tls;
static PubSubClient s_mqtt(s_tls);
static SemaphoreHandle_t s_lock;
static PrintInfo s_state;

static char s_topicReport[64], s_topicRequest[64];
static String s_user;
static uint32_t s_lastTry = 0, s_backoff = 0, s_lastPushall = 0, s_connectedAt = 0, s_lastMsg = 0;

static void setStatus(const char* fmt, ...) {
  char buf[sizeof(s_state.status)];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  xSemaphoreTake(s_lock, portMAX_DELAY);
  strncpy(s_state.status, buf, sizeof(s_state.status) - 1);
  xSemaphoreGive(s_lock);
  Serial.printf("[tiskarna] %s\n", buf);
}

static void onMessage(char*, uint8_t* payload, unsigned int len) {
  PrintInfo tmp;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  tmp = s_state;
  xSemaphoreGive(s_lock);
  if (!parseReport((const char*)payload, len, tmp)) return;
  s_lastMsg = millis();
  tmp.lastMsgMs = s_lastMsg;
  tmp.status[0] = 0;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  s_state = tmp;
  xSemaphoreGive(s_lock);
}

static void requestPushall() {
  s_mqtt.publish(s_topicRequest,
                 "{\"pushing\":{\"sequence_id\":\"0\",\"command\":\"pushall\",\"version\":1,\"push_target\":1}}");
  s_lastPushall = millis();
}

static DevCfg s_cfg;
static uint32_t s_cfgVer = 0;

// Cloud: MQTT uzivatel "u_<uid>" k tokenu (z tokenu nebo dotazem na server)
static bool resolveCloudUser() {
  if (s_cfg.user.length()) { s_user = s_cfg.user; return true; }
  int code = 0;
  String u = bambu::userFor(s_cfg.token, s_cfg.region, &code);
  if (u.isEmpty()) {
    if (code == 401 || code == 403) setStatus(tr(S_P_TOKEN_REJECTED), code);
    else if (code <= 0) setStatus(tr(S_P_CLOUD_DOWN));
    else setStatus(tr(S_P_NO_UID));
    return false;
  }
  s_user = u;
  DevCfg c = cfg::get();          // ulozit, priste uz se nemusi zjistovat
  if (c.token == s_cfg.token) { c.user = u; cfg::set(c); s_cfg.user = u; s_cfgVer = cfg::version(); }
  Serial.printf("[tiskarna] cloud uzivatel %s\n", u.c_str());
  return true;
}

static void applyConfig() {
  s_cfg = cfg::get();
  s_cfgVer = cfg::version();
  if (s_mqtt.connected()) s_mqtt.disconnect();
  s_user = "";
  s_lastTry = 0; s_backoff = 0; s_lastMsg = 0;
  snprintf(s_topicReport, sizeof s_topicReport, "device/%s/report", s_cfg.sn.c_str());
  snprintf(s_topicRequest, sizeof s_topicRequest, "device/%s/request", s_cfg.sn.c_str());
  PrintInfo fresh;
  fresh.configured = s_cfg.printerReady();
  xSemaphoreTake(s_lock, portMAX_DELAY);
  s_state = fresh;
  xSemaphoreGive(s_lock);
  if (!fresh.configured) setStatus(tr(S_P_NOT_SET));
}

void begin() {
  s_lock = xSemaphoreCreateMutex();
  s_mqtt.setCallback(onMessage);
  s_mqtt.setKeepAlive(30);
  s_mqtt.setSocketTimeout(15);
  if (!s_mqtt.setBufferSize(65000)) s_mqtt.setBufferSize(32768);
  applyConfig();
}

void loop() {
  if (cfg::version() != s_cfgVer) {
    DevCfg c = cfg::get();
    // zmena jen jinych polozek (napr. poloha) spojeni neprerusi
    if (c.mode != s_cfg.mode || c.sn != s_cfg.sn || c.ip != s_cfg.ip || c.acode != s_cfg.acode ||
        c.token != s_cfg.token || c.region != s_cfg.region) applyConfig();
    else { s_cfg = c; s_cfgVer = cfg::version(); }
  }
  if (!s_state.configured) return;
  uint32_t now = millis();

  if (!s_mqtt.connected()) {
    if (s_state.online) {
      xSemaphoreTake(s_lock, portMAX_DELAY);
      s_state.online = false;
      xSemaphoreGive(s_lock);
    }
    if (s_lastTry && now - s_lastTry < s_backoff) return;
    s_lastTry = now;
    s_backoff = s_backoff ? min<uint32_t>(s_backoff * 2, 120000) : 5000;

    const bool cloud = s_cfg.mode == PM_CLOUD;
    String host, pass;
    if (cloud) {
      if (s_user.isEmpty() && !resolveCloudUser()) return;
      host = s_cfg.region ? "cn.mqtt.bambulab.com" : "us.mqtt.bambulab.com";
      pass = s_cfg.token;
    } else {
      s_user = "bblp";
      host = s_cfg.ip;
      pass = s_cfg.acode;
    }
    s_tls.setInsecure();          // tiskarna ma vlastni certifikat
    s_tls.setHandshakeTimeout(15);
    s_mqtt.setServer(host.c_str(), 8883);
    setStatus(tr(S_P_CONNECTING), host.c_str());
    String cid = String("nw3d_") + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (!s_mqtt.connect(cid.c_str(), s_user.c_str(), pass.c_str())) {
      int st = s_mqtt.state();
      if (st == 4 || st == 5) setStatus(tr(cloud ? S_P_DENIED_TOKEN : S_P_DENIED_CODE));
      else setStatus(tr(cloud ? S_P_UNREACH_CLOUD : S_P_UNREACH_LAN), st);
      if (cloud && (st == 4 || st == 5)) {   // zkusit znovu zjistit uid
        s_user = "";
        DevCfg c = cfg::get();
        if (c.user.length() && c.token == s_cfg.token) { c.user = ""; cfg::set(c); s_cfg.user = ""; s_cfgVer = cfg::version(); }
      }
      return;
    }
    s_mqtt.subscribe(s_topicReport);
    s_backoff = 0;
    s_connectedAt = now;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_state.online = true;
    s_state.status[0] = 0;
    xSemaphoreGive(s_lock);
    Serial.println("[tiskarna] pripojeno, zadam o stav");
    requestPushall();
    return;
  }

  s_mqtt.loop();

  // Kdyz neprijdou zadna data, pozadat znovu a ukazat napovedu
  uint32_t quiet = now - (s_lastMsg ? s_lastMsg : s_connectedAt);
  if (quiet > 60000 && now - s_lastPushall > 60000) {
    requestPushall();
    if (!s_lastMsg) setStatus(tr(s_cfg.mode == PM_LAN ? S_P_NO_DATA_LAN : S_P_NO_DATA_CLOUD));
  }
}

PrintInfo snapshot() {
  PrintInfo out;
  if (!s_lock) return out;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  out = s_state;
  xSemaphoreGive(s_lock);
  return out;
}

}  // namespace printer

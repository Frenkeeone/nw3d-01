#include "cfg.h"
#include "config.h"
#include <Preferences.h>

namespace cfg {

static DevCfg s_cur;
static SemaphoreHandle_t s_lock;
static volatile uint32_t s_ver = 1;
static Preferences s_p;

static bool real(const char* v, const char* placeholder) { return v && *v && strcmp(v, placeholder) != 0; }

static void save(const DevCfg& c) {
  s_p.putString("ssid", c.ssid);   s_p.putString("pass", c.pass);
  s_p.putUChar("mode", c.mode);
  s_p.putString("sn", c.sn);       s_p.putString("pname", c.pname);  s_p.putString("pmodel", c.pmodel);
  s_p.putString("ip", c.ip);       s_p.putString("acode", c.acode);
  s_p.putString("token", c.token); s_p.putString("user", c.user);    s_p.putString("email", c.email);
  s_p.putUChar("region", c.region);
  s_p.putFloat("lat", c.lat);      s_p.putFloat("lon", c.lon);       s_p.putString("city", c.city);
  s_p.putString("tz", c.tz);       s_p.putString("tzname", c.tzName);
  s_p.putUChar("v", 1);
}

static DevCfg load() {
  DevCfg c;
  c.ssid = s_p.getString("ssid", "");   c.pass = s_p.getString("pass", "");
  c.mode = s_p.getUChar("mode", PM_NONE);
  c.sn = s_p.getString("sn", "");       c.pname = s_p.getString("pname", "");  c.pmodel = s_p.getString("pmodel", "");
  c.ip = s_p.getString("ip", "");       c.acode = s_p.getString("acode", "");
  c.token = s_p.getString("token", ""); c.user = s_p.getString("user", "");    c.email = s_p.getString("email", "");
  c.region = s_p.getUChar("region", 0);
  c.lat = s_p.getFloat("lat", NAN);     c.lon = s_p.getFloat("lon", NAN);      c.city = s_p.getString("city", "");
  c.tz = s_p.getString("tz", c.tz);     c.tzName = s_p.getString("tzname", c.tzName);
  return c;
}

// Prvni spusteni: prevzit udaje z config.h, pokud je nekdo vyplnil (stary zpusob)
static DevCfg fromConfigH() {
  DevCfg c;
  if (real(WIFI_SSID, "nazev_wifi")) { c.ssid = WIFI_SSID; c.pass = WIFI_PASSWORD; }
  bool snOk = strlen(PRINTER_SERIAL) > 5 && PRINTER_SERIAL[1] != 'X';
  if (snOk) {
    c.sn = PRINTER_SERIAL;
    if (PRINTER_MODE == 1 && real(BAMBU_TOKEN, "sem_vloz_token")) {
      c.mode = PM_CLOUD;
      c.token = BAMBU_TOKEN;
      if (strlen(BAMBU_UID) > 0) c.user = String("u_") + BAMBU_UID;
      c.region = BAMBU_CHINA ? 1 : 0;
    } else if (PRINTER_MODE == 0 && real(PRINTER_ACCESS_CODE, "12345678")) {
      c.mode = PM_LAN;
      c.ip = PRINTER_IP;
      c.acode = PRINTER_ACCESS_CODE;
    }
  }
  if ((float)WEATHER_LAT != 0.0f || (float)WEATHER_LON != 0.0f) { c.lat = WEATHER_LAT; c.lon = WEATHER_LON; }
  c.tz = TIMEZONE;
  return c;
}

void begin() {
  s_lock = xSemaphoreCreateMutex();
  s_p.begin("nw3dnet", false);
  DevCfg c;
  if (s_p.getUChar("v", 0) == 0) {
    c = fromConfigH();
    save(c);
  } else {
    c = load();
  }
  xSemaphoreTake(s_lock, portMAX_DELAY);
  s_cur = c;
  xSemaphoreGive(s_lock);
}

DevCfg get() {
  xSemaphoreTake(s_lock, portMAX_DELAY);
  DevCfg c = s_cur;
  xSemaphoreGive(s_lock);
  return c;
}

void set(const DevCfg& c) {
  xSemaphoreTake(s_lock, portMAX_DELAY);
  s_cur = c;
  s_ver = s_ver + 1;
  xSemaphoreGive(s_lock);
  save(c);
}

uint32_t version() { return s_ver; }

void factoryReset() {
  s_p.clear();
  s_p.putUChar("v", 1);   // config.h uz znovu nepouzivat
  DevCfg c;
  save(c);
  xSemaphoreTake(s_lock, portMAX_DELAY);
  s_cur = c;
  s_ver = s_ver + 1;
  xSemaphoreGive(s_lock);
}

}  // namespace cfg

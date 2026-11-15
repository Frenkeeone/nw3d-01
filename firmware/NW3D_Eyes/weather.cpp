#include "weather.h"
#include <ArduinoJson.h>
#include <math.h>

#ifdef ARDUINO
#include "cfg.h"
#include "i18n.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#endif

namespace weather {

bool parse(const char* json, size_t len, WeatherInfo& w) {
  JsonDocument d;
  if (deserializeJson(d, json, len)) return false;
  JsonObject cur = d["current"];
  if (cur.isNull()) return false;
  w.temp     = cur["temperature_2m"] | NAN;
  w.code     = cur["weather_code"] | 0;
  w.isDay    = (cur["is_day"] | 1) == 1;
  w.wind     = cur["wind_speed_10m"] | 0.0f;
  w.humidity = cur["relative_humidity_2m"] | -1;
  JsonObject day = d["daily"];
  w.tmin     = day["temperature_2m_min"][0] | NAN;
  w.tmax     = day["temperature_2m_max"][0] | NAN;
  w.rainProb = day["precipitation_probability_max"][0] | -1;
  w.valid = !isnan(w.temp);
  return w.valid;
}

#ifdef ARDUINO
static WeatherInfo s_w;
static SemaphoreHandle_t s_lock;
static uint32_t s_next = 0;

// Stazeni pres HTTP (bez sifrovani - nepotrebuje pamet pro TLS) nebo HTTPS
static int fetch(const char* url, bool tls, String& body) {
  WiFiClient plain;
  WiFiClientSecure sec;
  HTTPClient http;
  http.setTimeout(8000);
  http.setReuse(false);
  bool ok;
  if (tls) { sec.setInsecure(); ok = http.begin(sec, url); }
  else     { ok = http.begin(plain, url); }
  if (!ok) return -100;
  int code = http.GET();
  if (code == 200) body = http.getString();
  http.end();
  return code;
}

static void setStatus(const char* fmt, ...) {
  char buf[sizeof(s_w.status)];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  xSemaphoreTake(s_lock, portMAX_DELAY);
  strncpy(s_w.status, buf, sizeof(s_w.status) - 1);
  s_w.status[sizeof(s_w.status) - 1] = 0;
  xSemaphoreGive(s_lock);
  Serial.printf("[pocasi] %s\n", buf);
}

static float s_lat = NAN, s_lon = NAN;

void loop() {
  if (!s_lock) s_lock = xSemaphoreCreateMutex();
  DevCfg c = cfg::get();
  if (!c.hasLocation()) {
    if (!isnan(s_lat) || s_w.valid || strcmp(s_w.status, tr(S_SET_CITY)) != 0) {
      xSemaphoreTake(s_lock, portMAX_DELAY);
      s_w = WeatherInfo();
      snprintf(s_w.status, sizeof s_w.status, "%s", tr(S_SET_CITY));
      xSemaphoreGive(s_lock);
      s_lat = s_lon = NAN;
    }
    return;
  }
  if (c.lat != s_lat || c.lon != s_lon) { s_lat = c.lat; s_lon = c.lon; s_next = 0; }   // nove misto
  if (s_next && (int32_t)(millis() - s_next) < 0) return;

  char query[240];
  snprintf(query, sizeof query,
           "api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,weather_code,is_day,wind_speed_10m,relative_humidity_2m"
           "&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max"
           "&timezone=auto&forecast_days=1",
           (double)s_lat, (double)s_lon);
  char url[260];
  String body;
  int code = -1;
  bool ok = false;
  for (int attempt = 0; attempt < 2 && !ok; attempt++) {
    bool tls = attempt == 1;
    snprintf(url, sizeof url, "%s://%s", tls ? "https" : "http", query);
    body = "";
    code = fetch(url, tls, body);
    if (code == 200) {
      WeatherInfo w;
      if (parse(body.c_str(), body.length(), w)) {
        xSemaphoreTake(s_lock, portMAX_DELAY);
        s_w = w;
        s_w.status[0] = 0;
        xSemaphoreGive(s_lock);
        Serial.printf("[pocasi] %.1f C, kod %d (%s)\n", w.temp, w.code, tls ? "https" : "http");
        ok = true;
      } else {
        setStatus("chybná data (%u B)", (unsigned)body.length());
      }
    } else {
      setStatus("%s: %s (%d), volná RAM %u kB", tls ? "https" : "http",
                code == -100 ? "nelze spojit" : HTTPClient::errorToString(code).c_str(), code,
                (unsigned)(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL) / 1024));
    }
  }
  s_next = millis() + (ok ? 15UL * 60 * 1000 : 60UL * 1000);
}

WeatherInfo snapshot() {
  WeatherInfo out;
  if (!s_lock) return out;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  out = s_w;
  xSemaphoreGive(s_lock);
  return out;
}
#endif

}  // namespace weather

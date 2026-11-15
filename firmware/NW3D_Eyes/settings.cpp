#include "settings.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>

namespace settings {

static Settings s_cur;
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;
static volatile uint32_t s_ver = 1;
static Preferences s_prefs;

static Settings defaults() {
  Settings d;
  d.showEyes = SHOW_EYES; d.showClock = SHOW_CLOCK; d.showWeather = SHOW_WEATHER;
  d.screenSec = SCREEN_SECONDS;
  d.bright = BRIGHTNESS; d.nightBright = NIGHT_BRIGHTNESS;
  d.nightFrom = NIGHT_FROM; d.nightTo = NIGHT_TO;
  d.rotation = EYE_ROTATION & 3; d.swapEyes = SWAP_EYES;
  d.finishedMin = FINISHED_MINUTES;
  // kdo vyplnil config.h (stary zpusob), chce cestinu a pruvodce nepotrebuje
  if (strcmp(WIFI_SSID, "nazev_wifi") != 0 && strlen(WIFI_SSID) > 0) { d.lang = 0; }
#ifdef QUOTE_SYMBOLS
  snprintf(d.tickers, sizeof d.tickers, "%s", QUOTE_SYMBOLS);
#endif
  return d;
}

void begin() {
  Settings d = defaults();
  s_prefs.begin("nw3d", false);
  // starsi verze ulozila kratsi strukturu - nacte se zacatek, nove polozky zustanou vychozi
  size_t len = s_prefs.getBytesLength("cfg2");
  if (len > 0 && len <= sizeof(Settings)) s_prefs.getBytes("cfg2", &d, len);
  portENTER_CRITICAL(&s_mux);
  s_cur = d;
  portEXIT_CRITICAL(&s_mux);
}

Settings get() {
  portENTER_CRITICAL(&s_mux);
  Settings c = s_cur;
  portEXIT_CRITICAL(&s_mux);
  return c;
}

void set(const Settings& s) {
  portENTER_CRITICAL(&s_mux);
  s_cur = s;
  s_ver = s_ver + 1;
  portEXIT_CRITICAL(&s_mux);
  s_prefs.putBytes("cfg2", &s, sizeof(Settings));
}

uint32_t version() { return s_ver; }

void reset() {
  Settings old = get();
  s_prefs.remove("cfg2");
  Settings d = defaults();
  d.lang = old.lang;              // jazyk a dokonceny pruvodce zustavaji
  d.setupDone = old.setupDone;
  portENTER_CRITICAL(&s_mux);
  s_cur = d;
  s_ver = s_ver + 1;
  portEXIT_CRITICAL(&s_mux);
  s_prefs.putBytes("cfg2", &d, sizeof(Settings));
}

void wipe() {
  s_prefs.remove("cfg2");
  Settings d = defaults();
  portENTER_CRITICAL(&s_mux);
  s_cur = d;
  s_ver = s_ver + 1;
  portEXIT_CRITICAL(&s_mux);
}

}  // namespace settings

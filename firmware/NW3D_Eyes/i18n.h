// Preklady textu na displejich a hlaseni (cestina / anglictina).
#pragma once
#include <stdint.h>

enum Lang : uint8_t { LANG_CS = 0, LANG_EN = 1, LANG_COUNT };

enum Str : uint16_t {
  // testovaci obrazovka
  S_UP, S_LEFT_EYE, S_RIGHT_EYE,
  // pocasi
  S_WEATHER, S_RAIN_PCT, S_SET_CITY,
  S_W_CLEAR, S_W_MOSTLY_CLEAR, S_W_PARTLY, S_W_CLOUDY, S_W_FOG, S_W_DRIZZLE, S_W_RAIN, S_W_SHOWER,
  S_W_SNOW, S_W_STORM,
  // tisk
  S_ENDS_IN, S_PRINTING, S_PAUSED, S_PREPARING, S_AT_TIME, S_AT_TIME_DAYS, S_LAYER, S_NOZZLE_DEG,
  S_DONE, S_ERROR, S_CHECK_PRINTER, S_LEFT_TIME, S_DONE_AT, S_NOZZLE, S_BED, S_NOTHING_PRINTING,
  S_MODEL, S_NO_PREVIEW,
  // kurzy
  S_NO_CHART, S_NO_QUOTE,
  // sit a nastaveni
  S_WIFI, S_CONNECTING, S_SETUP, S_JOIN_WIFI, S_THEN_OPEN, S_SET_ME_UP, S_PRINTER,
  S_UPDATING,
  // stav tiskarny (dva radky oddelene |)
  S_P_NOT_SET, S_P_CONNECTING, S_P_CLOUD_DOWN, S_P_TOKEN_REJECTED, S_P_NO_UID, S_P_DENIED_TOKEN,
  S_P_DENIED_CODE, S_P_UNREACH_CLOUD, S_P_UNREACH_LAN, S_P_NO_DATA_LAN, S_P_NO_DATA_CLOUD,
  // nazvy obrazovek pro web
  S_SCR_EYES, S_SCR_CLOCK, S_SCR_QUOTES, S_SCR_PRINT, S_SCR_FINISHED, S_SCR_FAILED,
  S_SCR_PRINTER, S_SCR_TEST, S_SCR_OFF, S_SCR_SETUP,
  S_COUNT
};

namespace i18n {
void        setLang(uint8_t lang);
uint8_t     lang();
const char* tr(Str s);
const char* weekday(int wday);      // 0 = nedele
const char* month(int month);       // 1-12, cesky v 2. pade ("listopadu")
const char* widgetName(uint8_t w);  // nazev obsahu oka (pro web)
}

inline const char* tr(Str s) { return i18n::tr(s); }

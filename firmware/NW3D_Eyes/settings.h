// Nastaveni menitelna z weboveho rozhrani. Ukladaji se do pameti desky (NVS),
// vychozi hodnoty jsou z config.h.
#pragma once
#include <stdint.h>

// Co muze ukazovat jedno oko
enum Widget : uint8_t {
  W_AUTO = 0,        // stridani (oci / hodiny / pocasi) - jen bez tisku
  W_EYE,             // animovane oko
  W_CLOCK,           // hodiny HH:MM
  W_HOURS,           // velke hodiny
  W_MINUTES,         // velke minuty
  W_DATE,            // den a datum
  W_WEATHER_ICON,    // pocasi - obrazek
  W_WEATHER_TEMP,    // pocasi - teplota
  W_PRINT_TIME,      // tisk - zbyvajici cas
  W_PRINT_PERCENT,   // tisk - procenta a vrstva
  W_PRINT_COMBO,     // tisk - procenta i cas v jednom oku
  W_PRINT_TEMPS,     // tisk - teplota trysky a podlozky
  W_OFF,             // cerna
  W_QUOTE_PRICE,     // kurz - cena a zmena
  W_QUOTE_CHART,     // kurz - graf dne
  W_MODEL,           // nahled tisteneho modelu (Bambu cloud)
  W_COUNT
};

struct Settings {
  uint8_t  printW[2] = {W_PRINT_TIME, W_PRINT_PERCENT};  // pri tisku: leve, prave
  uint8_t  idleW[2]  = {W_AUTO, W_AUTO};                 // bez tisku: leve, prave
  bool     showEyes = true, showClock = true, showWeather = true;
  uint16_t screenSec = 15;
  uint8_t  bright = 80, nightBright = 10, nightFrom = 22, nightTo = 7;
  uint8_t  rotation = 0;
  bool     swapEyes = false;
  uint16_t finishedMin = 30;
  // --- pridano ve verzi 1.3 (musi zustat na konci kvuli starym ulozenym nastavenim) ---
  bool     showQuotes = true;
  char     tickers[48] = "BZ=F";   // symboly Yahoo Finance oddelene carkou (max 4)
  // --- pridano ve verzi 2.0 ---
  uint8_t  lang = 0xFF;            // 0 = cestina, 1 = anglictina, 0xFF = jeste nevybrano
  uint8_t  volume = 60;            // hlasitost 0-100 (0 = bez zvuku)
  bool     soundFinish = true;     // zvuk po dotisku
  bool     soundError = true;      // zvuk pri chybe tisku
  bool     setupDone = false;      // pruvodce nastavenim dokoncen
};

namespace settings {
void     begin();                 // nacte z NVS (nebo z config.h)
Settings get();                   // kopie (vlaknove bezpecne)
void     set(const Settings& s);  // ulozi a zvysi cislo verze
uint32_t version();               // meni se pri kazde zmene
void     reset();                 // vzhled zpet na vychozi hodnoty
void     wipe();                  // uplne smazani (tovarni nastaveni)
}

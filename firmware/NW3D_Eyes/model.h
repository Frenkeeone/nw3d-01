// Datove struktury sdilene mezi sitovou casti a vykreslovanim.
#pragma once
#include <stdint.h>

enum class Phase : uint8_t { Unknown, Idle, Preparing, Printing, Paused, Finished, Failed };

struct PrintInfo {
  bool     configured  = false;   // je vubec nastavena tiskarna
  bool     online      = false;   // MQTT spojeni navazano
  bool     hasData     = false;   // prisla aspon jedna zprava o stavu
  Phase    phase       = Phase::Unknown;
  int      percent     = -1;      // 0-100
  int      remainingMin= -1;      // zbyvajici minuty
  int      layer       = -1;
  int      totalLayers = -1;
  float    nozzle      = -1;      // C
  float    bed         = -1;      // C
  uint32_t error       = 0;       // print_error
  uint32_t lastMsgMs   = 0;       // millis() posledni zpravy z tiskarny
  char     job[48]     = "";
  char     status[64]  = "";      // lidsky citelny stav spojeni / chyby
};

struct WeatherInfo {
  bool  valid = false;
  float temp = 0, tmin = 0, tmax = 0, wind = 0;
  int   code = 0;          // WMO kod pocasi
  bool  isDay = true;
  int   rainProb = -1;     // % pravdepodobnost srazek dnes
  int   humidity = -1;
  char  status[48] = "...";   // posledni chyba (pro web)
};

// Kurz akcie / komodity / meny (Yahoo Finance)
struct QuoteInfo {
  bool  valid = false;
  char  sym[16] = "";       // symbol Yahoo, napr. "BZ=F"
  char  name[24] = "";      // zobrazovany nazev
  char  unit[16] = "";      // mena nebo jednotka, napr. "USD/barel"
  float price = 0;          // posledni cena
  float prev = 0;           // zaviraci cena predchoziho dne
  float spark[48];          // prubeh za posledni obchodni den
  uint8_t nspark = 0;
  char  status[48] = "...";
};

struct ClockInfo {
  bool valid = false;
  int  hour = 0, minute = 0, second = 0;
  int  wday = 0;           // 0 = nedele
  int  day = 1, month = 1;
};

struct EyeAnim {
  float gx = 0, gy = 0;    // smer pohledu v pixelech
  float open = 1;          // 0 = zavrene, 1 = otevrene
};

enum class Mood : uint8_t { Normal, Sleepy, Happy };

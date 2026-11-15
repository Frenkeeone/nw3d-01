// Wi-Fi: pripojeni k domaci siti, zalozni vlastni sit "NW3D-xxxx" s nastavovaci strankou,
// vyhledani siti a zadani Wi-Fi pres USB hned po instalaci (protokol Improv).
#pragma once
#include <Arduino.h>

namespace wifimgr {

enum class State : uint8_t { Connecting, Connected, Portal };

void begin();                 // po startu (spusti pripojeni nebo nastavovaci sit)
void loop();                  // v sitove uloze
State state();
bool connected();
String apName();              // nazev nastavovaci site
String ip();                  // IP v domaci siti (nebo 192.168.4.1 v nastavovaci siti)

// Nove udaje - ulozi a pripoji se. wait > 0: ceka na vysledek (ms) a vrati, jestli se povedlo.
bool connectTo(const String& ssid, const String& pass, uint32_t waitMs);

// Vyhledani siti: JSON pole [{"s":"nazev","r":-60,"l":1}, ...]
String scanJson();
int    scanCount();
bool   scanItem(int i, String& ssid, int& rssi, bool& locked);

void improvLoop();            // obsluha Improv na USB (vola se z loop())
}

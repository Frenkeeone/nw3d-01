// Nahled prave tisteneho modelu (obrazek ulohy z Bambu cloudu).
#pragma once
#include <LovyanGFX.hpp>

namespace cover {
constexpr int SIZE = 150;                 // velikost obrazku v px

void loop();                              // v sitove uloze: po startu tisku stahne obrazek
bool draw(LGFX_Sprite& dst, int x, int y);  // vykresli obrazek (false = neni k dispozici)
bool available();
const char* status();                     // posledni stav (pro web)
}

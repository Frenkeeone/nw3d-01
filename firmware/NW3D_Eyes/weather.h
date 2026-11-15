// Pocasi z Open-Meteo (zdarma, bez API klice).
#pragma once
#include <stddef.h>
#include "model.h"

namespace weather {
void loop();                 // v sitove uloze; stahuje kazdych 15 minut
WeatherInfo snapshot();
bool parse(const char* json, size_t len, WeatherInfo& w);  // rozbor odpovedi (testovatelne na PC)
}

// Kurzy akcii, komodit, men a kryptomen z Yahoo Finance (zdarma, bez klice).
#pragma once
#include <stddef.h>
#include "model.h"

namespace quotes {
static constexpr int MAX = 4;

void loop();                       // v sitove uloze; kazdy symbol obnovi jednou za 10 minut
int  count();                      // pocet nastavenych symbolu
QuoteInfo get(int i);              // kopie (vlaknove bezpecne)
bool parse(const char* json, size_t len, QuoteInfo& q);   // rozbor odpovedi (testovatelne na PC)
const char* presetName(const char* sym);                   // cesky nazev znamych symbolu
const char* presetUnit(const char* sym);
}

// Vykreslovani obrazovek pro jedno oko (240 x 240 px, kulaty displej).
// Nezavisi na hardware - stejny kod se da prelozit i na PC pro nahledy.
#pragma once
#include <LovyanGFX.hpp>
#include "model.h"

namespace ui {

enum Icon { ICON_WIFI, ICON_PRINTER, ICON_WARN };

void begin();  // nacte pisma

void boot     (LGFX_Sprite& s, int eye, int rotation, const char* version, const char* ip = nullptr);
void eyes     (LGFX_Sprite& s, int eye, const EyeAnim& a, Mood mood);
void clock    (LGFX_Sprite& s, int eye, const ClockInfo& c);
void weather  (LGFX_Sprite& s, int eye, const WeatherInfo& w);
void print    (LGFX_Sprite& s, int eye, const PrintInfo& p, const ClockInfo& c, uint32_t ms);
void finished (LGFX_Sprite& s, int eye, const char* atTime);
void failed   (LGFX_Sprite& s, int eye, uint32_t error);
void message  (LGFX_Sprite& s, int eye, Icon icon, const char* l1, const char* l2, const char* l3);
void statusDot(LGFX_Sprite& s, uint32_t rgb);

// Samostatne obsahy pro jedno oko (lze kombinovat libovolne)
void clockHM    (LGFX_Sprite& s, const ClockInfo& c);                 // HH:MM
void dateDay    (LGFX_Sprite& s, const ClockInfo& c);                 // den v tydnu + datum
void printCombo (LGFX_Sprite& s, const PrintInfo& p, const ClockInfo& c, uint32_t ms);  // % + zbyva
void printTemps (LGFX_Sprite& s, const PrintInfo& p, uint32_t ms);    // tryska / podlozka
void printerIdle(LGFX_Sprite& s);                                     // tiskarna necinna
void blank      (LGFX_Sprite& s);
void quotePrice (LGFX_Sprite& s, const QuoteInfo& q);                 // kurz: nazev, cena, zmena
void quoteChart (LGFX_Sprite& s, const QuoteInfo& q);                 // kurz: graf dne
void formatPrice(char* buf, int n, float v);                          // 1 234,56
void modelBase   (LGFX_Sprite& s, const PrintInfo& p, uint32_t ms, bool printing);
void modelOverlay(LGFX_Sprite& s, const PrintInfo& p, bool printing, bool hasImage);
void setup      (LGFX_Sprite& s, int eye, bool portal, const char* apName, const char* ip, uint32_t ms);
void updating   (LGFX_Sprite& s, int pct);

}  // namespace ui

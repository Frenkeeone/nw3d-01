// Hardware desky Waveshare ESP32-S3-DualEye-LCD-1.28: dva displeje GC9A01,
// podsviceni a tlacitko BOOT.
#pragma once
#include <Arduino.h>

namespace board {
constexpr int W = 240, H = 240;

bool begin(int rotation, bool swapEyes);        // inicializace obou displeju
void setOrientation(int rotation, bool swapEyes);  // zmena natoceni za behu
void pushFrame(int eye, const void* rgb565be);  // eye 0 = leve, 1 = prave
void setBrightness(uint8_t percent);            // 0-100
bool bootPressed();                              // tlacitko BOOT (GPIO0)
}  // namespace board

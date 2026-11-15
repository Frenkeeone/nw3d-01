// Zvuky z reproduktoru desky (kodek ES8311): po dotisku, pri chybe, test.
#pragma once
#include <stdint.h>

namespace audio {
enum Tune : uint8_t { TUNE_FINISH, TUNE_ERROR, TUNE_TEST };
void begin();
void play(Tune t, uint8_t volume);   // neblokuje; hlasitost 0-100 (0 = nic)
}

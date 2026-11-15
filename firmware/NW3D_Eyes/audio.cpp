#include "audio.h"
#include <Arduino.h>
#include <Wire.h>
#include <ESP_I2S.h>
#include <math.h>
#include "es8311.h"

// Piny podle ukazek Waveshare (ESP32-S3-DualEye-LCD-1.28)
#define PIN_I2C_SDA 11
#define PIN_I2C_SCL 10
#define PIN_MCLK    12
#define PIN_BCLK    13
#define PIN_LRCK    14
#define PIN_DIN     15
#define PIN_DOUT    16
#define PIN_PA       9
#define RATE     16000

namespace audio {

static I2SClass s_i2s;
static es8311_handle_t s_codec = nullptr;
static QueueHandle_t s_q;
static bool s_ok = false;

struct Note { float hz; uint16_t ms; };
static const Note FINISH[] = {{523.25f, 120}, {659.25f, 120}, {783.99f, 120}, {1046.5f, 320}, {0, 80}, {783.99f, 110}, {1046.5f, 380}};
static const Note ERROR_[] = {{659.25f, 220}, {523.25f, 220}, {392.0f, 420}};
static const Note TEST[]   = {{880.0f, 150}, {0, 60}, {1318.5f, 220}};

static void tone(float hz, uint16_t ms, float amp) {
  const int N = 256;
  int16_t buf[N * 2];
  uint32_t total = (uint32_t)RATE * ms / 1000;
  uint32_t fade = RATE / 200;     // 5 ms nabeh/dobeh, at to necvaka
  float ph = 0, step = 2.0f * (float)M_PI * hz / RATE;
  for (uint32_t i = 0; i < total;) {
    int n = 0;
    for (; n < N && i < total; n++, i++) {
      float env = 1.0f;
      if (i < fade) env = (float)i / fade;
      else if (total - i < fade) env = (float)(total - i) / fade;
      // trocha vyssi harmonicke, zni to mene "pipave"
      float v = hz > 0 ? (sinf(ph) + 0.25f * sinf(2 * ph)) * 0.8f * env * amp : 0;
      ph += step;
      if (ph > 2 * (float)M_PI) ph -= 2 * (float)M_PI;
      int16_t s = (int16_t)(v * 32767);
      buf[2 * n] = s;
      buf[2 * n + 1] = s;
    }
    s_i2s.write((uint8_t*)buf, n * 4);
  }
}

static void task(void*) {
  uint16_t msg;
  for (;;) {
    if (xQueueReceive(s_q, &msg, portMAX_DELAY) != pdTRUE) continue;
    uint8_t tune = msg & 0xFF, vol = msg >> 8;
    if (!s_ok || vol == 0) continue;
    const Note* notes; int n;
    switch (tune) {
      case TUNE_FINISH: notes = FINISH; n = sizeof(FINISH) / sizeof(Note); break;
      case TUNE_ERROR:  notes = ERROR_; n = sizeof(ERROR_) / sizeof(Note); break;
      default:          notes = TEST;   n = sizeof(TEST) / sizeof(Note); break;
    }
    es8311_voice_volume_set(s_codec, 40 + vol * 45 / 100, nullptr);
    digitalWrite(PIN_PA, HIGH);
    vTaskDelay(pdMS_TO_TICKS(30));
    float amp = 0.25f + 0.55f * vol / 100.0f;
    for (int i = 0; i < n; i++) tone(notes[i].hz, notes[i].ms, amp);
    tone(0, 60, 0);
    vTaskDelay(pdMS_TO_TICKS(40));
    digitalWrite(PIN_PA, LOW);
  }
}

void begin() {
  pinMode(PIN_PA, OUTPUT);
  digitalWrite(PIN_PA, LOW);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  s_i2s.setPins(PIN_BCLK, PIN_LRCK, PIN_DOUT, PIN_DIN, PIN_MCLK);
  if (!s_i2s.begin(I2S_MODE_STD, RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("[zvuk] I2S se nepodarilo spustit");
    return;
  }
  s_codec = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
  es8311_clock_config_t clk = {};
  clk.mclk_inverted = false;
  clk.sclk_inverted = false;
  clk.mclk_from_mclk_pin = true;
  clk.mclk_frequency = RATE * 256;
  clk.sample_frequency = RATE;
  if (!s_codec || es8311_init(s_codec, &clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16) != ESP_OK) {
    Serial.println("[zvuk] kodek ES8311 neodpovida");
    return;
  }
  es8311_microphone_config(s_codec, false);
  s_q = xQueueCreate(4, sizeof(uint16_t));
  xTaskCreatePinnedToCore(task, "audio", 4096, nullptr, 2, nullptr, 0);
  s_ok = true;
  Serial.println("[zvuk] pripraven");
}

void play(Tune t, uint8_t volume) {
  if (!s_ok || volume == 0) return;
  uint16_t msg = (uint16_t)t | ((uint16_t)(volume > 100 ? 100 : volume) << 8);
  xQueueSend(s_q, &msg, 0);
}

}  // namespace audio

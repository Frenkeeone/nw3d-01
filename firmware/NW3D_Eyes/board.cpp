#include "board.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_gc9a01.h"
#include "freertos/semphr.h"

// Piny podle ukazek Waveshare (ESP32-S3-DualEye-LCD-1.28)
#define PIN_MISO      40
#define PIN_MOSI      42
#define PIN_SCLK      41
#define PIN_DC        45
#define PIN_LCD1_CS   47
#define PIN_LCD1_RST  48
#define PIN_LCD1_BL   46
#define PIN_LCD2_CS   38
#define PIN_LCD2_RST   8
#define PIN_LCD2_BL   39
#define PIN_BOOT       0

#define LCD_HOST   SPI2_HOST
#define PCLK_HZ    (80 * 1000 * 1000)
#define STRIP      24   // radku na jeden prenos DMA

namespace board {

static esp_lcd_panel_handle_t s_panel[2] = {nullptr, nullptr};
static int s_map[2] = {0, 1};
static SemaphoreHandle_t s_free;     // pocet volnych DMA bufferu
static uint8_t* s_dma[2];
static int s_next = 0;

static bool IRAM_ATTR onColorDone(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t*, void*) {
  BaseType_t woken = pdFALSE;
  xSemaphoreGiveFromISR(s_free, &woken);
  return woken == pdTRUE;
}

static esp_lcd_panel_handle_t initPanel(int cs, int rst) {
  esp_lcd_panel_io_handle_t io = nullptr;
  esp_lcd_panel_io_spi_config_t io_cfg = {};
  io_cfg.cs_gpio_num = cs;
  io_cfg.dc_gpio_num = PIN_DC;
  io_cfg.spi_mode = 0;
  io_cfg.pclk_hz = PCLK_HZ;
  io_cfg.trans_queue_depth = 4;
  io_cfg.on_color_trans_done = onColorDone;
  io_cfg.lcd_cmd_bits = 8;
  io_cfg.lcd_param_bits = 8;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_cfg, &io));

  esp_lcd_panel_dev_config_t cfg = {};
  cfg.reset_gpio_num = rst;
  cfg.rgb_endian = LCD_RGB_ENDIAN_BGR;
  cfg.bits_per_pixel = 16;
  esp_lcd_panel_handle_t p = nullptr;
  ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io, &cfg, &p));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(p));
  ESP_ERROR_CHECK(esp_lcd_panel_init(p));
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(p, true));
  return p;
}

// Natoceni podle tabulky Waveshare: [rotace][panel] = {mirror_x, mirror_y, swap_xy}
static const bool ORIENT[4][2][3] = {
  {{true,  false, false}, {false, true,  false}},   //   0 st.
  {{false, false, true }, {true,  true,  true }},   //  90 st.
  {{false, true,  false}, {true,  false, false}},   // 180 st.
  {{true,  true,  true }, {false, false, true }},   // 270 st.
};

bool begin(int rotation, bool swapEyes) {
  spi_bus_config_t bus = {};
  bus.mosi_io_num = PIN_MOSI;
  bus.miso_io_num = PIN_MISO;
  bus.sclk_io_num = PIN_SCLK;
  bus.quadwp_io_num = -1;
  bus.quadhd_io_num = -1;
  bus.max_transfer_sz = W * STRIP * 2 + 64;
  if (spi_bus_initialize(LCD_HOST, &bus, SPI_DMA_CH_AUTO) != ESP_OK) return false;

  s_free = xSemaphoreCreateCounting(2, 2);
  for (auto& b : s_dma) {
    b = (uint8_t*)heap_caps_malloc(W * STRIP * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!b) return false;
  }

  s_panel[0] = initPanel(PIN_LCD1_CS, PIN_LCD1_RST);
  s_panel[1] = initPanel(PIN_LCD2_CS, PIN_LCD2_RST);
  setOrientation(rotation, swapEyes);
  for (int i = 0; i < 2; i++) esp_lcd_panel_disp_on_off(s_panel[i], true);

  ledcAttach(PIN_LCD1_BL, 20000, 10);
  ledcAttach(PIN_LCD2_BL, 20000, 10);
  setBrightness(0);
  pinMode(PIN_BOOT, INPUT_PULLUP);
  return true;
}

void setOrientation(int rotation, bool swapEyes) {
  rotation &= 3;
  for (int i = 0; i < 2; i++) {
    if (!s_panel[i]) continue;
    esp_lcd_panel_mirror(s_panel[i], ORIENT[rotation][i][0], ORIENT[rotation][i][1]);
    esp_lcd_panel_swap_xy(s_panel[i], ORIENT[rotation][i][2]);
  }
  s_map[0] = swapEyes ? 1 : 0;
  s_map[1] = swapEyes ? 0 : 1;
}

void pushFrame(int eye, const void* fb) {
  esp_lcd_panel_handle_t p = s_panel[s_map[eye & 1]];
  if (!p) return;
  const uint8_t* src = (const uint8_t*)fb;
  const size_t strip = (size_t)W * STRIP * 2;
  for (int y = 0; y < H; y += STRIP) {
    xSemaphoreTake(s_free, portMAX_DELAY);
    uint8_t* b = s_dma[s_next];
    s_next ^= 1;
    memcpy(b, src + (size_t)y * W * 2, strip);
    esp_lcd_panel_draw_bitmap(p, 0, y, W, y + STRIP, b);
  }
  // pockat na dokonceni, at dalsi displej zacina na volne sbernici
  xSemaphoreTake(s_free, portMAX_DELAY);
  xSemaphoreTake(s_free, portMAX_DELAY);
  xSemaphoreGive(s_free);
  xSemaphoreGive(s_free);
  s_next = 0;
}

void setBrightness(uint8_t percent) {
  if (percent > 100) percent = 100;
  uint32_t duty = (uint32_t)percent * 1023 / 100;
  ledcWrite(PIN_LCD1_BL, duty);
  ledcWrite(PIN_LCD2_BL, duty);
}

bool bootPressed() { return digitalRead(PIN_BOOT) == LOW; }

}  // namespace board

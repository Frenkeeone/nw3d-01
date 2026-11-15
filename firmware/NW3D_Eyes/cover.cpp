#include "cover.h"
#include "cfg.h"
#include "bambu.h"
#include "printer.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

namespace cover {

static LGFX_Sprite s_img;
static bool s_ok = false;
static SemaphoreHandle_t s_lock;
static char s_job[48] = "";
static char s_status[64] = "-";
static uint32_t s_due = 0;
static int s_tries = 0;

static void setStatus(const char* t) {
  snprintf(s_status, sizeof s_status, "%s", t);
  Serial.printf("[nahled] %s\n", t);
}

const char* status() { return s_status; }
bool available() { return s_ok; }

bool draw(LGFX_Sprite& dst, int x, int y) {
  if (!s_lock || !s_ok) return false;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  s_img.pushSprite(&dst, x, y, (uint16_t)0);   // cerne pozadi pruhledne
  xSemaphoreGive(s_lock);
  return true;
}

// Zapis stahovanych dat do bufferu v PSRAM (HTTPClient::writeToStream zvlada i "chunked" prenos)
class BufStream : public Stream {
 public:
  uint8_t* buf = nullptr;
  size_t len = 0, cap = 0;
  bool overflow = false;
  size_t write(uint8_t c) override { return write(&c, 1); }
  size_t write(const uint8_t* d, size_t n) override {
    if (len + n > cap) {
      size_t nc = cap ? cap * 2 : 64 * 1024;
      while (nc < len + n) nc *= 2;
      if (nc > 1024 * 1024) { overflow = true; return 0; }
      uint8_t* nb = (uint8_t*)heap_caps_realloc(buf, nc, MALLOC_CAP_SPIRAM);
      if (!nb) { overflow = true; return 0; }
      buf = nb;
      cap = nc;
    }
    memcpy(buf + len, d, n);
    len += n;
    return n;
  }
  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }
  void flush() override {}
};

// Stazeni obrazku do PSRAM. Vraci delku (0 = chyba).
static size_t download(const String& url, uint8_t*& buf) {
  buf = nullptr;
  WiFiClientSecure c;
  c.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(c, url)) return 0;
  int code = http.GET();
  if (code != 200) { http.end(); return 0; }
  BufStream bs;
  int r = http.writeToStream(&bs);
  http.end();
  if (r <= 0 || bs.overflow || bs.len < 16) { free(bs.buf); return 0; }
  buf = bs.buf;
  return bs.len;
}

static bool decode(const uint8_t* d, size_t n) {
  LGFX_Sprite tmp;
  tmp.setPsram(true);
  tmp.setColorDepth(16);
  if (!tmp.createSprite(SIZE, SIZE)) return false;
  tmp.fillScreen(0x0000);
  bool ok = false;
  if (n > 8 && d[0] == 0x89 && d[1] == 'P' && d[2] == 'N' && d[3] == 'G')
    ok = tmp.drawPng(d, n, 0, 0, SIZE, SIZE, 0, 0, 0.0f, 0.0f, lgfx::datum_t::middle_center);
  else if (n > 4 && d[0] == 0xFF && d[1] == 0xD8)
    ok = tmp.drawJpg(d, n, 0, 0, SIZE, SIZE, 0, 0, 0.0f, 0.0f, lgfx::datum_t::middle_center);
  if (ok) {
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (!s_img.getBuffer()) { s_img.setPsram(true); s_img.setColorDepth(16); s_img.createSprite(SIZE, SIZE); }
    if (s_img.getBuffer()) {
      memcpy(s_img.getBuffer(), tmp.getBuffer(), SIZE * SIZE * 2);
      s_ok = true;
    }
    xSemaphoreGive(s_lock);
  }
  tmp.deleteSprite();
  return ok;
}

void loop() {
  if (!s_lock) s_lock = xSemaphoreCreateMutex();
  PrintInfo p = printer::snapshot();
  bool active = p.hasData && (p.phase == Phase::Printing || p.phase == Phase::Preparing || p.phase == Phase::Paused);
  if (!active || !p.job[0]) return;
  if (strcmp(p.job, s_job) != 0) {           // nova uloha
    snprintf(s_job, sizeof s_job, "%s", p.job);
    s_ok = false;
    s_tries = 0;
    s_due = millis() + 15000;                 // chvili pockat, nez ji cloud zaregistruje
    setStatus("...");
  }
  if (!s_due || (int32_t)(millis() - s_due) < 0) return;
  s_due = 0;

  DevCfg c = cfg::get();
  if (c.mode != PM_CLOUD || c.token.isEmpty()) { setStatus("cloud only"); return; }
  String title, url;
  if (!bambu::lastTask(c.token, c.region, c.sn, title, url)) {
    setStatus("task not found");
  } else {
    uint8_t* buf;
    size_t n = download(url, buf);
    if (!n) setStatus("download failed");
    else {
      bool ok = decode(buf, n);
      free(buf);
      setStatus(ok ? "ok" : "unknown image format");
      if (ok) return;
    }
  }
  if (++s_tries < 4) s_due = millis() + 60000;   // zkusit znovu
}

}  // namespace cover

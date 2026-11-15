#include "ui.h"
#include "fonts_vlw.h"
#include "i18n.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace ui {

// --------------------------------------------------------------------
//  Barvy (RGB888) a rozmery
// --------------------------------------------------------------------
static constexpr uint32_t BG       = 0x000000u;
static constexpr uint32_t TRACK    = 0x12212Bu;
static constexpr uint32_t TICK     = 0x223441u;
static constexpr uint32_t CYAN     = 0x18E0D8u;   // barva loga
static constexpr uint32_t AMBER    = 0xFFB020u;
static constexpr uint32_t GREEN    = 0x3DDC84u;
static constexpr uint32_t RED      = 0xFF4545u;
static constexpr uint32_t WHITE    = 0xF2F6F8u;
static constexpr uint32_t DIM      = 0x8396A6u;
static constexpr uint32_t LID_EDGE = 0x1E2C36u;

static constexpr int CX = 120, CY = 120;
static constexpr int RING_IN = 106, RING_OUT = 118;

// --------------------------------------------------------------------
//  Pisma (VLW z fonts_vlw.h, cesky)
// --------------------------------------------------------------------
static lgfx::PointerWrapper wSmall, wMed, wBig, wHuge;
static lgfx::VLWfont fSmall, fMed, fBig, fHuge;

void begin() {
  wSmall.set(font_small, sizeof(font_small)); fSmall.loadFont(&wSmall);
  wMed.set(font_med, sizeof(font_med));       fMed.loadFont(&wMed);
  wBig.set(font_big, sizeof(font_big));       fBig.loadFont(&wBig);
  wHuge.set(font_huge, sizeof(font_huge));    fHuge.loadFont(&wHuge);
}

static void text(LGFX_Sprite& s, const lgfx::IFont& f, const char* str, int x, int y,
                 uint32_t color, float size = 1.0f) {
  s.setFont(&f);
  s.setTextSize(size);
  s.setTextDatum(lgfx::middle_center);
  s.setTextColor(color);
  s.drawString(str, x, y);
  s.setTextSize(1.0f);
}

// Text zmenseny tak, aby se vesel do kruhu v dane vysce
static void textFit(LGFX_Sprite& s, const lgfx::IFont& f, const char* str, int x, int y, uint32_t color) {
  s.setFont(&f);
  s.setTextSize(1.0f);
  int w = s.textWidth(str);
  float dy = fabsf((float)(y - CY)) + 12.0f;
  float maxw = 2.0f * sqrtf(fmaxf(0.0f, 112.0f * 112.0f - dy * dy)) - 12.0f;
  float size = (w > maxw && w > 0) ? maxw / w : 1.0f;
  text(s, f, str, x, y, color, size);
}

// Oblouk: uhly ve stupnich od 12 hodin po smeru hodinovych rucicek.
static void arc(LGFX_Sprite& s, int r0, int r1, float a0, float a1, uint32_t c) {
  if (a1 <= a0) return;
  if (a1 - a0 >= 359.9f) {  // plny kruh
    s.fillArc(CX, CY, r0, r1, 0, 180, c);
    s.fillArc(CX, CY, r0, r1, 180, 360, c);
    return;
  }
  float s0 = a0 - 90.0f, s1 = a1 - 90.0f;  // LovyanGFX: 0 st. = 3 hodiny
  while (s0 < 0) { s0 += 360.0f; s1 += 360.0f; }
  while (s0 >= 360.0f) { s0 -= 360.0f; s1 -= 360.0f; }
  if (s1 <= 360.0f) {
    s.fillArc(CX, CY, r0, r1, s0, s1, c);
  } else {
    s.fillArc(CX, CY, r0, r1, s0, 360.0f, c);
    s.fillArc(CX, CY, r0, r1, 0, s1 - 360.0f, c);
  }
}

static void capDot(LGFX_Sprite& s, float angleDeg, uint32_t c) {
  float a = (angleDeg - 90.0f) * (float)M_PI / 180.0f;
  float r = (RING_IN + RING_OUT) / 2.0f;
  s.fillSmoothCircle(CX + (int)lroundf(cosf(a) * r), CY + (int)lroundf(sinf(a) * r),
                     (RING_OUT - RING_IN) / 2 + 1, c);
}

static void ticks(LGFX_Sprite& s) {
  for (int i = 0; i < 60; i++) {
    float a = i * 6.0f * (float)M_PI / 180.0f;
    int r0 = (i % 5 == 0) ? 96 : 99;
    float ca = cosf(a), sa = sinf(a);
    s.drawLine(CX + (int)(ca * r0), CY + (int)(sa * r0), CX + (int)(ca * 102), CY + (int)(sa * 102), TICK);
  }
}

// Prstenec prubehu (frac 0..1)
static void ring(LGFX_Sprite& s, float frac, uint32_t color, bool withTicks = true) {
  arc(s, RING_IN, RING_OUT, 0, 360, TRACK);
  if (withTicks) ticks(s);
  if (frac <= 0) return;
  if (frac > 1) frac = 1;
  float end = 360.0f * frac;
  arc(s, RING_IN, RING_OUT, 0, end, color);
  if (frac < 1) { capDot(s, 0, color); capDot(s, end, color); }
}

// --------------------------------------------------------------------
//  Testovaci obrazovka pri startu: L / P a sipka nahoru
// --------------------------------------------------------------------
void boot(LGFX_Sprite& s, int eye, int rotation, const char* version, const char* ip) {
  s.fillScreen(BG);
  ring(s, 1.0f, eye == 0 ? CYAN : AMBER, false);
  // sipka nahoru
  uint32_t c = WHITE;
  s.fillTriangle(CX, 30, CX - 18, 54, CX + 18, 54, c);
  s.fillRect(CX - 5, 52, 10, 16, c);
  text(s, fSmall, tr(S_UP), CX, 80, DIM);
  text(s, fMed, eye == 0 ? "L" : (i18n::lang() == LANG_CS ? "P" : "R"), CX, 128, eye == 0 ? CYAN : AMBER, 2.4f);
  char buf[32];
  snprintf(buf, sizeof buf, "%s", tr(eye == 0 ? S_LEFT_EYE : S_RIGHT_EYE));
  text(s, fSmall, buf, CX, 178, WHITE);
  if (eye == 1 && ip && *ip) snprintf(buf, sizeof buf, "%s", ip);
  else snprintf(buf, sizeof buf, "rot %d  v%s", rotation * 90, version);
  text(s, fSmall, buf, CX, 204, DIM);
}

// --------------------------------------------------------------------
//  Animovane oci
// --------------------------------------------------------------------
static void happyEye(LGFX_Sprite& s, uint32_t c) {
  // obloucek "^" - usmevave oko
  s.fillArc(CX, 150, 50, 68, 200, 340, c);
  for (float a : {200.0f, 340.0f}) {
    float r = a * (float)M_PI / 180.0f;
    s.fillSmoothCircle(CX + (int)lroundf(cosf(r) * 59), 150 + (int)lroundf(sinf(r) * 59), 9, c);
  }
}

void eyes(LGFX_Sprite& s, int eye, const EyeAnim& a, Mood mood) {
  (void)eye;
  s.fillScreen(BG);
  arc(s, 112, 119, 0, 360, 0x0B151Cu);
  if (mood == Mood::Happy) { happyEye(s, CYAN); return; }

  int cx = CX + (int)lroundf(a.gx), cy = CY + (int)lroundf(a.gy);
  s.fillSmoothCircle(cx, cy, 66, 0x08323Du);
  s.fillSmoothCircle(cx, cy, 60, 0x0D6676u);
  s.fillSmoothCircle(cx, cy, 52, 0x1AB8CCu);
  s.fillSmoothCircle(cx, cy, 44, 0x0E5C6Bu);
  // paprsky duhovky
  for (int i = 0; i < 24; i++) {
    float r = i * 15.0f * (float)M_PI / 180.0f;
    float ca = cosf(r), sa = sinf(r);
    s.drawLine(cx + (int)(ca * 30), cy + (int)(sa * 30), cx + (int)(ca * 43), cy + (int)(sa * 43), 0x158A9Bu);
  }
  s.fillSmoothCircle(cx, cy, 27, 0x000000u);
  s.fillSmoothCircle(cx - 19, cy - 21, 9, 0xFFFFFFu);
  s.fillSmoothCircle(cx + 14, cy + 15, 4, 0xC8F4F8u);

  float open = a.open;
  if (mood == Mood::Sleepy && open > 0.42f) open = 0.42f;
  int lid = (int)lroundf((1.0f - open) * 124.0f);
  if (lid > 0) {
    const int R = 260;
    s.fillCircle(CX, lid - R, R, BG);
    s.fillCircle(CX, 240 - lid + R, R, BG);
    s.fillArc(CX, lid - R, R - 4, R, 30, 150, LID_EDGE);
    s.fillArc(CX, 240 - lid + R, R - 4, R, 210, 330, LID_EDGE);
  }
}

// --------------------------------------------------------------------
//  Hodiny: leve oko hodiny, prave minuty, kolem vterinova rucicka
// --------------------------------------------------------------------

void clock(LGFX_Sprite& s, int eye, const ClockInfo& c) {
  s.fillScreen(BG);
  ring(s, c.valid ? c.second / 60.0f : 0, 0x0F8F8Au);
  char buf[24];
  if (!c.valid) { text(s, fMed, "--", CX, 120, DIM); return; }
  if (eye == 0) {
    snprintf(buf, sizeof buf, "%02d", c.hour);
    text(s, fHuge, buf, CX, 112, WHITE);
    text(s, fSmall, i18n::weekday(c.wday), CX, 182, DIM);
  } else {
    snprintf(buf, sizeof buf, "%02d", c.minute);
    text(s, fHuge, buf, CX, 112, CYAN);
    snprintf(buf, sizeof buf, "%d. %d.", c.day, c.month);
    text(s, fSmall, buf, CX, 182, DIM);
  }
}

// --------------------------------------------------------------------
//  Pocasi
// --------------------------------------------------------------------
enum WIcon { W_CLEAR, W_PARTLY, W_CLOUDY, W_FOG, W_DRIZZLE, W_RAIN, W_SHOWER, W_SNOW, W_STORM };

static WIcon iconFor(int code) {
  if (code == 0) return W_CLEAR;
  if (code <= 2) return W_PARTLY;
  if (code == 3) return W_CLOUDY;
  if (code == 45 || code == 48) return W_FOG;
  if (code >= 51 && code <= 57) return W_DRIZZLE;
  if (code >= 61 && code <= 67) return W_RAIN;
  if (code >= 71 && code <= 77) return W_SNOW;
  if (code >= 80 && code <= 82) return W_SHOWER;
  if (code == 85 || code == 86) return W_SNOW;
  if (code >= 95) return W_STORM;
  return W_CLOUDY;
}

static const char* describe(int code) {
  switch (iconFor(code)) {
    case W_CLEAR:   return tr(S_W_CLEAR);
    case W_PARTLY:  return tr(code == 1 ? S_W_MOSTLY_CLEAR : S_W_PARTLY);
    case W_CLOUDY:  return tr(S_W_CLOUDY);
    case W_FOG:     return tr(S_W_FOG);
    case W_DRIZZLE: return tr(S_W_DRIZZLE);
    case W_RAIN:    return tr(S_W_RAIN);
    case W_SHOWER:  return tr(S_W_SHOWER);
    case W_SNOW:    return tr(S_W_SNOW);
    case W_STORM:   return tr(S_W_STORM);
  }
  return "";
}

static void sun(LGFX_Sprite& s, int x, int y, int r) {
  for (int i = 0; i < 8; i++) {
    float a = i * 45.0f * (float)M_PI / 180.0f;
    s.drawWideLine(x + (int)(cosf(a) * (r + 8)), y + (int)(sinf(a) * (r + 8)),
                   x + (int)(cosf(a) * (r + 20)), y + (int)(sinf(a) * (r + 20)), 3.0f, 0xFFC83Du);
  }
  s.fillSmoothCircle(x, y, r, 0xFFC83Du);
}

static void moon(LGFX_Sprite& s, int x, int y, int r) {
  s.fillSmoothCircle(x, y, r, 0xE8E2C0u);
  s.fillSmoothCircle(x + r / 2, y - r / 3, r - 4, BG);
}

static void cloud(LGFX_Sprite& s, int x, int y, uint32_t c) {
  s.fillSmoothCircle(x - 30, y + 8, 22, c);
  s.fillSmoothCircle(x + 2, y - 8, 32, c);
  s.fillSmoothCircle(x + 32, y + 10, 20, c);
  s.fillSmoothRoundRect(x - 52, y + 6, 104, 26, 13, c);
}

static void drops(LGFX_Sprite& s, int x, int y, int n, uint32_t c, bool snow) {
  for (int i = 0; i < n; i++) {
    int dx = x - (n - 1) * 14 + i * 28;
    if (snow) {
      s.fillSmoothCircle(dx, y + 6, 5, c);
      s.fillSmoothCircle(dx + 10, y + 22, 5, c);
    } else {
      s.drawWideLine(dx + 6, y, dx - 2, y + 18, 3.0f, c);
    }
  }
}

static void weatherIcon(LGFX_Sprite& s, int x, int y, int code, bool day) {
  switch (iconFor(code)) {
    case W_CLEAR:  day ? sun(s, x, y, 30) : moon(s, x, y, 34); break;
    case W_PARTLY: day ? sun(s, x + 26, y - 20, 22) : moon(s, x + 26, y - 22, 22);
                   cloud(s, x - 6, y + 6, 0xDCE4EAu); break;
    case W_CLOUDY: cloud(s, x + 14, y - 14, 0x6F7F8Cu); cloud(s, x - 8, y + 8, 0xC9D3DBu); break;
    case W_FOG:    cloud(s, x, y - 14, 0xA9B5BFu);
                   for (int i = 0; i < 3; i++) s.fillSmoothRoundRect(x - 50 + i * 8, y + 28 + i * 12, 100 - i * 16, 6, 3, 0x8C99A4u);
                   break;
    case W_DRIZZLE:cloud(s, x, y - 18, 0xB8C3CCu); drops(s, x, y + 26, 3, 0x6FB6FFu, false); break;
    case W_RAIN:   cloud(s, x, y - 18, 0x8D9AA6u); drops(s, x, y + 24, 4, 0x4FA3FFu, false); break;
    case W_SHOWER: day ? sun(s, x + 28, y - 34, 18) : (void)0;
                   cloud(s, x - 4, y - 14, 0xB0BCC6u); drops(s, x - 4, y + 28, 3, 0x4FA3FFu, false); break;
    case W_SNOW:   cloud(s, x, y - 18, 0xC9D3DBu); drops(s, x, y + 22, 3, 0xFFFFFFu, true); break;
    case W_STORM:  cloud(s, x, y - 20, 0x6F7F8Cu);
                   s.fillTriangle(x + 4, y + 10, x - 14, y + 40, x + 2, y + 40, 0xFFD23Fu);
                   s.fillTriangle(x - 4, y + 36, x + 14, y + 36, x - 8, y + 66, 0xFFD23Fu);
                   break;
  }
}

void weather(LGFX_Sprite& s, int eye, const WeatherInfo& w) {
  s.fillScreen(BG);
  arc(s, 112, 119, 0, 360, 0x0B151Cu);
  if (!w.valid) {
    text(s, fMed, eye == 0 ? tr(S_WEATHER) : "--", CX, 120, DIM);
    if (eye == 1 && w.status[0]) textFit(s, fSmall, w.status, CX, 160, DIM);
    return;
  }
  char buf[32];
  if (eye == 0) {
    weatherIcon(s, CX, 100, w.code, w.isDay);
    text(s, fMed, describe(w.code), CX, 184, WHITE);
  } else {
    snprintf(buf, sizeof buf, "%d°", (int)lroundf(w.temp));
    text(s, fBig, buf, CX + 6, 98, WHITE);
    snprintf(buf, sizeof buf, "%d° / %d°", (int)lroundf(w.tmin), (int)lroundf(w.tmax));
    text(s, fMed, buf, CX, 152, DIM);
    if (w.rainProb >= 0) {
      snprintf(buf, sizeof buf, tr(S_RAIN_PCT), w.rainProb);
      text(s, fSmall, buf, CX, 186, 0x6FB6FFu);
    }
  }
}

// --------------------------------------------------------------------
//  Prubeh tisku
//   leve oko:  "KONEC ZA" + zbyvajici cas + cas dokonceni
//   prave oko: stav + procenta + vrstvy
// --------------------------------------------------------------------
void print(LGFX_Sprite& s, int eye, const PrintInfo& p, const ClockInfo& c, uint32_t ms) {
  s.fillScreen(BG);
  uint32_t col = CYAN;
  const char* label = tr(S_PRINTING);
  if (p.phase == Phase::Paused)    { col = AMBER; label = tr(S_PAUSED); }
  if (p.phase == Phase::Preparing) { label = tr(S_PREPARING); }

  float frac = p.percent >= 0 ? p.percent / 100.0f : 0.0f;
  if (p.phase == Phase::Preparing) {
    // tocici se segment - tiskarna se pripravuje
    arc(s, RING_IN, RING_OUT, 0, 360, TRACK);
    ticks(s);
    float a0 = fmodf(ms * 0.25f, 360.0f);
    arc(s, RING_IN, RING_OUT, a0, a0 + 70.0f, col);
  } else {
    ring(s, frac, col);
  }

  char buf[32];
  if (eye == 0) {
    text(s, fSmall, tr(S_ENDS_IN), CX, 66, DIM);
    if (p.remainingMin >= 0) snprintf(buf, sizeof buf, "%d:%02d", p.remainingMin / 60, p.remainingMin % 60);
    else                     snprintf(buf, sizeof buf, "-:--");
    text(s, fBig, buf, CX, 116, WHITE);
    if (c.valid && p.remainingMin >= 0) {
      int t = c.hour * 60 + c.minute + p.remainingMin;
      int days = t / (24 * 60);
      t %= 24 * 60;
      if (days == 0) snprintf(buf, sizeof buf, tr(S_AT_TIME), t / 60, t % 60);
      else           snprintf(buf, sizeof buf, tr(S_AT_TIME_DAYS), days, t / 60, t % 60);
      text(s, fMed, buf, CX, 168, col);
    }
  } else {
    text(s, fSmall, label, CX, 66, col);
    if (p.percent >= 0) snprintf(buf, sizeof buf, "%d%%", p.percent);
    else                snprintf(buf, sizeof buf, "--%%");
    text(s, fBig, buf, CX + 4, 116, WHITE);
    if (p.totalLayers > 0 && p.layer >= 0) {
      snprintf(buf, sizeof buf, tr(S_LAYER), p.layer, p.totalLayers);
      text(s, fSmall, buf, CX, 168, DIM);
    } else if (p.nozzle > 0) {
      snprintf(buf, sizeof buf, tr(S_NOZZLE_DEG), (int)lroundf(p.nozzle));
      text(s, fSmall, buf, CX, 168, DIM);
    }
  }
}

// --------------------------------------------------------------------
//  Dotisknuto / chyba
// --------------------------------------------------------------------
void finished(LGFX_Sprite& s, int eye, const char* atTime) {
  s.fillScreen(BG);
  ring(s, 1.0f, GREEN, false);
  s.fillArc(CX, 140, 46, 62, 200, 340, GREEN);
  for (float a : {200.0f, 340.0f}) {
    float r = a * (float)M_PI / 180.0f;
    s.fillSmoothCircle(CX + (int)lroundf(cosf(r) * 54), 140 + (int)lroundf(sinf(r) * 54), 8, GREEN);
  }
  if (eye == 0) text(s, fMed, tr(S_DONE), CX, 176, WHITE);
  else if (atTime && *atTime) text(s, fMed, atTime, CX, 176, DIM);
}

void failed(LGFX_Sprite& s, int eye, uint32_t error) {
  s.fillScreen(BG);
  ring(s, 1.0f, RED, false);
  s.drawWideLine(CX - 34, 72, CX + 34, 140, 8.0f, RED);
  s.drawWideLine(CX + 34, 72, CX - 34, 140, 8.0f, RED);
  char buf[24];
  if (eye == 0) {
    text(s, fMed, tr(S_ERROR), CX, 178, WHITE);
  } else if (error) {
    snprintf(buf, sizeof buf, "%04X-%04X", (unsigned)(error >> 16), (unsigned)(error & 0xFFFF));
    text(s, fSmall, buf, CX, 178, DIM);
  } else {
    text(s, fSmall, tr(S_CHECK_PRINTER), CX, 178, DIM);
  }
}

// --------------------------------------------------------------------
//  Hlaseni (pripojovani, chyba spojeni ...)
// --------------------------------------------------------------------
static void iconWifi(LGFX_Sprite& s, uint32_t c) {
  for (int i = 0; i < 3; i++) {
    int r = 22 + i * 20;
    s.fillArc(CX, 140, r, r + 9, 225, 315, c);
  }
  s.fillSmoothCircle(CX, 140, 8, c);
}

static void iconPrinter(LGFX_Sprite& s, uint32_t c) {
  s.drawRoundRect(CX - 46, 66, 92, 92, 8, c);
  s.drawRoundRect(CX - 45, 67, 90, 90, 7, c);
  s.fillRect(CX - 36, 80, 72, 5, c);
  s.fillRect(CX - 6, 85, 12, 16, c);
  s.fillTriangle(CX - 10, 101, CX + 10, 101, CX, 112, c);
  s.fillRect(CX - 36, 140, 72, 6, c);
}

static void iconWarn(LGFX_Sprite& s, uint32_t c) {
  s.fillTriangle(CX, 62, CX - 56, 158, CX + 56, 158, c);
  s.fillTriangle(CX, 80, CX - 42, 150, CX + 42, 150, BG);
  s.fillRoundRect(CX - 5, 96, 10, 34, 4, c);
  s.fillSmoothCircle(CX, 140, 6, c);
}

void message(LGFX_Sprite& s, int eye, Icon icon, const char* l1, const char* l2, const char* l3) {
  s.fillScreen(BG);
  arc(s, 112, 119, 0, 360, 0x0B151Cu);
  if (eye == 0) {
    uint32_t c = icon == ICON_WARN ? AMBER : DIM;
    if (icon == ICON_WIFI) iconWifi(s, c);
    else if (icon == ICON_PRINTER) iconPrinter(s, c);
    else iconWarn(s, c);
    return;
  }
  if (l1) textFit(s, fMed, l1, CX, 86, WHITE);
  if (l2) textFit(s, fSmall, l2, CX, 124, DIM);
  if (l3) textFit(s, fSmall, l3, CX, 150, DIM);
}

void statusDot(LGFX_Sprite& s, uint32_t rgb) {
  s.fillSmoothCircle(CX, 222, 5, rgb);
}

// --------------------------------------------------------------------
//  Samostatne obsahy pro jedno oko
// --------------------------------------------------------------------
void clockHM(LGFX_Sprite& s, const ClockInfo& c) {
  s.fillScreen(BG);
  ring(s, c.valid ? c.second / 60.0f : 0, 0x0F8F8Au);
  char buf[16];
  if (!c.valid) { text(s, fMed, "--:--", CX, 120, DIM); return; }
  snprintf(buf, sizeof buf, "%d:%02d", c.hour, c.minute);
  text(s, fBig, buf, CX, 114, WHITE);
  text(s, fSmall, i18n::weekday(c.wday), CX, 166, DIM);
}

void dateDay(LGFX_Sprite& s, const ClockInfo& c) {
  s.fillScreen(BG);
  arc(s, 112, 119, 0, 360, 0x0B151Cu);
  char buf[16];
  if (!c.valid) { text(s, fMed, "--", CX, 120, DIM); return; }
  text(s, fMed, i18n::weekday(c.wday), CX, 76, CYAN);
  snprintf(buf, sizeof buf, i18n::lang() == LANG_CS ? "%d." : "%d", c.day);
  text(s, fBig, buf, CX, 124, WHITE);
  text(s, fMed, i18n::month(c.month), CX, 172, DIM);
}

static uint32_t phaseColor(const PrintInfo& p) { return p.phase == Phase::Paused ? AMBER : CYAN; }

static void printRing(LGFX_Sprite& s, const PrintInfo& p, uint32_t ms) {
  uint32_t col = phaseColor(p);
  if (p.phase == Phase::Preparing) {
    arc(s, RING_IN, RING_OUT, 0, 360, TRACK);
    ticks(s);
    float a0 = fmodf(ms * 0.25f, 360.0f);
    arc(s, RING_IN, RING_OUT, a0, a0 + 70.0f, col);
  } else {
    ring(s, p.percent >= 0 ? p.percent / 100.0f : 0.0f, col);
  }
}

void printCombo(LGFX_Sprite& s, const PrintInfo& p, const ClockInfo& c, uint32_t ms) {
  s.fillScreen(BG);
  printRing(s, p, ms);
  char buf[32];
  if (p.percent >= 0) snprintf(buf, sizeof buf, "%d%%", p.percent);
  else snprintf(buf, sizeof buf, "--%%");
  text(s, fBig, buf, CX + 4, 100, WHITE);
  if (p.remainingMin >= 0) {
    snprintf(buf, sizeof buf, tr(S_LEFT_TIME), p.remainingMin / 60, p.remainingMin % 60);
    text(s, fMed, buf, CX, 150, phaseColor(p));
    if (c.valid) {
      int t = (c.hour * 60 + c.minute + p.remainingMin) % (24 * 60);
      snprintf(buf, sizeof buf, tr(S_DONE_AT), t / 60, t % 60);
      text(s, fSmall, buf, CX, 180, DIM);
    }
  }
}

void printTemps(LGFX_Sprite& s, const PrintInfo& p, uint32_t ms) {
  s.fillScreen(BG);
  printRing(s, p, ms);
  char buf[16];
  text(s, fSmall, tr(S_NOZZLE), CX, 62, DIM);
  if (p.nozzle > 0) snprintf(buf, sizeof buf, "%d°", (int)lroundf(p.nozzle)); else snprintf(buf, sizeof buf, "--");
  text(s, fBig, buf, CX + 6, 98, WHITE);
  text(s, fSmall, tr(S_BED), CX, 146, DIM);
  if (p.bed > 0) snprintf(buf, sizeof buf, "%d°", (int)lroundf(p.bed)); else snprintf(buf, sizeof buf, "--");
  text(s, fMed, buf, CX + 4, 178, WHITE);
}

void printerIdle(LGFX_Sprite& s) {
  s.fillScreen(BG);
  arc(s, 112, 119, 0, 360, 0x0B151Cu);
  iconPrinter(s, DIM);
  text(s, fSmall, tr(S_NOTHING_PRINTING), CX, 186, DIM);
}

void blank(LGFX_Sprite& s) { s.fillScreen(BG); }

// --------------------------------------------------------------------
//  Nahled modelu: prstenec prubehu, obrazek doplni volajici (cover::draw)
// --------------------------------------------------------------------
void modelBase(LGFX_Sprite& s, const PrintInfo& p, uint32_t ms, bool printing) {
  s.fillScreen(BG);
  if (printing) printRing(s, p, ms);
  else arc(s, 112, 119, 0, 360, 0x0B151Cu);
}

void modelOverlay(LGFX_Sprite& s, const PrintInfo& p, bool printing, bool hasImage) {
  if (!hasImage) {
    iconPrinter(s, DIM);
    text(s, fSmall, tr(S_NO_PREVIEW), CX, 186, DIM);
    return;
  }
  if (printing && p.percent >= 0) {
    char buf[16];
    snprintf(buf, sizeof buf, "%d%%", p.percent);
    s.fillSmoothRoundRect(CX - 34, 186, 68, 26, 13, 0x0B151Cu);
    text(s, fSmall, buf, CX, 199, WHITE);
  }
}

// --------------------------------------------------------------------
//  Prvni nastaveni: leve oko = co udelat, prave = adresa
// --------------------------------------------------------------------
void setup(LGFX_Sprite& s, int eye, bool portal, const char* apName, const char* ip, uint32_t ms) {
  s.fillScreen(BG);
  // pomalu se tocici oblouk - robot ceka
  arc(s, RING_IN, RING_OUT, 0, 360, TRACK);
  float a0 = fmodf(ms * 0.06f + eye * 180.0f, 360.0f);
  arc(s, RING_IN, RING_OUT, a0, a0 + 50.0f, AMBER);
  if (eye == 0) {
    text(s, fSmall, tr(S_SETUP), CX, 58, AMBER);
    iconWifi(s, WHITE);
    if (portal) {
      text(s, fSmall, tr(S_JOIN_WIFI), CX, 162, DIM);
      textFit(s, fMed, apName, CX, 190, WHITE);
    } else {
      text(s, fMed, "Wi-Fi OK", CX, 184, GREEN);
    }
  } else {
    text(s, fSmall, portal ? tr(S_THEN_OPEN) : tr(S_SET_ME_UP), CX, 76, DIM);
    textFit(s, fMed, ip && *ip ? ip : "...", CX, 116, WHITE);
    if (!portal) text(s, fSmall, "nw3d-robot.local", CX, 154, DIM);
  }
}

void updating(LGFX_Sprite& s, int pct) {
  s.fillScreen(BG);
  ring(s, pct / 100.0f, AMBER, false);
  char buf[16];
  snprintf(buf, sizeof buf, "%d%%", pct);
  text(s, fBig, buf, CX + 4, 112, WHITE);
  text(s, fSmall, tr(S_UPDATING), CX, 164, DIM);
}

// --------------------------------------------------------------------
//  Kurzy (akcie, komodity, meny)
// --------------------------------------------------------------------
// Cena s ceskou carkou a mezerou mezi tisici, pocet desetin podle velikosti
void formatPrice(char* buf, int n, float v) {
  float a = fabsf(v);
  int dec = a >= 10000 ? 0 : (a >= 100 ? 1 : (a >= 1 ? 2 : 4));
  if (a >= 100 && a < 1000) dec = 2;
  char raw[32];
  snprintf(raw, sizeof raw, "%.*f", dec, (double)a);
  char* dot = strchr(raw, '.');
  int intLen = dot ? (int)(dot - raw) : (int)strlen(raw);
  int j = 0;
  if (v < 0 && j < n - 1) buf[j++] = '-';
  for (int i = 0; i < intLen && j < n - 2; i++) {
    if (i > 0 && (intLen - i) % 3 == 0 && intLen > 4) buf[j++] = ' ';
    buf[j++] = raw[i];
  }
  if (dot) { if (j < n - 1) buf[j++] = ','; for (const char* p = dot + 1; *p && j < n - 1; p++) buf[j++] = *p; }
  buf[j] = 0;
}

static float changePct(const QuoteInfo& q) { return q.prev != 0 ? (q.price - q.prev) / q.prev * 100.0f : 0.0f; }
static uint32_t changeColor(float pct) { return pct > 0.005f ? GREEN : (pct < -0.005f ? RED : DIM); }

// "▲ +1,23 %" se sipkou nakreslenou jako trojuhelnik
static void changeLine(LGFX_Sprite& s, float pct, int y, const lgfx::IFont& f) {
  char buf[24];
  snprintf(buf, sizeof buf, "%+.2f %%", (double)pct);
  for (char* p = buf; *p; p++) if (*p == '.') *p = ',';
  uint32_t col = changeColor(pct);
  s.setFont(&f);
  s.setTextSize(1.0f);
  int w = s.textWidth(buf);
  int tri = 18, gap = 8;
  int x0 = CX - (w + tri + gap) / 2;
  int tx = x0 + tri / 2;
  if (pct > 0.005f)       s.fillTriangle(tx, y - 8, tx - 9, y + 7, tx + 9, y + 7, col);
  else if (pct < -0.005f) s.fillTriangle(tx, y + 8, tx - 9, y - 7, tx + 9, y - 7, col);
  else                    s.fillRect(tx - 8, y - 2, 16, 5, col);
  text(s, f, buf, x0 + tri + gap + w / 2, y, col);
}

void quotePrice(LGFX_Sprite& s, const QuoteInfo& q) {
  s.fillScreen(BG);
  float pct = changePct(q);
  arc(s, 112, 119, 0, 360, 0x0B151Cu);
  if (q.valid) arc(s, 112, 119, 300, 420, changeColor(pct));   // barevny oblouk nahore
  textFit(s, fMed, q.name[0] ? q.name : q.sym, CX, 62, CYAN);
  if (!q.valid) {
    text(s, fBig, "--", CX, 116, DIM);
    textFit(s, fSmall, q.status, CX, 166, DIM);
    return;
  }
  char buf[32];
  formatPrice(buf, sizeof buf, q.price);
  textFit(s, fBig, buf, CX, 114, WHITE);
  textFit(s, fSmall, q.unit, CX, 152, DIM);
  changeLine(s, pct, 186, fMed);
}

void quoteChart(LGFX_Sprite& s, const QuoteInfo& q) {
  s.fillScreen(BG);
  arc(s, 112, 119, 0, 360, 0x0B151Cu);
  textFit(s, fSmall, q.name[0] ? q.name : q.sym, CX, 50, DIM);
  if (!q.valid || q.nspark < 2) {
    text(s, fMed, q.valid ? tr(S_NO_CHART) : "--", CX, 120, DIM);
    return;
  }
  float pct = changePct(q);
  uint32_t col = changeColor(pct);
  float lo = q.prev, hi = q.prev;
  for (int i = 0; i < q.nspark; i++) { lo = fminf(lo, q.spark[i]); hi = fmaxf(hi, q.spark[i]); }
  if (hi - lo < 1e-6f) { hi += 1; lo -= 1; }
  const int X0 = 34, X1 = 206, Y0 = 76, Y1 = 150;
  auto px = [&](int i) { return X0 + (float)(X1 - X0) * i / (q.nspark - 1); };
  auto py = [&](float v) { return Y1 - (float)(Y1 - Y0) * (v - lo) / (hi - lo); };
  // vcerejsi zaviraci cena - prerusovana cara
  int yPrev = (int)lroundf(py(q.prev));
  for (int x = X0; x < X1; x += 8) s.drawFastHLine(x, yPrev, 4, 0x3A4B58u);
  // vypln pod krivkou (ztlumena barva)
  uint32_t fillc = col == GREEN ? 0x0F3A24u : (col == RED ? 0x3E1414u : 0x1A2630u);
  for (int i = 0; i < q.nspark - 1; i++) {
    float xa = px(i), xb = px(i + 1), ya = py(q.spark[i]), yb = py(q.spark[i + 1]);
    for (int x = (int)xa; x <= (int)xb; x++) {
      float t = xb > xa ? (x - xa) / (xb - xa) : 0;
      int y = (int)lroundf(ya + (yb - ya) * t);
      int yb2 = yPrev;
      if (y < yb2) s.drawFastVLine(x, y, yb2 - y, fillc);
      else if (y > yb2) s.drawFastVLine(x, yb2, y - yb2, fillc);
    }
  }
  for (int i = 0; i < q.nspark - 1; i++) {
    s.drawWideLine(px(i), py(q.spark[i]), px(i + 1), py(q.spark[i + 1]), 1.6f, col);
  }
  s.fillSmoothCircle((int)px(q.nspark - 1), (int)py(q.spark[q.nspark - 1]), 4, col);
  char buf[32];
  formatPrice(buf, sizeof buf, q.price);
  text(s, fMed, buf, CX, 178, WHITE);
  changeLine(s, pct, 206, fSmall);
}

}  // namespace ui

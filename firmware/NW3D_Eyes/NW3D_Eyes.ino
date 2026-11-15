// =====================================================================
//  NW3D-01 - robot s ocima ukazujici stav tiskarny Bambu Lab
//  Deska: Waveshare ESP32-S3-DualEye-LCD-1.28
//
//  Nastavuje se z webu robota: http://nw3d-robot.local (pri prvnim spusteni
//  vlastni Wi-Fi "NW3D-xxxx"). Navod: README.md v repozitari Frenkeeone/nw3d-01.
//  Vlakna: sit (Wi-Fi, MQTT, pocasi) bezi na jadru 0, vykreslovani v loop() na jadru 1.
// =====================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <LovyanGFX.hpp>

#include "config.h"
#include "cfg.h"
#include "i18n.h"
#include "wifimgr.h"
#include "cover.h"
#include "audio.h"
#include "ota.h"
#include "board.h"
#include "ui.h"
#include "printer.h"
#include "weather.h"
#include "quotes.h"
#include "settings.h"
#include "web.h"
#include "version.h"

enum class Scr { None, Eyes, Clock, Weather, Quote, Print, Finished, Failed, MsgWifi, MsgPrinter, MsgIdle, Test, Off, Setup, Update };

static const char* scrName(Scr s) {
  switch (s) {
    case Scr::None: return "";
    case Scr::Eyes: return tr(S_SCR_EYES);
    case Scr::Clock: return tr(S_SCR_CLOCK);
    case Scr::Weather: return tr(S_WEATHER);
    case Scr::Quote: return tr(S_SCR_QUOTES);
    case Scr::Print: return tr(S_SCR_PRINT);
    case Scr::Finished: return tr(S_SCR_FINISHED);
    case Scr::Failed: return tr(S_SCR_FAILED);
    case Scr::MsgWifi: return tr(S_WIFI);
    case Scr::MsgPrinter: return tr(S_SCR_PRINTER);
    case Scr::MsgIdle: return tr(S_NOTHING_PRINTING);
    case Scr::Test: return tr(S_SCR_TEST);
    case Scr::Off: return tr(S_SCR_OFF);
    case Scr::Setup: return tr(S_SCR_SETUP);
    case Scr::Update: return tr(S_UPDATING);
  }
  return "";
}

static LGFX_Sprite fb[2];
static EyeAnim anim;

// --------------------------------------------------------------------
//  Sit (jadro 0)
// --------------------------------------------------------------------
static void netTask(void*) {
  printer::begin();
  String tz;
  for (;;) {
    wifimgr::loop();
    if (wifimgr::connected()) {
      DevCfg c = cfg::get();
      if (c.tz != tz) { tz = c.tz; configTzTime(tz.c_str(), "pool.ntp.org", "time.google.com"); }
      printer::loop();
      weather::loop();
      quotes::loop();
      cover::loop();
      ota::loop();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// --------------------------------------------------------------------
//  Pomocne funkce
// --------------------------------------------------------------------
static ClockInfo readClock() {
  ClockInfo c;
  time_t now = time(nullptr);
  if (now < 1700000000) return c;  // cas jeste neni z NTP
  struct tm t;
  localtime_r(&now, &t);
  c.valid = true;
  c.hour = t.tm_hour; c.minute = t.tm_min; c.second = t.tm_sec;
  c.wday = t.tm_wday; c.day = t.tm_mday; c.month = t.tm_mon + 1;
  return c;
}

static bool isNight(const ClockInfo& c, const Settings& S) {
  if (!c.valid || S.nightFrom == S.nightTo) return false;
  if (S.nightFrom > S.nightTo) return c.hour >= S.nightFrom || c.hour < S.nightTo;
  return c.hour >= S.nightFrom && c.hour < S.nightTo;
}

static bool isActive(Phase p) { return p == Phase::Printing || p == Phase::Paused || p == Phase::Preparing; }

// Pohyb oci a mrkani
static void updateEyes(uint32_t ms) {
  static float tx = 0, ty = 0;
  static uint32_t nextLook = 0, nextBlink = 2000, blinkStart = 0;
  if (ms > nextLook) {
    if (random(4) == 0) { tx = 0; ty = 0; }
    else {
      float a = random(360) * (float)M_PI / 180.0f, r = random(10, 32);
      tx = cosf(a) * r; ty = sinf(a) * r * 0.7f;
    }
    nextLook = ms + random(1200, 4200);
  }
  anim.gx += (tx - anim.gx) * 0.25f;
  anim.gy += (ty - anim.gy) * 0.25f;
  if (ms > nextBlink) {
    blinkStart = ms;
    nextBlink = ms + random(2500, 6500);
    if (random(5) == 0) nextBlink = ms + 350;  // obcas dvojite mrknuti
  }
  float t = (ms - blinkStart) / 200.0f;
  anim.open = t < 1.0f ? fabsf(1.0f - 2.0f * t) : 1.0f;
}

// Rozdeli "radek1|radek2" na dve casti
static void splitStatus(const char* in, char* a, size_t na, char* b, size_t nb) {
  const char* bar = strchr(in, '|');
  size_t la = bar ? (size_t)(bar - in) : strlen(in);
  if (la >= na) la = na - 1;
  memcpy(a, in, la); a[la] = 0;
  strncpy(b, bar ? bar + 1 : "", nb - 1); b[nb - 1] = 0;
}

// --------------------------------------------------------------------
//  Setup
// --------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nNW3D-01 v" FW_VERSION);

  settings::begin();
  cfg::begin();
  Settings S = settings::get();
  i18n::setLang(S.lang == 0xFF ? LANG_EN : S.lang);
  if (!S.setupDone && cfg::get().printerReady()) { S.setupDone = true; settings::set(S); }
  if (!board::begin(S.rotation, S.swapEyes)) Serial.println("[displej] chyba inicializace");
  for (auto& s : fb) {
    s.setPsram(true);
    s.setColorDepth(16);
    if (!s.createSprite(board::W, board::H)) Serial.println("[displej] malo pameti (zapni PSRAM: OPI)");
  }
  ui::begin();

  for (int e = 0; e < 2; e++) { ui::boot(fb[e], e, S.rotation, FW_VERSION); board::pushFrame(e, fb[e].getBuffer()); }
  board::setBrightness(S.bright);
  randomSeed(esp_random());

  audio::begin();
  wifimgr::begin();
  xTaskCreatePinnedToCore(netTask, "net", 16384, nullptr, 1, nullptr, 0);
  web::setFrameSource(fb[0].getBuffer(), fb[1].getBuffer());
  web::begin();
  delay(3000);
}

// --------------------------------------------------------------------
//  Hlavni smycka - volba obrazovky a vykresleni
// --------------------------------------------------------------------
void loop() {
  static Phase lastPhase = Phase::Unknown;
  static uint32_t finishedAt = 0, failedAt = 0, testUntil = 0;
  static char finishedTime[16] = "";
  static int rotIdx = 0;
  static uint32_t rotSince = 0;
  static uint32_t btnDown = 0;
  static uint8_t lastBright = 255;
  static uint32_t lastSetVer = 0;
  static char ipStr[20] = "";

  uint32_t ms = millis();
  Settings S = settings::get();
  ClockInfo clk = readClock();
  PrintInfo p = printer::snapshot();
  WeatherInfo w = weather::snapshot();
  bool wifiUp = wifimgr::connected();
  snprintf(ipStr, sizeof ipStr, "%s", wifimgr::ip().c_str());
  bool portal = wifimgr::state() == wifimgr::State::Portal;
  i18n::setLang(S.lang == 0xFF ? LANG_EN : S.lang);

  // zmena nastaveni z webu
  if (settings::version() != lastSetVer) {
    if (lastSetVer) board::setOrientation(S.rotation, S.swapEyes);
    lastSetVer = settings::version();
    rotSince = ms;
  }
  if (web::takeTestRequest()) testUntil = ms + 6000;
  if (web::takeSoundRequest()) audio::play(audio::TUNE_TEST, S.volume ? S.volume : 50);

  // prechody stavu tiskarny
  if (p.hasData && p.phase != lastPhase) {
    if (p.phase == Phase::Finished && isActive(lastPhase)) {
      finishedAt = ms;
      if (clk.valid) snprintf(finishedTime, sizeof finishedTime, tr(S_AT_TIME), clk.hour, clk.minute);
      if (S.soundFinish && !(isNight(clk, S) && S.nightBright == 0)) audio::play(audio::TUNE_FINISH, S.volume);
    }
    if (p.phase == Phase::Failed && isActive(lastPhase)) {
      failedAt = ms;
      if (S.soundError) audio::play(audio::TUNE_ERROR, S.volume);
    }
    lastPhase = p.phase;
  }

  // tlacitko BOOT (jen pred montazi): kratce = dalsi obrazovka, podrzet 3 s = test displeju
  bool manualNext = false;
  if (board::bootPressed()) {
    if (!btnDown) btnDown = ms;
    if (ms - btnDown > 3000) testUntil = ms + 6000;
  } else if (btnDown) {
    if (ms - btnDown < 1500) manualNext = true;
    btnDown = 0;
  }

  bool night = isNight(clk, S);
  bool printerProblem = p.configured && (!p.online || !p.hasData) && p.status[0];
  bool printing = p.hasData && isActive(p.phase) && (p.online || ms - p.lastMsgMs < 120000UL);
  bool showFinished = p.phase == Phase::Finished && finishedAt && ms - finishedAt < S.finishedMin * 60000UL;
  bool showFailed = p.phase == Phase::Failed && failedAt && ms - failedAt < 30 * 60000UL;

  // 1) obrazovky pres obe oci (maji prednost)
  Scr full = Scr::None;
  bool needSetup = portal || (wifiUp && !S.setupDone && !p.configured);
  if (ota::progress() >= 0) full = Scr::Update;
  else if (testUntil && ms < testUntil) full = Scr::Test;
  else if (needSetup && !printing) full = Scr::Setup;
  else if (showFinished && !printing) full = Scr::Finished;
  else if (showFailed && !printing) full = Scr::Failed;
  else if (night && S.nightBright == 0 && !printing) full = Scr::Off;

  // 2) stridani pro oci nastavene na "stridani"
  // kurzy: platne polozky
  int qValid[quotes::MAX], qn = 0;
  for (int i = 0; i < quotes::count(); i++) if (quotes::get(i).valid) qValid[qn++] = i;
  int autoQuote = 0;

  Scr autoScr = Scr::Eyes;
  {
    Scr list[12];
    int qOf[12] = {0};
    int n = 0;
    if (printing) {
      list[n++] = Scr::Print;
    } else if (night) {
      list[n++] = clk.valid ? Scr::Clock : Scr::Eyes;
    } else {
      if (S.showEyes) list[n++] = Scr::Eyes;
      if (S.showClock && clk.valid) list[n++] = Scr::Clock;
      if (S.showWeather && w.valid) list[n++] = Scr::Weather;
      if (S.showQuotes) for (int k = 0; k < qn; k++) { qOf[n] = qValid[k]; list[n++] = Scr::Quote; }
      if (!wifiUp) list[n++] = Scr::MsgWifi;
      else if (printerProblem) list[n++] = Scr::MsgPrinter;
      if (n == 0) list[n++] = Scr::Eyes;
    }
    if (manualNext || ms - rotSince > S.screenSec * 1000UL) { rotIdx++; rotSince = ms; }
    autoScr = list[rotIdx % n];
    autoQuote = qOf[rotIdx % n];
  }

  // 3) co ukazuje ktere oko
  uint8_t wid[2];
  for (int e = 0; e < 2; e++) wid[e] = printing ? S.printW[e] : S.idleW[e];

  // kurz pro oci s pevne zvolenym kurzem: pri vice polozkach se stridaji
  QuoteInfo q;
  if (full == Scr::None) {
    bool fixed = false;
    for (int e = 0; e < 2; e++) fixed |= (wid[e] == W_QUOTE_PRICE || wid[e] == W_QUOTE_CHART);
    if (fixed) {
      if (qn > 0) q = quotes::get(qValid[(ms / (S.screenSec * 1000UL)) % qn]);
      else if (quotes::count() > 0) q = quotes::get(0);
      else snprintf(q.status, sizeof q.status, "%s", tr(S_NO_QUOTE));
    }
  }
  QuoteInfo qAuto;
  if (full == Scr::None && autoScr == Scr::Quote) qAuto = quotes::get(autoQuote);

  // nazev pro web
  {
    char nm[96];
    if (full != Scr::None) snprintf(nm, sizeof nm, "%s", scrName(full));
    else {
      char a[40], b[40];
      snprintf(a, sizeof a, "%s", wid[0] == W_AUTO ? scrName(autoScr) : i18n::widgetName(wid[0]));
      snprintf(b, sizeof b, "%s", wid[1] == W_AUTO ? scrName(autoScr) : i18n::widgetName(wid[1]));
      snprintf(nm, sizeof nm, "L: %s · P: %s", a, b);
    }
    web::setScreenName(nm);
  }

  // jas
  uint8_t bright = night ? S.nightBright : S.bright;
  if (full == Scr::Off) bright = 0;
  if (night && bright < 8 && full != Scr::Off) bright = 8;
  if ((full == Scr::Test || full == Scr::Setup || full == Scr::Update) && bright < 30) bright = 30;
  if (bright != lastBright) { board::setBrightness(bright); lastBright = bright; }
  if (full == Scr::Off) { delay(500); return; }

  bool anyEye = (full == Scr::None) &&
                (wid[0] == W_EYE || wid[1] == W_EYE ||
                 ((wid[0] == W_AUTO || wid[1] == W_AUTO) && autoScr == Scr::Eyes));
  if (anyEye) updateEyes(ms);
  Mood mood = night ? Mood::Sleepy : Mood::Normal;

  char l2[48], l3[48];
  for (int e = 0; e < 2; e++) {
    LGFX_Sprite& s = fb[e];
    Scr sc = full;
    if (sc == Scr::None && wid[e] == W_AUTO) sc = autoScr;
    if (sc != Scr::None) {
      // cela obrazovka (polovina pro toto oko)
      switch (sc) {
        case Scr::Eyes:     ui::eyes(s, e, anim, mood); break;
        case Scr::Clock:    ui::clock(s, e, clk); break;
        case Scr::Weather:  ui::weather(s, e, w); break;
        case Scr::Quote:    e == 0 ? ui::quotePrice(s, qAuto) : ui::quoteChart(s, qAuto); break;
        case Scr::Print:    ui::print(s, e, p, clk, ms); break;
        case Scr::Finished: ui::finished(s, e, finishedTime); break;
        case Scr::Failed:   ui::failed(s, e, p.error); break;
        case Scr::Test:     ui::boot(s, e, S.rotation, FW_VERSION, ipStr); break;
        case Scr::MsgWifi:  ui::message(s, e, ui::ICON_WIFI, tr(S_WIFI), tr(S_CONNECTING), cfg::get().ssid.c_str()); break;
        case Scr::Setup:    ui::setup(s, e, portal, wifimgr::apName().c_str(), ipStr, ms); break;
        case Scr::Update:   ui::updating(s, ota::progress()); break;
        case Scr::MsgPrinter:
          splitStatus(p.status, l2, sizeof l2, l3, sizeof l3);
          ui::message(s, e, ui::ICON_PRINTER, tr(S_PRINTER), l2, l3);
          break;
        default: ui::blank(s); break;
      }
    } else {
      // samostatne zvoleny obsah oka
      switch (wid[e]) {
        case W_EYE:           ui::eyes(s, e, anim, mood); break;
        case W_CLOCK:         ui::clockHM(s, clk); break;
        case W_HOURS:         ui::clock(s, 0, clk); break;
        case W_MINUTES:       ui::clock(s, 1, clk); break;
        case W_DATE:          ui::dateDay(s, clk); break;
        case W_WEATHER_ICON:  ui::weather(s, 0, w); break;
        case W_WEATHER_TEMP:  ui::weather(s, 1, w); break;
        case W_PRINT_TIME:    printing ? ui::print(s, 0, p, clk, ms) : ui::printerIdle(s); break;
        case W_PRINT_PERCENT: printing ? ui::print(s, 1, p, clk, ms) : ui::printerIdle(s); break;
        case W_PRINT_COMBO:   printing ? ui::printCombo(s, p, clk, ms) : ui::printerIdle(s); break;
        case W_PRINT_TEMPS:   printing ? ui::printTemps(s, p, ms) : ui::printerIdle(s); break;
        case W_QUOTE_PRICE:   ui::quotePrice(s, q); break;
        case W_QUOTE_CHART:   ui::quoteChart(s, q); break;
        case W_MODEL: {
          ui::modelBase(s, p, ms, printing);
          bool img = printing && cover::draw(s, 120 - cover::SIZE / 2, 120 - cover::SIZE / 2 - 4);
          ui::modelOverlay(s, p, printing, img);
          break;
        }
        default:              ui::blank(s); break;
      }
    }
    // tecka stavu tiskarny (prave oko, mimo tisk)
    if (e == 1 && p.configured && full == Scr::None && !printing && wid[1] != W_OFF) {
      ui::statusDot(s, (p.online && p.hasData) ? 0x2E7D4Fu : 0xC07A10u);
    }
    board::pushFrame(e, s.getBuffer());
  }

  // tempo: oci a priprava plynule, ostatni staci obcas
  bool smooth = anyEye || (printing && p.phase == Phase::Preparing) || full == Scr::Setup;
  uint32_t frame = smooth ? 33 : 200;
  uint32_t spent = millis() - ms;
  if (spent < frame) delay(frame - spent);
}

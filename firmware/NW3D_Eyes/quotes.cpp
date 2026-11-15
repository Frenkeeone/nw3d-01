#include "quotes.h"
#include <ArduinoJson.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "i18n.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "settings.h"
#endif

namespace quotes {

// Zname symboly: cesky nazev a jednotka (ostatni berou nazev z Yahoo)
struct Preset { const char* sym; const char* name; const char* unit; const char* nameEn; const char* unitEn; };
static const Preset PRESETS[] = {
  {"BZ=F", "Ropa Brent", "USD/barel", "Brent crude", "USD/bbl"},
  {"CL=F", "Ropa WTI", "USD/barel", "WTI crude", "USD/bbl"},
  {"GC=F", "Zlato", "USD/unce", "Gold", "USD/oz"},
  {"SI=F", "Stříbro", "USD/unce", "Silver", "USD/oz"},
  {"NG=F", "Zemní plyn", "USD/MMBtu", "Natural gas", "USD/MMBtu"},
  {"HG=F", "Měď", "USD/libra", "Copper", "USD/lb"},
  {"BTC-USD", "Bitcoin", "USD", "Bitcoin", "USD"},
  {"ETH-USD", "Ethereum", "USD", "Ethereum", "USD"},
  {"EURCZK=X", "EUR/CZK", "Kč za 1 €", "EUR/CZK", "CZK per EUR"},
  {"CZK=X", "USD/CZK", "Kč za 1 $", "USD/CZK", "CZK per USD"},
  {"EURUSD=X", "EUR/USD", "$ za 1 €", "EUR/USD", "USD per EUR"},
  {"^GSPC", "S&P 500", "body", "S&P 500", "points"},
  {"^IXIC", "Nasdaq", "body", "Nasdaq", "points"},
  {"^GDAXI", "DAX", "body", "DAX", "points"},
  {"^STOXX50E", "Euro Stoxx 50", "body", "Euro Stoxx 50", "points"},
  {"CEZ.PR", "ČEZ", "Kč", "CEZ", "CZK"},
  {"KOMB.PR", "Komerční banka", "Kč", "Komercni banka", "CZK"},
  {"AAPL", "Apple", "USD", "Apple", "USD"},
  {"MSFT", "Microsoft", "USD", "Microsoft", "USD"},
  {"NVDA", "Nvidia", "USD", "Nvidia", "USD"},
  {"TSLA", "Tesla", "USD", "Tesla", "USD"},
  {"AMZN", "Amazon", "USD", "Amazon", "USD"},
  {"GOOGL", "Alphabet", "USD", "Alphabet", "USD"},
};

static const Preset* findPreset(const char* sym) {
  for (const auto& p : PRESETS)
    if (strcasecmp(p.sym, sym) == 0) return &p;
  return nullptr;
}
const char* presetName(const char* sym) {
  const Preset* p = findPreset(sym);
  return p ? (i18n::lang() == LANG_CS ? p->name : p->nameEn) : nullptr;
}
const char* presetUnit(const char* sym) {
  const Preset* p = findPreset(sym);
  return p ? (i18n::lang() == LANG_CS ? p->unit : p->unitEn) : nullptr;
}

// Jen znaky, ktere umi pisma na displeji
static void asciiCopy(char* dst, size_t n, const char* src) {
  size_t j = 0;
  for (size_t i = 0; src[i] && j + 1 < n; i++) {
    unsigned char ch = (unsigned char)src[i];
    if (ch >= 0x20 && ch < 0x7F) dst[j++] = (char)ch;
  }
  while (j > 0 && dst[j - 1] == ' ') j--;
  dst[j] = 0;
}

bool parse(const char* json, size_t len, QuoteInfo& q) {
  JsonDocument filter;
  JsonObject fr = filter["chart"]["result"][0].to<JsonObject>();
  JsonObject fm = fr["meta"].to<JsonObject>();
  for (const char* k : {"currency", "symbol", "shortName", "regularMarketPrice", "previousClose", "chartPreviousClose"})
    fm[k] = true;
  fr["indicators"]["quote"][0]["close"] = true;
  filter["chart"]["error"] = true;

  JsonDocument d;
  if (deserializeJson(d, json, len, DeserializationOption::Filter(filter))) {
    snprintf(q.status, sizeof q.status, "chybná data");
    return false;
  }
  JsonObject r = d["chart"]["result"][0];
  if (r.isNull()) {
    const char* e = d["chart"]["error"]["description"] | "neznámý symbol";
    char tmp[40];
    asciiCopy(tmp, sizeof tmp, e);
    snprintf(q.status, sizeof q.status, "%s", tmp);
    return false;
  }
  JsonObject m = r["meta"];
  float price = m["regularMarketPrice"] | NAN;
  if (isnan(price)) { snprintf(q.status, sizeof q.status, "bez ceny"); return false; }
  float prev = m["previousClose"] | NAN;
  if (isnan(prev)) prev = m["chartPreviousClose"] | NAN;

  q.price = price;
  q.prev = isnan(prev) ? price : prev;

  const char* nm = presetName(q.sym);
  if (nm) snprintf(q.name, sizeof q.name, "%s", nm);
  else asciiCopy(q.name, sizeof q.name, m["shortName"] | q.sym);
  if (!q.name[0]) snprintf(q.name, sizeof q.name, "%s", q.sym);

  const char* un = presetUnit(q.sym);
  if (un) snprintf(q.unit, sizeof q.unit, "%s", un);
  else { const char* cur = m["currency"] | ""; snprintf(q.unit, sizeof q.unit, "%s", strcmp(cur, "CZK") == 0 ? "Kč" : cur); }

  // prubeh dne - vynechat prazdne hodnoty, zmensit na max. 48 bodu
  JsonArray cl = r["indicators"]["quote"][0]["close"];
  float tmp[160];
  int n = 0;
  for (JsonVariant v : cl) {
    if (v.isNull()) continue;
    if (n < 160) tmp[n++] = v.as<float>();
  }
  int cap = (int)(sizeof(q.spark) / sizeof(q.spark[0]));
  if (n <= cap) { for (int i = 0; i < n; i++) q.spark[i] = tmp[i]; q.nspark = n; }
  else { for (int i = 0; i < cap; i++) q.spark[i] = tmp[(long)i * (n - 1) / (cap - 1)]; q.nspark = cap; }

  q.valid = true;
  q.status[0] = 0;
  return true;
}

#ifdef ARDUINO
static QuoteInfo s_q[MAX];
static uint32_t s_next[MAX];
static int s_n = 0;
static char s_list[48] = "\x01";     // posledni zpracovany seznam (na zacatku neplatny)
static SemaphoreHandle_t s_lock;

int count() {
  if (!s_lock) return 0;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  int n = s_n;
  xSemaphoreGive(s_lock);
  return n;
}

QuoteInfo get(int i) {
  QuoteInfo out;
  if (!s_lock || i < 0 || i >= MAX) return out;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  out = s_q[i];
  xSemaphoreGive(s_lock);
  // nazev a jednotka znamych symbolu v aktualnim jazyce
  if (const char* n = presetName(out.sym)) snprintf(out.name, sizeof out.name, "%s", n);
  if (const char* u = presetUnit(out.sym)) snprintf(out.unit, sizeof out.unit, "%s", u);
  return out;
}

// Prevzit novy seznam symbolu z nastaveni
static void syncList() {
  Settings S = settings::get();
  if (strcmp(S.tickers, s_list) == 0) return;
  QuoteInfo fresh[MAX];
  int n = 0;
  char buf[48];
  snprintf(buf, sizeof buf, "%s", S.tickers);
  for (char* tok = strtok(buf, ","); tok && n < MAX; tok = strtok(nullptr, ",")) {
    while (*tok == ' ') tok++;
    if (!*tok) continue;
    // stejny symbol uz stazeny? ponechat data
    bool kept = false;
    for (int i = 0; i < s_n; i++)
      if (strcasecmp(s_q[i].sym, tok) == 0) { fresh[n] = s_q[i]; kept = true; break; }
    if (!kept) {
      snprintf(fresh[n].sym, sizeof fresh[n].sym, "%s", tok);
      const char* nm = presetName(tok);
      snprintf(fresh[n].name, sizeof fresh[n].name, "%s", nm ? nm : tok);
    }
    n++;
  }
  xSemaphoreTake(s_lock, portMAX_DELAY);
  for (int i = 0; i < MAX; i++) { s_q[i] = fresh[i]; s_next[i] = (i < n && fresh[i].valid) ? millis() + 600000UL : 0; }
  s_n = n;
  xSemaphoreGive(s_lock);
  snprintf(s_list, sizeof s_list, "%s", S.tickers);
}

static void urlEncode(const char* in, char* out, size_t n) {
  size_t j = 0;
  for (; *in && j + 4 < n; in++) {
    char c = *in;
    if (isalnum((unsigned char)c) || c == '.' || c == '-' || c == '_') out[j++] = c;
    else j += snprintf(out + j, n - j, "%%%02X", (unsigned char)c);
  }
  out[j] = 0;
}

static int fetch(const char* host, const char* sym, String& body) {
  char enc[48], url[160];
  urlEncode(sym, enc, sizeof enc);
  snprintf(url, sizeof url, "https://%s/v8/finance/chart/%s?range=1d&interval=30m", host, enc);
  WiFiClientSecure c;
  c.setInsecure();
  HTTPClient http;
  http.setTimeout(10000);
  http.setReuse(false);
  http.setUserAgent("Mozilla/5.0 (NW3D-robot)");
  if (!http.begin(c, url)) return -100;
  int code = http.GET();
  if (code == 200 || code == 404) body = http.getString();   // 404 = neznamy symbol (s popisem v JSON)
  http.end();
  return code;
}

void loop() {
  if (!s_lock) s_lock = xSemaphoreCreateMutex();
  syncList();
  // jeden symbol za pruchod, aby sit (tiskarna) nestala dlouho
  int i = -1;
  uint32_t now = millis();
  for (int k = 0; k < s_n; k++)
    if (s_next[k] == 0 || (int32_t)(now - s_next[k]) >= 0) { i = k; break; }
  if (i < 0) return;

  QuoteInfo q = get(i);
  String body;
  int code = fetch("query1.finance.yahoo.com", q.sym, body);
  if (code != 200 && code != 404) code = fetch("query2.finance.yahoo.com", q.sym, body);
  bool ok = false;
  if (code == 200 || code == 404) {
    ok = parse(body.c_str(), body.length(), q);
  } else {
    snprintf(q.status, sizeof q.status, "%s (%d), RAM %u kB",
             code == -100 ? "nelze spojit" : HTTPClient::errorToString(code).c_str(), code,
             (unsigned)(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL) / 1024));
  }
  if (ok) Serial.printf("[kurzy] %s %.3f (%.3f)\n", q.sym, q.price, q.prev);
  else    Serial.printf("[kurzy] %s: %s\n", q.sym, q.status);

  xSemaphoreTake(s_lock, portMAX_DELAY);
  if (i < s_n && strcmp(s_q[i].sym, q.sym) == 0) {   // seznam se mezitim nezmenil
    if (ok) s_q[i] = q;
    else snprintf(s_q[i].status, sizeof s_q[i].status, "%s", q.status);   // stara data zustanou
    s_next[i] = millis() + (ok ? 600000UL : (code == 404 ? 3600000UL : 120000UL));
  }
  xSemaphoreGive(s_lock);
}
#endif

}  // namespace quotes

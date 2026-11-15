#include "i18n.h"

namespace i18n {

static volatile uint8_t s_lang = LANG_CS;

void setLang(uint8_t l) { s_lang = l < LANG_COUNT ? l : LANG_EN; }
uint8_t lang() { return s_lang; }

struct Pair { const char* cs; const char* en; };

static const Pair T[S_COUNT] = {
  /* S_UP */              {"NAHORU", "UP"},
  /* S_LEFT_EYE */        {"levé oko", "left eye"},
  /* S_RIGHT_EYE */       {"pravé oko", "right eye"},
  /* S_WEATHER */         {"Počasí", "Weather"},
  /* S_RAIN_PCT */        {"srážky %d %%", "rain %d %%"},
  /* S_SET_CITY */        {"nastav město", "set your city"},
  /* S_W_CLEAR */         {"Jasno", "Clear"},
  /* S_W_MOSTLY_CLEAR */  {"Skoro jasno", "Mostly clear"},
  /* S_W_PARTLY */        {"Polojasno", "Partly cloudy"},
  /* S_W_CLOUDY */        {"Zataženo", "Overcast"},
  /* S_W_FOG */           {"Mlha", "Fog"},
  /* S_W_DRIZZLE */       {"Mrholení", "Drizzle"},
  /* S_W_RAIN */          {"Déšť", "Rain"},
  /* S_W_SHOWER */        {"Přeháňky", "Showers"},
  /* S_W_SNOW */          {"Sníh", "Snow"},
  /* S_W_STORM */         {"Bouřka", "Storm"},
  /* S_ENDS_IN */         {"KONEC ZA", "ENDS IN"},
  /* S_PRINTING */        {"TISK", "PRINTING"},
  /* S_PAUSED */          {"PAUZA", "PAUSED"},
  /* S_PREPARING */       {"PŘÍPRAVA", "PREPARING"},
  /* S_AT_TIME */         {"v %d:%02d", "at %d:%02d"},
  /* S_AT_TIME_DAYS */    {"+%dd v %d:%02d", "+%dd at %d:%02d"},
  /* S_LAYER */           {"vrstva %d/%d", "layer %d/%d"},
  /* S_NOZZLE_DEG */      {"tryska %d°", "nozzle %d°"},
  /* S_DONE */            {"HOTOVO", "DONE"},
  /* S_ERROR */           {"CHYBA", "ERROR"},
  /* S_CHECK_PRINTER */   {"zkontroluj tiskárnu", "check the printer"},
  /* S_LEFT_TIME */       {"zbývá %d:%02d", "left %d:%02d"},
  /* S_DONE_AT */         {"hotovo v %d:%02d", "done at %d:%02d"},
  /* S_NOZZLE */          {"TRYSKA", "NOZZLE"},
  /* S_BED */             {"PODLOŽKA", "BED"},
  /* S_NOTHING_PRINTING */{"nic se netiskne", "not printing"},
  /* S_MODEL */           {"MODEL", "MODEL"},
  /* S_NO_PREVIEW */      {"bez náhledu", "no preview"},
  /* S_NO_CHART */        {"graf zatím není", "no chart yet"},
  /* S_NO_QUOTE */        {"není zvolen žádný kurz", "no ticker selected"},
  /* S_WIFI */            {"Wi-Fi", "Wi-Fi"},
  /* S_CONNECTING */      {"připojuji k síti", "connecting"},
  /* S_SETUP */           {"NASTAVENÍ", "SETUP"},
  /* S_JOIN_WIFI */       {"připoj se k Wi-Fi", "join Wi-Fi"},
  /* S_THEN_OPEN */       {"a otevři", "then open"},
  /* S_SET_ME_UP */       {"nastav mě na", "set me up at"},
  /* S_PRINTER */         {"Tiskárna", "Printer"},
  /* S_UPDATING */        {"aktualizuji", "updating"},
  /* S_P_NOT_SET */       {"Není nastavena|otevři web robota", "Not set up|open the robot's web"},
  /* S_P_CONNECTING */    {"Připojuji...|%s", "Connecting...|%s"},
  /* S_P_CLOUD_DOWN */    {"Cloud nedostupný|zkontroluj internet", "Cloud unreachable|check internet"},
  /* S_P_TOKEN_REJECTED */{"Přihlášení vypršelo|přihlas se znovu (%d)", "Login expired|sign in again (%d)"},
  /* S_P_NO_UID */        {"Cloud: chybí uid|přihlas se znovu", "Cloud: no user id|sign in again"},
  /* S_P_DENIED_TOKEN */  {"Přístup odmítnut|přihlas se znovu", "Access denied|sign in again"},
  /* S_P_DENIED_CODE */   {"Přístup odmítnut|zkontroluj access code", "Access denied|check access code"},
  /* S_P_UNREACH_CLOUD */ {"Nedostupná (%d)|zkontroluj tiskárnu", "Unreachable (%d)|check the printer"},
  /* S_P_UNREACH_LAN */   {"Nedostupná (%d)|zkontroluj IP a LAN režim", "Unreachable (%d)|check IP and LAN mode"},
  /* S_P_NO_DATA_LAN */   {"Žádná data|zapni Developer mode", "No data|enable Developer mode"},
  /* S_P_NO_DATA_CLOUD */ {"Žádná data|zkontroluj tiskárnu", "No data|check the printer"},
  /* S_SCR_EYES */        {"Oči", "Eyes"},
  /* S_SCR_CLOCK */       {"Hodiny", "Clock"},
  /* S_SCR_QUOTES */      {"Kurzy", "Tickers"},
  /* S_SCR_PRINT */       {"Tisk", "Print"},
  /* S_SCR_FINISHED */    {"Hotovo", "Finished"},
  /* S_SCR_FAILED */      {"Chyba tisku", "Print failed"},
  /* S_SCR_PRINTER */     {"Stav tiskárny", "Printer status"},
  /* S_SCR_TEST */        {"Test displejů", "Display test"},
  /* S_SCR_OFF */         {"Vypnuto (noc)", "Off (night)"},
  /* S_SCR_SETUP */       {"Nastavení", "Setup"},
};

const char* tr(Str s) {
  if (s >= S_COUNT) return "";
  return s_lang == LANG_CS ? T[s].cs : T[s].en;
}

const char* weekday(int w) {
  static const char* const CS[] = {"neděle", "pondělí", "úterý", "středa", "čtvrtek", "pátek", "sobota"};
  static const char* const EN[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  w = ((w % 7) + 7) % 7;
  return s_lang == LANG_CS ? CS[w] : EN[w];
}

const char* month(int m) {
  static const char* const CS[] = {"ledna", "února", "března", "dubna", "května", "června",
                                   "července", "srpna", "září", "října", "listopadu", "prosince"};
  static const char* const EN[] = {"January", "February", "March", "April", "May", "June",
                                   "July", "August", "September", "October", "November", "December"};
  m = ((m - 1) % 12 + 12) % 12;
  return s_lang == LANG_CS ? CS[m] : EN[m];
}

const char* widgetName(uint8_t w) {
  static const char* const CS[] = {"střídání", "oko", "hodiny", "hodiny (h)", "minuty", "datum",
                                   "počasí – obrázek", "počasí – teplota", "zbývající čas", "procenta",
                                   "průběh vše v jednom", "teploty tisku", "vypnuto", "kurz – cena",
                                   "kurz – graf", "náhled modelu"};
  static const char* const EN[] = {"rotation", "eye", "clock", "clock (h)", "minutes", "date",
                                   "weather – icon", "weather – temperature", "time left", "percent",
                                   "progress all-in-one", "print temperatures", "off", "ticker – price",
                                   "ticker – chart", "model preview"};
  const int n = sizeof(CS) / sizeof(CS[0]);
  if (w >= n) return "?";
  return s_lang == LANG_CS ? CS[w] : EN[w];
}

}  // namespace i18n

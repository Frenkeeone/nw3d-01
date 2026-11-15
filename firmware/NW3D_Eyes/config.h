// =====================================================================
//  NW3D-01 - VYCHOZI NASTAVENI (NEPOVINNE)
//  Od verze 2.0 se vse nastavuje na webu robota (pruvodce po prvnim spusteni).
//  Tento soubor vyplnuj jen kdyz firmware prekladas sam v Arduino IDE a chces,
//  aby robot znal Wi-Fi a tiskarnu hned po nahrani. Pouzije se jen pri uplne
//  prvnim spusteni (dokud je pamet desky prazdna).
// =====================================================================
#pragma once

// ---------------------------------------------------------------------
//  Wi-Fi (jen 2,4 GHz sit, ESP32 neumi 5 GHz)
// ---------------------------------------------------------------------
#define WIFI_SSID      "nazev_wifi"
#define WIFI_PASSWORD  "heslo_wifi"

// ---------------------------------------------------------------------
//  Tiskarna Bambu Lab
// ---------------------------------------------------------------------
// Seriove cislo tiskarny (Bambu Studio > Zarizeni, nebo na displeji tiskarny
// Nastaveni > Zarizeni). Velkymi pismeny, bez mezer.
#define PRINTER_SERIAL   "0XXXXXXXXXXXXXX"

// Zpusob pripojeni:
//   1 = CLOUD - pres Bambu cloud. Tiskarna zustava v normalnim rezimu
//               (funguje Bambu Handy i tisk z Bambu Studia pres cloud).
//               Potrebuje token z prihlaseni na bambulab.com (viz NAVOD).
//   0 = LAN   - primo na tiskarnu v mistni siti. X2D (rada H2) k tomu
//               potrebuje zapnout "LAN only" + "Developer mode", ktery
//               vypne cloud a Bambu Handy.
#define PRINTER_MODE     1

// --- jen pro LAN (PRINTER_MODE 0) ---
#define PRINTER_IP           "192.168.1.50"   // IP tiskarny (Nastaveni > Sit)
#define PRINTER_ACCESS_CODE  "12345678"       // LAN access code (8 znaku)

// --- jen pro CLOUD (PRINTER_MODE 1) ---
#define BAMBU_TOKEN   "sem_vloz_token"        // cookie "token" z bambulab.com
#define BAMBU_UID     ""                      // nech prazdne, zjisti se samo
#define BAMBU_CHINA   0                       // 0 = Evropa/svet, 1 = Cina

// ---------------------------------------------------------------------
//  Pocasi (Open-Meteo, zdarma, bez registrace)
// ---------------------------------------------------------------------
#define WEATHER_ENABLED  1
#define WEATHER_LAT      0       // zemepisna sirka (0 = nastavit na webu)
#define WEATHER_LON      0       // zemepisna delka

// ---------------------------------------------------------------------
//  Kurzy (Yahoo Finance) - vychozi seznam, jinak se nastavuje na webu.
//  Nepovinne: kdyz radek chybi, pouzije se ropa Brent.
// ---------------------------------------------------------------------
// #define QUOTE_SYMBOLS  "BZ=F,GC=F"

// ---------------------------------------------------------------------
//  Cas (stredoevropsky cas vcetne letniho)
// ---------------------------------------------------------------------
#define TIMEZONE  "CET-1CEST,M3.5.0,M10.5.0/3"

// ---------------------------------------------------------------------
//  Displeje
// ---------------------------------------------------------------------
// Pri spusteni se na 4 s ukaze testovaci obrazovka: levé oko "L", prave "P"
// a sipka nahoru. Kdyz neni nahoru, zmen EYE_ROTATION (0-3, po 90 st.).
// Kdyz jsou oci prohozene, nastav SWAP_EYES na 1.
#define EYE_ROTATION   0
#define SWAP_EYES      0

#define BRIGHTNESS        80   // jas pres den, 0-100 %
#define NIGHT_BRIGHTNESS  10   // jas v noci (0 = displeje zhasnout, pokud se netiskne)
#define NIGHT_FROM        22   // od kolika hodin je noc
#define NIGHT_TO           7   // do kolika hodin je noc

// ---------------------------------------------------------------------
//  Obrazovky, kdyz se netiskne (stridaji se)
// ---------------------------------------------------------------------
#define SHOW_EYES       1   // animovane oci
#define SHOW_CLOCK      1   // hodiny
#define SHOW_WEATHER    1   // pocasi
#define SCREEN_SECONDS  15  // jak dlouho je kazda obrazovka videt

#define FINISHED_MINUTES  30   // jak dlouho po dotisku ukazovat "HOTOVO"

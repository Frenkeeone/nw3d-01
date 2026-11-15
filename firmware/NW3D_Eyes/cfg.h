// Nastaveni pripojeni (Wi-Fi, tiskarna, ucet, poloha). Uklada se do pameti desky (NVS).
// Zadava se pri instalaci a na webu robota; hodnoty z config.h se pouziji jen jednou
// jako vychozi (pro ty, kdo firmware prekladaji sami v Arduino IDE).
#pragma once
#include <Arduino.h>

enum PrinterMode : uint8_t { PM_NONE = 0, PM_CLOUD = 1, PM_LAN = 2 };

struct DevCfg {
  String  ssid, pass;
  uint8_t mode = PM_NONE;
  String  sn, pname, pmodel;   // seriove cislo, nazev a model tiskarny
  String  ip, acode;           // LAN: IP a access code
  String  token, user;         // cloud: token a MQTT uzivatel "u_<uid>"
  String  email;               // jen pro zobrazeni na webu (ne heslo!)
  uint8_t region = 0;          // 0 = svet, 1 = Cina
  float   lat = NAN, lon = NAN;
  String  city;
  String  tz = "CET-1CEST,M3.5.0,M10.5.0/3";
  String  tzName = "Europe/Prague";

  bool printerReady() const {
    if (sn.length() < 6) return false;
    if (mode == PM_CLOUD) return token.length() > 20;
    if (mode == PM_LAN) return ip.length() > 6 && acode.length() >= 6;
    return false;
  }
  bool hasLocation() const { return !isnan(lat) && !isnan(lon); }
};

namespace cfg {
void     begin();
DevCfg   get();
void     set(const DevCfg& c);      // ulozi vse
uint32_t version();                 // meni se pri kazde zmene
void     factoryReset();            // smaze vse vcetne Wi-Fi
}

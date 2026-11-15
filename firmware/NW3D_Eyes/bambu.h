// Bambu Lab cloud: prihlaseni k uctu, seznam tiskaren, posledni uloha (nahled modelu).
// Heslo se nikam neuklada - pouzije se jen jednou pro ziskani tokenu.
#pragma once
#include <Arduino.h>

namespace bambu {

struct Login {
  enum Kind : uint8_t { Ok, NeedCode, NeedTfa, Error } kind = Error;
  String token;     // pri Ok
  String tfaKey;    // pri NeedTfa
  String error;     // pri Error
};

Login login(const String& email, const String& password, uint8_t region);
bool  sendCode(const String& email, uint8_t region);
Login loginCode(const String& email, const String& code, uint8_t region);
Login loginTfa(const String& tfaKey, const String& code, uint8_t region);

// MQTT uzivatel "u_<uid>" - z tokenu (JWT) nebo dotazem na server. Prazdne = nepovedlo se.
String userFor(const String& token, uint8_t region, int* httpCode = nullptr);

struct Device { String sn, name, model, acode; bool online; };
// Tiskarny na uctu. Vraci pocet (nebo -HTTP kod pri chybe).
int devices(const String& token, uint8_t region, Device* out, int max);

// Posledni uloha tiskarny: nazev a URL obrazku. false = nepovedlo se.
bool lastTask(const String& token, uint8_t region, const String& sn, String& title, String& coverUrl);

}  // namespace bambu

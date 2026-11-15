// Aktualizace firmwaru: kontrola nove verze na GitHubu a instalace pres Wi-Fi.
#pragma once
#include <Arduino.h>

namespace ota {
void   loop();                 // v sitove uloze: obcas zkontroluje novou verzi, provede pozadovanou instalaci
void   requestCheck();         // zkontrolovat hned
void   requestInstall();       // nainstalovat nalezenou verzi (probehne v sitove uloze)
String latest();               // nejnovejsi verze na GitHubu ("" = nezjisteno)
bool   updateAvailable();
int    progress();             // -1 = nic neprobiha, 0-100 = instalace
String status();
bool   newer(const char* a, const char* b);   // je verze a novejsi nez b?
}

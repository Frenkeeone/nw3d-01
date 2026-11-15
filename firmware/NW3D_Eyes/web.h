// Webove rozhrani: http://nw3d-robot.local (nebo IP adresa desky).
#pragma once

namespace web {
void begin();                          // spusti vlastni ulohu se serverem
void setScreenName(const char* name);  // jakou obrazovku robot prave ukazuje
void setFrameSource(const void* left, const void* right);  // obraz oci pro nahled na webu
bool takeTestRequest();                // z webu prislo "test displeju"
bool takeSoundRequest();               // z webu prislo "zkusit zvuk"
}

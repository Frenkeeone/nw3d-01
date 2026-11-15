// Stav tiskarny Bambu Lab pres MQTT (LAN nebo Bambu cloud).
#pragma once
#include <stddef.h>
#include "model.h"

namespace printer {

void begin();               // jednou po startu
void loop();                // volat porad dokola (v sitove uloze)
PrintInfo snapshot();       // kopie aktualniho stavu (vlaknove bezpecne)

// Zpracovani jedne zpravy z tiskarny (JSON z tematu device/<SN>/report).
// Tiskarny posilaji casto jen zmenene polozky - prepisuje se jen to, co ve zprave je.
bool parseReport(const char* json, size_t len, PrintInfo& st);
Phase phaseFromState(const char* gcodeState);

}  // namespace printer

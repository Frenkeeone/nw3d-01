// Rozbor zprav z tiskarny - bez zavislosti na Wi-Fi, da se otestovat i na PC.
#include "printer.h"
#include <ArduinoJson.h>
#include <string.h>

namespace printer {

Phase phaseFromState(const char* s) {
  if (!s) return Phase::Unknown;
  if (!strcmp(s, "RUNNING")) return Phase::Printing;
  if (!strcmp(s, "PAUSE"))   return Phase::Paused;
  if (!strcmp(s, "PREPARE") || !strcmp(s, "SLICING")) return Phase::Preparing;
  if (!strcmp(s, "FINISH"))  return Phase::Finished;
  if (!strcmp(s, "FAILED"))  return Phase::Failed;
  if (!strcmp(s, "IDLE") || !strcmp(s, "INIT") || !strcmp(s, "OFFLINE")) return Phase::Idle;
  return Phase::Unknown;
}

static JsonDocument& filter() {
  static JsonDocument f;
  static bool init = false;
  if (!init) {
    JsonObject p = f["print"].to<JsonObject>();
    for (const char* k : {"gcode_state", "mc_percent", "mc_remaining_time", "layer_num",
                          "total_layer_num", "nozzle_temper", "bed_temper", "subtask_name",
                          "print_error"}) {
      p[k] = true;
    }
    init = true;
  }
  return f;
}

bool parseReport(const char* json, size_t len, PrintInfo& st) {
  JsonDocument doc;
  if (deserializeJson(doc, json, len, DeserializationOption::Filter(filter()))) return false;
  JsonObject p = doc["print"];
  if (p.isNull()) return false;

  if (p["gcode_state"].is<const char*>()) st.phase = phaseFromState(p["gcode_state"]);
  if (p["mc_percent"].is<int>())          st.percent = p["mc_percent"];
  if (p["mc_remaining_time"].is<int>())   st.remainingMin = p["mc_remaining_time"];
  if (p["layer_num"].is<int>())           st.layer = p["layer_num"];
  if (p["total_layer_num"].is<int>())     st.totalLayers = p["total_layer_num"];
  if (p["nozzle_temper"].is<float>())     st.nozzle = p["nozzle_temper"];
  if (p["bed_temper"].is<float>())        st.bed = p["bed_temper"];
  if (p["print_error"].is<uint32_t>())    st.error = p["print_error"];
  if (p["subtask_name"].is<const char*>()) {
    strncpy(st.job, p["subtask_name"] | "", sizeof(st.job) - 1);
    st.job[sizeof(st.job) - 1] = 0;
  }
  st.hasData = true;
  return true;
}

}  // namespace printer

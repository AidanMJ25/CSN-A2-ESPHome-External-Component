#include "csn_a2.h"

#include "esphome/core/log.h"

namespace esphome::csn_a2 {

static const char *const TAG = "csn_a2";

void CSNA2::dump_config() { ESP_LOGCONFIG(TAG, "CSN-A2 thermal receipt printer"); }

}  // namespace esphome::csn_a2

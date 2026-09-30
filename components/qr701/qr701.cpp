#include "qr701.h"

#include "esphome/core/log.h"

namespace esphome::qr701 {

static const char *const TAG = "qr701";

void QR701::dump_config() { ESP_LOGCONFIG(TAG, "QR701 thermal receipt printer"); }

}  // namespace esphome::qr701

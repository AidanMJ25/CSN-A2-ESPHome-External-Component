#pragma once

#include <string>

#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome::qr701 {

class QR701 : public Component, public uart::UARTDevice {
 public:
  void print(const std::string &text) {
    // ESC/POS-compatible printers accept printable bytes directly. Use the
    // string length rather than write_str() so embedded newlines are retained.
    this->write_array(reinterpret_cast<const uint8_t *>(text.data()), text.size());

    // Terminate the final text line and advance the receipt far enough to tear.
    static constexpr uint8_t FEED[] = {'\n', '\n', '\n'};
    this->write_array(FEED, sizeof(FEED));
  }

  void dump_config() override;
};

template<typename... Ts> class QR701PrintAction : public Action<Ts...> {
 public:
  explicit QR701PrintAction(QR701 *parent) : parent_(parent) {}

  TEMPLATABLE_VALUE(std::string, text)

  void play(const Ts &...x) override { this->parent_->print(this->text_.value(x...)); }

 protected:
  QR701 *parent_;
};

}  // namespace esphome::qr701

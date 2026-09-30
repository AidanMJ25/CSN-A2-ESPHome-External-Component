#pragma once

#include <string>

#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome::qr701 {

class QR701 : public PollingComponent, public uart::UARTDevice {
 public:
  void set_status_text_sensor(text_sensor::TextSensor *status) { this->status_ = status; }

  void print(const std::string &text) {
    // ESC/POS requires a status reply to be read before additional data is
    // sent. A poll takes at most 400 ms, so queue one action arriving in that
    // narrow window rather than corrupting either transaction.
    if (this->awaiting_status_) {
      this->queued_text_ = text;
      this->print_queued_ = true;
      this->publish_status_("printing");
      return;
    }
    this->start_print_(text);
  }

  void update() override;
  void loop() override;
  void dump_config() override;

 protected:
  void start_print_(const std::string &text) {
    // ESC/POS-compatible printers accept printable bytes directly. Use the
    // string length rather than write_str() so embedded newlines are retained.
    this->write_array(reinterpret_cast<const uint8_t *>(text.data()), text.size());

    // Terminate the final text line and advance the receipt far enough to tear.
    static constexpr uint8_t FEED[] = {'\n', '\n', '\n'};
    this->write_array(FEED, sizeof(FEED));

    // ESC/POS status has no portable "mechanism currently printing" bit.
    // Keep a local state briefly, then verify the printer's fault state.
    uint32_t line_count = 1;
    for (const char character : text) {
      if (character == '\n')
        line_count++;
    }
    this->printing_until_ = millis() + 250 + line_count * 100;
    this->printing_ = true;
    this->publish_status_("printing");
  }

  void request_status_(uint8_t query);
  void process_status_(uint8_t status);
  void publish_printer_status_();
  void publish_status_(const char *status);

  text_sensor::TextSensor *status_{nullptr};
  uint8_t query_{0};
  uint32_t query_started_at_{0};
  uint32_t printing_until_{0};
  std::string queued_text_;
  bool awaiting_status_{false};
  bool printing_{false};
  bool print_queued_{false};
  bool offline_{false};
  bool cover_open_{false};
  bool paper_out_{false};
  bool error_{false};
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

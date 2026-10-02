#include "qr701.h"

#include "esphome/core/log.h"

namespace esphome::qr701 {

static const char *const TAG = "qr701";

void QR701::print_text_field() {
  if (this->print_text_ == nullptr) {
    ESP_LOGW(TAG, "Print button pressed without a text entity");
    return;
  }
  this->print(this->print_text_->state);
}

void QR701::update() {
  if (!this->status_enabled_() || this->awaiting_status_)
    return;
  this->refresh_status();
}

void QR701::refresh_status() {
  if (this->awaiting_status_)
    return;
  this->offline_ = false;
  this->cover_open_ = false;
  this->paper_out_ = false;
  this->error_ = false;
  this->request_status_(1);
}

void QR701::loop() {
  if (!this->awaiting_status_)
    return;

  uint8_t response;
  if (this->read_byte(&response)) {
    this->awaiting_status_ = false;
    this->process_status_(response);
    if (this->query_ < 4) {
      this->request_status_(this->query_ + 1);
    } else {
      this->publish_printer_status_();
      if (this->feed_queued_) {
        this->feed_queued_ = false;
        this->start_feed_(this->queued_feed_lines_);
      }
      if (this->print_queued_) {
        this->print_queued_ = false;
        this->start_print_(this->queued_text_);
        this->queued_text_.clear();
      }
    }
    return;
  }

  if (millis() - this->query_started_at_ > 100) {
    this->awaiting_status_ = false;
    this->publish_status_("unavailable");
    if (this->feed_queued_) {
      this->feed_queued_ = false;
      this->start_feed_(this->queued_feed_lines_);
    }
    if (this->print_queued_) {
      this->print_queued_ = false;
      this->start_print_(this->queued_text_);
      this->queued_text_.clear();
    }
  }
}

void QR701::request_status_(uint8_t query) {
  this->query_ = query;
  this->write_byte(0x10);  // DLE
  this->write_byte(0x04);  // EOT
  this->write_byte(query);
  this->query_started_at_ = millis();
  this->awaiting_status_ = true;
}

void QR701::process_status_(uint8_t status) {
  // ESC/POS DLE EOT n real-time responses, n = 1..4.
  switch (this->query_) {
    case 1:
      this->offline_ = (status & 0x08) != 0;
      break;
    case 2:
      this->cover_open_ = (status & 0x04) != 0;
      this->paper_out_ = (status & 0x20) != 0;
      this->error_ = (status & 0x40) != 0;
      break;
    case 3:
      this->error_ = this->error_ || (status & 0x68) != 0;
      break;
    case 4:
      this->paper_out_ = this->paper_out_ || (status & 0x60) != 0;
      break;
  }
}

void QR701::publish_printer_status_() {
  this->publish_binary_status_();
  if (this->error_) {
    this->publish_status_("error");
  } else if (this->paper_out_) {
    this->publish_status_("paper_out");
  } else if (this->cover_open_) {
    this->publish_status_("cover_open");
  } else if (this->offline_) {
    this->publish_status_("offline");
  } else if (this->printing_ && millis() < this->printing_until_) {
    this->publish_status_("printing");
  } else {
    this->printing_ = false;
    this->publish_status_("idle");
  }
}

void QR701::publish_binary_status_() {
  if (this->paper_out_sensor_ != nullptr)
    this->paper_out_sensor_->publish_state(this->paper_out_);
  if (this->cover_open_sensor_ != nullptr)
    this->cover_open_sensor_->publish_state(this->cover_open_);
  if (this->error_sensor_ != nullptr)
    this->error_sensor_->publish_state(this->error_);
}

void QR701::publish_status_(const char *status) {
  ESP_LOGD(TAG, "Printer status: %s", status);
  if (this->status_ != nullptr)
    this->status_->publish_state(status);
}

bool QR701::status_enabled_() const {
  return this->status_ != nullptr || this->paper_out_sensor_ != nullptr || this->cover_open_sensor_ != nullptr ||
         this->error_sensor_ != nullptr;
}

void QR701::dump_config() {
  ESP_LOGCONFIG(TAG, "QR701 thermal receipt printer");
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace esphome::qr701

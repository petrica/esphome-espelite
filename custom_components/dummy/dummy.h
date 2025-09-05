#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/components/dht/dht.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace dummy {

class DummyComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  float get_setup_priority() const override;
  void dump_config() override;
  
  void set_pin(GPIOPin *pin) { this->pin_ = pin; }
  void set_temperature(float temperature);
  void set_humidity(float humidity);
  void set_dht_component(dht::DHT *dht_component) { this->dht_component_ = dht_component; }
  void set_dht_update_interval(uint32_t interval_ms) { this->dht_update_interval_ms_ = interval_ms; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { this->humidity_sensor_ = humidity_sensor; }
  void set_temp_age_threshold(uint32_t threshold_ms) { this->temp_age_threshold_ms_ = threshold_ms; }

 protected:
  void send_si7021_response_();
  bool poll_for_request_();
  
  GPIOPin *pin_{nullptr};
  dht::DHT *dht_component_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};
  bool responding_{false};
  uint32_t request_start_time_{0};
  bool last_pin_state_{true};
  
  // Timing control for 1-second intervals
  uint32_t last_loop_time_{0};
  uint32_t last_dht_update_time_{0};
  uint32_t dht_update_interval_ms_{2000};  // Default 2 seconds
  uint32_t temp_age_threshold_ms_{25000};  // Default 25 seconds
  
  // SI7021 dummy data (25.0°C, 50.0% humidity)
  static const uint8_t DUMMY_DATA[5];
  
  // Current temperature and humidity values
  float current_temperature_{NAN};
  float current_humidity_{NAN};
  
  // Timestamps for when values were last updated
  uint32_t last_temperature_update_{0};
  uint32_t last_humidity_update_{0};
};

}  // namespace dummy
}  // namespace esphome

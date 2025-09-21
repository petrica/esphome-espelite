#include "dummy.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <cmath>

static const char *const TAG = "dummy";

namespace esphome {
namespace dummy {

// Static instance for interrupt callback
static DummyComponent *instance_ = nullptr;

// Define static const array
// SI7021 format: [humidity_low, humidity_high, temp_low, temp_high, checksum]
// First 2 bytes: 0b0000000111100000 = 0x01E0 (480)
// Second 2 bytes: 0b0000000100000000 = 0x0100 (256)
// Checksum: 0b11100010 = 0xE2 (226)
const uint8_t DummyComponent::DUMMY_DATA[5] = {0x01, 0xE0, 0x01, 0x00, 0xE2};

void DummyComponent::setup() {
  if (this->pin_ != nullptr) {
    // Configure pin as input with pull-up to detect host requests
    this->pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    this->last_pin_state_ = this->pin_->digital_read();
    
    ESP_LOGD(TAG, "Dummy SI7021 component setup complete");
  }
}

void DummyComponent::loop() {
  uint32_t current_time = millis();
  uint32_t diff_time = current_time - this->last_loop_time_;

  // Check if it's time for DHT update
  if (this->dht_component_ != nullptr && 
      diff_time > 150 &&
      diff_time < 350 &&
      (current_time - this->last_dht_update_time_) >= this->dht_update_interval_ms_) {

    this->dht_component_->update();
    this->last_dht_update_time_ = current_time;
    ESP_LOGD(TAG, "DHT component updated");
  }
  
  if (this->pin_ != nullptr && 
      diff_time >= 500) {

    // Poll for request and handle if detected
    this->poll_for_request_();
    this->last_loop_time_ = current_time;
    ESP_LOGD(TAG, "Pool for ELITE Request");
  }
}

bool DummyComponent::poll_for_request_() {
  // Blocking poll for up to 50ms to detect the pattern
  uint32_t poll_start = millis();
  bool high_detected = false;
  bool low_detected = false;
  
  while ((millis() - poll_start) < 50) {
    bool current_state = this->pin_->digital_read();
    
    // Detect HIGH signal first
    if (!high_detected && current_state) {
      high_detected = true;
    }
    
    // Detect LOW transition after HIGH
    if (high_detected && !low_detected && !current_state) {
      low_detected = true;
      this->request_start_time_ = micros();
      
      // Wait for pin to go HIGH again
      while (!this->pin_->digital_read()) {
        // Block until pin goes HIGH
      }
      
      uint32_t pulse_duration = micros() - this->request_start_time_;
      
      // Check if the LOW pulse is within acceptable range (400μs to 600μs)
      if (pulse_duration >= 400 && pulse_duration <= 600) {
        ESP_LOGD(TAG, "Valid pulse duration confirmed (%u μs), sending response", pulse_duration);
        delayMicroseconds(38);  // 40μs delay before sending response
        this->send_si7021_response_();
        return true;  // Request handled successfully
      } else {
        ESP_LOGD(TAG, "Invalid pulse duration: %u μs (expected 400-600μs), continuing poll", pulse_duration);
        // Reset for next detection attempt
        high_detected = false;
        low_detected = false;
      }
    }
    
    // Small delay to prevent tight loop
    // delayMicroseconds(10);
  }
  
  return false;  // No valid request detected
}

void DummyComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Dummy SI7021 Component:");
  if (this->pin_ != nullptr) {
    LOG_PIN("  Pin: ", this->pin_);
  }
  ESP_LOGCONFIG(TAG, "  Simulating: SI7021 Temperature/Humidity Sensor");
  ESP_LOGCONFIG(TAG, "  DHT Update Interval: %u ms", this->dht_update_interval_ms_);
  ESP_LOGCONFIG(TAG, "  Temperature Age Threshold: %u ms", this->temp_age_threshold_ms_);
  
  if (this->temperature_sensor_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Temperature Sensor: Linked");
  }
  if (this->humidity_sensor_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Humidity Sensor: Linked");
  }
}

void DummyComponent::set_temperature(float temperature, bool isFallback) {
  uint32_t current_time = millis();
  uint32_t temperature_age = current_time - this->last_temperature_update_;
  
  // For fallback temperature, only update if:
  // 1. Current temperature is NaN, OR
  // 2. Current temperature is older than the threshold
  if (isFallback) {
    if (!std::isnan(this->current_temperature_) && 
        temperature_age <= this->temp_age_threshold_ms_) {
      ESP_LOGD(TAG, "Fallback temperature %.1f°C rejected - current temperature %.1f°C is still fresh (age: %u ms)", 
               temperature, this->current_temperature_, temperature_age);
      return;
    }
    // Do NOT update last_temperature_update_ for fallback temperature
    ESP_LOGI(TAG, "Fallback temperature updated: %.1f°C (age: %u ms)", temperature, temperature_age);
  } else {
    // Normal temperature update - update timestamp
    this->last_temperature_update_ = current_time;
    ESP_LOGI(TAG, "Temperature updated: %.1f°C", temperature);
  }
  
  // Always update the temperature value
  this->current_temperature_ = temperature;
}

void DummyComponent::set_humidity(float humidity, bool isFallback) {
  uint32_t current_time = millis();
  uint32_t humidity_age = current_time - this->last_humidity_update_;
  
  // For fallback humidity, only update if:
  // 1. Current humidity is NaN, OR
  // 2. Current humidity is older than the threshold
  if (isFallback) {
    if (!std::isnan(this->current_humidity_) && 
        humidity_age <= this->temp_age_threshold_ms_) {
      ESP_LOGD(TAG, "Fallback humidity %.1f%% rejected - current humidity %.1f%% is still fresh (age: %u ms)", 
               humidity, this->current_humidity_, humidity_age);
      return;
    }
    // Do NOT update last_humidity_update_ for fallback humidity
    ESP_LOGI(TAG, "Fallback humidity updated: %.1f%% (age: %u ms)", humidity, humidity_age);
  } else {
    // Normal humidity update - update timestamp
    this->last_humidity_update_ = current_time;
    ESP_LOGI(TAG, "Humidity updated: %.1f%%", humidity);
  }
  
  // Always update the humidity value
  this->current_humidity_ = humidity;
}

float DummyComponent::get_setup_priority() const {
  return esphome::setup_priority::DATA;
}

void DummyComponent::send_si7021_response_() {

  InterruptLock lock;

  // Always use current temperature and humidity values
  float temp_value = this->current_temperature_;
  float humidity_value = this->current_humidity_;
  
  // Check if we have valid data
  if (std::isnan(temp_value) || 
      std::isnan(humidity_value) || 
      (temp_value == 0.0f && humidity_value == 0.0f)) {
    ESP_LOGE(TAG, "No valid temperature or humidity data available, exiting response");
    return;
  }

  // Convert to raw values for SI7021 protocol
  uint16_t temp_raw = (uint16_t)(temp_value * 10.0f);
  uint16_t humidity_raw = (uint16_t)(humidity_value * 10.0f);
  
  // Switch pin to output mode
  this->pin_->pin_mode(gpio::FLAG_OUTPUT);
  
  // SI7021 response: pull pin LOW for 80μs, then HIGH for 80μs
  this->pin_->digital_write(false);
  delayMicroseconds(40);
  this->pin_->digital_write(true);
  delayMicroseconds(76);
  
  // Create response data: [humidity_high, humidity_low, temp_high, temp_low, checksum]
  uint8_t response_data[5];
  response_data[0] = (humidity_raw >> 8) & 0xFF; // Humidity high byte
  response_data[1] = humidity_raw & 0xFF;        // Humidity low byte
  response_data[2] = (temp_raw >> 8) & 0xFF;     // Temperature high byte
  response_data[3] = temp_raw & 0xFF;            // Temperature low byte
  
  // Calculate checksum (sum of all bytes)
  response_data[4] = response_data[0] + response_data[1] + response_data[2] + response_data[3];
  
  // Send SI7021 response data (40 bits = 5 bytes)
  for (int byte = 0; byte < 5; byte++) {
    uint8_t data_byte = response_data[byte];
    
    for (int bit = 7; bit >= 0; bit--) {
      // Start each bit with 50μs LOW
      this->pin_->digital_write(false);
      delayMicroseconds(42);
      
      // Then pull HIGH for data duration
      this->pin_->digital_write(true);
      
      // Hold high for bit duration according to protocol
      bool bit_value = (data_byte >> bit) & 0x01;
      if (bit_value) {
        delayMicroseconds(74);  // 70μs for '1'
      } else {
        delayMicroseconds(24);  // 26μs for '0'
      }
    }
  }
  
  // Pull line low for 50μs at the end of response
  this->pin_->digital_write(false);
  delayMicroseconds(16);
  
  // Return pin to input mode with pull-up
  this->pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  
  ESP_LOGD(TAG, "SI7021 response sent: %.1f°C, %.1f%% humidity", temp_value, humidity_value);
}

}  // namespace dummy
}  // namespace esphome

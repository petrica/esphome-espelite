# Dummy SI7021 Component

A dummy component for ESPHome that simulates an SI7021 temperature and humidity sensor using GPIO interrupts.

## Features

- Simulates SI7021 temperature and humidity sensor protocol
- Uses GPIO interrupts to detect host requests
- Responds with dummy values (25.0°C, 50.0% humidity)
- Realistic timing delays to simulate sensor processing
- Can be linked to ESPHome temperature and humidity sensors
- Supports Home Assistant integration through linked sensors
- Optional DHT component integration for real sensor data

## Configuration

```yaml
# Dummy Component
dummy:
  id: dummy_sensor
  pin: 
    number: GPIO05
    mode: INPUT_PULLUP
  dht_component: esp_elite_dht  # Optional: link to DHT component
  dht_update_interval: 500ms    # Optional: DHT update interval
  temperature_sensor: esp_elite_temperature  # Optional: link to temperature sensor
  humidity_sensor: esp_elite_humidity        # Optional: link to humidity sensor
  temp_age_threshold: 30s       # Optional: age threshold for internal values (default: 25s)
```

## Configuration Options

- `pin`: GPIO pin for sensor communication
- `dht_component`: Optional ID of a DHT component to integrate with
- `dht_update_interval`: Optional interval for DHT updates (default: 2000ms)
- `temperature_sensor`: Optional ID of a temperature sensor to use as fallback
- `humidity_sensor`: Optional ID of a humidity sensor to use as fallback
- `temp_age_threshold`: Optional age threshold for internal values before falling back to sensors (default: 25s)

## Files

- `__init__.py` - Python component definition
- `dummy.h` - C++ header file
- `dummy.cpp` - C++ implementation file
- `README.md` - This documentation file

## Usage

This component is useful for:
- Testing SI7021 sensor communication protocols
- Simulating sensor behavior without physical hardware
- Learning ESPHome interrupt handling and GPIO manipulation
- Debugging sensor communication issues
- Development and testing of sensor-related code
- Creating virtual sensors that can be controlled via API calls

## API Integration

The component supports setting temperature and humidity values via ESPHome API:

```yaml
api:
  actions:
    - action: set_temperature
      variables:
        value: int
      then:
        - lambda: |-
            id(dummy_sensor).set_temperature(value);
    - action: set_humidity
      variables:
        value: int
      then:
        - lambda: |-
            id(dummy_sensor).set_humidity(value);
```

When linked sensors are configured, setting values via API will automatically update the linked ESPHome sensors.

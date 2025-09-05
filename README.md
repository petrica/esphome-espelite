# ESP Elite - SONOFF TH Elite Bridge

ESP Elite is a custom ESPHome device that acts as a bridge between Home Assistant and the SONOFF TH Elite Smart Temperature and Humidity Monitoring Switch. It allows you to read real sensor data in Home Assistant while injecting custom temperature and humidity values into the eWeLink app, enabling advanced thermostat functionality with manual override capabilities.

## Purpose

The ESP Elite sits between your temperature/humidity sensor and the SONOFF TH Elite device to:

- **Read real sensor data** in Home Assistant for monitoring and automation
- **Inject custom values** into the eWeLink app to control the SONOFF TH Elite's switching behavior
- **Enable thermostat functionality** with manual temperature override capabilities
- **Support complex scenarios** like aggregating data from multiple sensors and feeding the calculated value to the SONOFF device

This allows you to create sophisticated temperature control systems where the SONOFF TH Elite acts as a thermostat but can be controlled by custom logic, multiple sensor inputs, or manual overrides from Home Assistant.

## How It Works

The ESP Elite device intercepts communication between the SONOFF TH Elite and its sensor:

1. **Real Sensor Connection**: The actual temperature/humidity sensor (SI7021) connects to the ESP Elite
2. **SONOFF Communication**: The SONOFF TH Elite connects to the ESP Elite's dummy component, which simulates the sensor
3. **Data Flow**: 
   - Real sensor data flows to Home Assistant via ESPHome
   - Custom/manipulated values are sent to the SONOFF TH Elite via the dummy component
   - The SONOFF device uses these custom values for its thermostat logic

## Hardware Configuration

- **Board**: ESP8266 (ESP12E)
- **Status LED**: GPIO2 (inverted)
- **Real Sensor**: GPIO13 (SI7021 connected here)
- **SONOFF Connection**: GPIO04 (SONOFF TH Elite connects here)

## Key Features

- **Dual Data Path**: Real sensor data to Home Assistant, custom values to SONOFF TH Elite
- **Manual Override**: Set custom temperature/humidity values via Home Assistant API
- **Sensor Aggregation**: Combine multiple sensor inputs for complex control logic
- **Thermostat Control**: Use SONOFF TH Elite's built-in thermostat with custom temperature inputs
- **Real-time Monitoring**: Monitor actual sensor readings in Home Assistant
- **SI7021 Protocol Simulation**: Seamlessly mimics sensor communication for SONOFF device

## Configuration

The main configuration is in `espelite.yaml`:

```yaml
esphome:
  name: espelite

esp8266:
  board: esp12e

# DHT Temperature and Humidity Sensor
dht:
  id: esp_elite_dht
  pin:
    number: GPIO13
    mode: INPUT_PULLUP
  model: SI7021
  temperature:
    id: esp_elite_temperature
    name: "ESP Elite Temperature"
    unit_of_measurement: "°C"
  humidity:
    id: esp_elite_humidity
    name: "ESP Elite Humidity"
    unit_of_measurement: "%"

# Dummy Component (SI7021 Simulator)
dummy:
  id: dummy_sensor
  pin: 
    number: GPIO04
    mode: INPUT_PULLUP
  dht_component: esp_elite_dht
  dht_update_interval: 500ms
  temperature_sensor: esp_elite_temperature
  humidity_sensor: esp_elite_humidity
  temp_age_threshold: 60s
```

## Home Assistant API Actions

Control the ESP Elite from Home Assistant:

- `set_temperature` - Override temperature value sent to SONOFF TH Elite
- `set_humidity` - Override humidity value sent to SONOFF TH Elite  
- `update_dht` - Manually trigger real sensor reading

**Example Use Cases:**
- Set a custom temperature to trigger heating/cooling
- Aggregate multiple sensor readings and send the average
- Implement time-based temperature adjustments
- Create complex logic that considers multiple factors

## Custom Components

### DHT Component (`custom_components/dht/`)
- `dht.h` - Header file with DHT class definition
- `dht.cpp` - Implementation with sensor communication protocol
- `__init__.py` - ESPHome component registration

### Dummy Component (`custom_components/dummy/`)
- `dummy.h` - Header file with DummyComponent class definition
- `dummy.cpp` - Implementation with SI7021 protocol simulation
- `__init__.py` - ESPHome component registration
- `README.md` - Detailed component documentation

## Use Cases

Perfect for advanced temperature control scenarios:

- **Multi-Sensor Aggregation**: Combine readings from multiple temperature sensors and feed the average to SONOFF TH Elite
- **Manual Thermostat Override**: Temporarily set custom temperatures to trigger heating/cooling
- **Time-Based Control**: Implement different temperature setpoints for day/night or seasonal adjustments
- **Conditional Logic**: Create complex rules (e.g., "if humidity > 70%, increase temperature by 2°C")
- **Remote Monitoring**: Monitor real sensor data in Home Assistant while controlling SONOFF device remotely
- **Smart Thermostat**: Use SONOFF TH Elite's built-in thermostat features with custom temperature inputs

## Building and Deployment

1. Install ESPHome
2. Copy the project files
3. Configure WiFi credentials in `espelite.yaml`
4. Build and flash to ESP8266:
   ```bash
   esphome run espelite.yaml
   ```

## Network Configuration

- **WiFi**: Configure your WiFi credentials in `espelite.yaml`
- **Fallback Hotspot**: "ESP Elite" (configure password in YAML)
- **OTA**: Configure OTA password in YAML for secure updates

## Wiring Diagram

```
Real SI7021 Sensor ──→ ESP Elite (GPIO13) ──→ Home Assistant
                                    ↓
SONOFF TH Elite ──→ ESP Elite (GPIO04) ←── Custom Values
```

## Technical Implementation

- **DHT Component**: Reads real SI7021 sensor data and publishes to Home Assistant
- **Dummy Component**: Simulates SI7021 protocol to respond to SONOFF TH Elite requests
- **API Integration**: Allows Home Assistant to inject custom temperature/humidity values
- **Age-Based Logic**: Uses custom values if recent, falls back to real sensor data if stale

## License

This project is for educational and development purposes. Please ensure compliance with any applicable licenses for ESPHome and related libraries.

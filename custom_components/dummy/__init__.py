import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PIN
from esphome import pins
from esphome.components import sensor

DummyComponent = cg.esphome_ns.namespace("dummy")
DummyComponentClass = DummyComponent.class_("DummyComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DummyComponentClass),
    cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
    cv.Optional("dht_component"): cv.use_id(cg.Component),
    cv.Optional("dht_update_interval", default="2000ms"): cv.positive_time_period_milliseconds,
    cv.Optional("temperature_sensor"): cv.use_id(sensor.Sensor),
    cv.Optional("humidity_sensor"): cv.use_id(sensor.Sensor),
    cv.Optional("temp_age_threshold", default="25s"): cv.positive_time_period_milliseconds,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))
    
    if "dht_component" in config:
        dht_component = await cg.get_variable(config["dht_component"])
        cg.add(var.set_dht_component(dht_component))
    
    # Set DHT update interval
    dht_interval = config["dht_update_interval"]
    cg.add(var.set_dht_update_interval(dht_interval.total_milliseconds))
    
    # Set temperature sensor if specified
    if "temperature_sensor" in config:
        temp_sensor = await cg.get_variable(config["temperature_sensor"])
        cg.add(var.set_temperature_sensor(temp_sensor))
    
    # Set humidity sensor if specified
    if "humidity_sensor" in config:
        humidity_sensor = await cg.get_variable(config["humidity_sensor"])
        cg.add(var.set_humidity_sensor(humidity_sensor))
    
    # Set temperature age threshold
    temp_age_threshold = config["temp_age_threshold"]
    cg.add(var.set_temp_age_threshold(temp_age_threshold.total_milliseconds))

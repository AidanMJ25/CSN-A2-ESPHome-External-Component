"""ESPHome support for the QR701 TTL thermal receipt printer."""

import inspect

import esphome.automation as automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.components import text_sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text_sensor"]

CONF_TEXT = "text"
CONF_STATUS = "status"

qr701_ns = cg.esphome_ns.namespace("qr701")
QR701 = qr701_ns.class_("QR701", cg.PollingComponent, uart.UARTDevice)
QR701PrintAction = qr701_ns.class_("QR701PrintAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(QR701),
        cv.Optional(CONF_STATUS): text_sensor.text_sensor_schema(),
    }
).extend(cv.polling_component_schema("1s")).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    if status_config := config.get(CONF_STATUS):
        status = await text_sensor.new_text_sensor(status_config)
        cg.add(var.set_status_text_sensor(status))


PRINT_ACTION_SCHEMA = cv.maybe_simple_value(
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(QR701),
            cv.Required(CONF_TEXT): cv.templatable(cv.string),
        }
    ),
    key=CONF_TEXT,
)


# ESPHome 2026.3 requires this explicit flag; keeping the decorator compatible
# with older ESPHome releases makes the external component easier to reuse.
_register_action_kwargs = {}
if "synchronous" in inspect.signature(automation.register_action).parameters:
    _register_action_kwargs["synchronous"] = True


@automation.register_action(
    "qr701.print", QR701PrintAction, PRINT_ACTION_SCHEMA, **_register_action_kwargs
)
async def qr701_print_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    template_ = await cg.templatable(config[CONF_TEXT], args, cg.std_string)
    cg.add(var.set_text(template_))
    return var

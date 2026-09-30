"""ESPHome support for the Cashino CSN-A2 thermal receipt printer."""

import inspect

import esphome.automation as automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

CONF_TEXT = "text"

csn_a2_ns = cg.esphome_ns.namespace("csn_a2")
CSNA2 = csn_a2_ns.class_("CSNA2", cg.Component, uart.UARTDevice)
CSNA2PrintAction = csn_a2_ns.class_("CSNA2PrintAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CSNA2),
    }
).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)


PRINT_ACTION_SCHEMA = cv.maybe_simple_value(
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(CSNA2),
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
    "csn_a2.print", CSNA2PrintAction, PRINT_ACTION_SCHEMA, **_register_action_kwargs
)
async def csn_a2_print_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    template_ = await cg.templatable(config[CONF_TEXT], args, cg.std_string)
    cg.add(var.set_text(template_))
    return var

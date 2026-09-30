# CSN-A2 ESPHome External Component

An ESPHome external component for the Cashino CSN-A2 58 mm thermal receipt
printer. It writes one block of text to a TTL UART printer and feeds three
lines afterwards, leaving the receipt ready to tear.

## Hardware

Use the **TTL UART** model of the CSN-A2. Connect the ESP's TX pin to the
printer RX pin and connect ground to ground. Power the printer from a supply
that meets the voltage/current rating printed on the specific unit; do not
power a printer from the ESP board's GPIO or 3.3 V rail.

The product listing also offers RS232 and USB variants. RS232 requires a
proper RS232 level shifter, while USB models require a supported ESPHome USB
host UART setup; neither connects directly to GPIO UART pins.

## Install and configure

For a Git-hosted copy of this repository, use this in the device YAML:

```yaml
external_components:
  - source: github://OWNER/REPOSITORY
    components: [csn_a2]

uart:
  id: printer_uart
  tx_pin: GPIO17
  rx_pin: GPIO16  # Optional if nothing reads from the printer.
  baud_rate: 9600 # Change if your printer is configured differently.

csn_a2:
  id: receipt_printer
  uart_id: printer_uart
```

For local development, replace the Git source with the `external_components`
block in [example.yaml](example.yaml).

## Print action

`csn_a2.print` takes either a text value directly or an object with `id` and
`text`. The text may contain line breaks and may be templated.

```yaml
on_...:
  then:
    - csn_a2.print:
        id: receipt_printer
        text: |-
          Order #42
          Thank you!
```

The component sends printable text as supplied, then sends three LF bytes for
line feed. The printer needs a compatible text code page for non-ASCII
characters; ASCII is the portable choice unless the unit's manual documents a
different code page.

## Notes

This is intentionally a text-only component: it does not reset the printer,
cut paper, set formatting, or query status. Those commands vary between
firmware variants, while the requested text-printing path stays simple and
safe.

# General
The protocol of the control interface is request–response with the first byte being the message type and the other bytes are the payload.

# Requests
## Device Info Request (0x00)
Payload:
- Byte 1-7: Unused (don't care)

## Get Button Configuration Request (0x01)
Payload:
- Byte 1: Button (0 to number of buttons - 1)
- Byte 2-7: Unused (don't care)

## Set Button Configuration Request (0x02)
Payload:
- Byte 1: Button (0 to number of buttons - 1)
- Byte 2: Keyboard scan code
- Byte 3: Keyboard modifier
- Byte 4-7: Unused (don't care)

_Note: The configuration is only stored in RAM unless a save configuration request is issued. This allows for PC controlled profiles without stressing the flash of the microcontroller_

## Get Button Debounce Request (0x03)
Payload:
- Byte 1-7: Unused (don't care)

## Set Button Debounce Request (0x04)
Payload:
- Byte 1: Button debounce time in ms
- Byte 2-7: Unused (don't care)

## Get LED Configuration Request (0x05)
Payload:
- Byte 1-7: Unused (don't care)

### Possible Led modes
| Mode Byte   | Description                       |
| ----------- | --------------------------------- |
| 0x00        | Leds disabled                     |
| 0x01        | Leds always on                    |
| 0x02        | Led is on when button is pressed  |
| 0x03        | Led is off when button is pressed |

## Set LED Configuration Request (0x06)
Payload:
- Byte 1: Led mode
- Byte 2-7: Unused (don't care)

_Note: See Get LED Configuration Request (0x03) for possible LED modes_

## Save Configuration Request (0x07)
Payload:
- Byte 1-7: Unused (don't care)

## Start Bootloader Request (0x7F)
Payload:
- Byte 1-7: Unused (don't care)

_Note: The device will restart to the internal bootloader. No response will be sent._

# Responses
The return codes are defined as:
- 0x00: OK
- 0x01: Unknown command
- 0x02: Button out of range
- 0x03: Unknown led mode
- 0x04: Save failed

## Device Info Response (0x80)
Payload:
- Byte 1: Return code
- Byte 2: Firmware version mayor
- Byte 3: Firmware version minor
- Byte 4: Number of buttons
- Byte 5-7: Unused (don't care)

## Get Button Configuration Response (0x81)
Payload:
- Byte 1: Return code
- Byte 2: Button
- Byte 3: Keyboard scan code
- Byte 4: Keyboard modifier
- Byte 5-7: Unused (don't care)

_Note: Bytes 2 to 4 contain invalid/unknown data when the return code is not OK._

## Set Button Configuration Response (0x82)
Payload:
- Byte 1: Return code
- Byte 2: Button
- Byte 3-7: Unused (don't care)

## Get Button Debounce Response (0x83)
Payload:
- Byte 1: Return code
- Byte 2: Button debounce time in ms
- Byte 3-7: Unused (don't care)

## Set Button Debounce Response (0x84)
Payload:
- Byte 1: Return code
- Byte 2-7: Unused (don't care)

## Get LED Configuration Response (0x85)
Payload:
- Byte 1: Return code
- Byte 2: Current LED configuration
- Byte 3-7: Unused (don't care)

## Set LED Configuration Request (0x86)
Payload:
- Byte 1: Return code
- Byte 2-7: Unused (don't care)

## Save configuration Response (0x87)
Payload:
- Byte 1: Return code
- Byte 2-7: Unused (don't care)
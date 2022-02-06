# USB Footswith
> Utilizing a STM32F4 microcontroller as configurable HID Keyboard with up to four buttons

## Hardware
This project is built around the "STM32 Black Pill" board which exists with multiple different controllers (STM32F411CE, STM32F401CC, STM32F401CE) which are all more or less binary compatible.
The linker file of the firmware is configured to only use up to 256 kiByte of flash memory so every version of the board should be supported, the actual usage is far below that.

Additionally up to 4 buttons and optional LEDs are needed. I used a Crate 3 Button AMP Footswitch where I replaced the toggle buttons with push buttons.

## Project Components
### Configurator
The Configurator folder contains the source of the desktop application written in C# and WPF to configure the device.

### Firmware
The Firmware folder contains the source of the microcontroller firmware. It is written in C++ utilizing the STM HAL LL drivers and tinyusb as USB stack.

#### Compiling the Firmware
The firmware was developed with VisuaGDB using the ARM toolchain (GCC 10.3.1) and the STM32 Device package (2022.01).
However it should be possible to compile the project without VisualGDB as long as the STM32F4 HAL is provided.

#### Install/Update
The easiest way to install or update the firmware is to use the internal DFU bootloader of the STM32F4. [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) or [dfu-util](http://dfu-util.sourceforge.net) can be used on the PC side to send the firmware to the device. 

### CAD
This folder contains 3D models for printing (a holder for the black pill board and a blind plug for 11 mm drill holes) and a sample schematic showing how to connect the buttons and LEDs to the board.

## Communication Protocol
The communication protocol for the HID configuration interface is described [here](https://github.com/julr/usb-footswitch/blob/main/ControlHidProtocol.md).

## Known Issues
Currently there is no support for multimedia keys. It should be possible to achieve this by adding an additional HID profile, see https://github.com/hathach/tinyusb/discussions/620. 
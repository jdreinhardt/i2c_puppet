# I2C Puppet

This is a port of the old [BB Q10 Keyboard-to-I2C Software](https://github.com/solderparty/bbq10kbd_i2c_sw) to the RP2040, expanded with new features, while still staying backwards compatible.

The target product/keyboard for this software is the BB Q20 keyboard, which adds a trackpad to the mix.

On the features side, this software adds USB support, the keyboard acts as a USB keyboard, and the trackpad acts as a USB mouse.

On the I2C side, you can access the key presses, the trackpad state, you can control some of the board GPIOs, as well as the backlight.

See [Protocol](#protocol) for details of the I2C puppet.

## Customizations

This version has been modified to the missing keys as well as provide additional functionality. The full keymap can be found [here](keymap.md).

## Checkout

The code depends on the Raspberry Pi Pico SDK, which is added as a submodule. Because the Pico SDK includes TinyUSB as a module, it is not recommended to do a recursive submodule init, and rather follow these steps:

    git clone https://github.com/solderparty/i2c_puppet
    cd i2c_puppet
    git submodule update --init
    cd 3rdparty/pico-sdk
    git submodule update --init

## Build

See the `boards` directory for a list of available boards.

    mkdir build
    cd build
    cmake -DPICO_BOARD=bbq20kbd_breakout -DCMAKE_BUILD_TYPE=Debug ..
    make

## Vendor USB Class

You can configure the software over USB in a similar way you would do it over I2C. You can access the same registers (like the backlight register) using the USB Vendor Class.
If you don't want to prefix all config interactions with `sudo`, you can copy the included udev rule:

    sudo cp etc/99-i2c_puppet.rules /lib/udev/rules.d/
    sudo udevadm control --reload
    sudo udevadm trigger

To interact with the internal registers of the keyboard over USB, use the `i2c_puppet.py` script included in the `etc` folder.
just import it, create a `I2C_Puppet` object, and you can interact with the keyboard in the same you would do using the I2C interface and the CircuitPython class linked below.

## Implementations

Here are libraries that allow I2C interaction with the boards running this software. Not all libraries might support all the features.

- [Arduino](https://github.com/solderparty/arduino_bbq10kbd)
- [CircuitPython](https://github.com/solderparty/arturo182_CircuitPython_BBQ10Keyboard)
- [Rust (Embedded-HAL)](https://crates.io/crates/bbq10kbd)

## Protocol

The device uses I2C slave interface to communicate, the address can be configured in `app/config/conf_app.h`, the default is `0x1F`.

You can read the values of all the registers, the number of returned bytes depends on the register.
It's also possible to write to the registers, to do that, apply the write mask `0x80` to the register ID (for example, the backlight register `0x05` becomes `0x85`).

### The FW Version register (REG_VER = 0x01)

Data written to this register is discarded. Reading this register returns 1 byte, the first nibble contains the major version and the second nibble contains the minor version of the firmware.

### The configuration register (REG_CFG = 0x02)

This register can be read and written to, it's 1 byte in size.

This register is a bit map of various settings that can be changed to customize the behaviour of the firmware.

See `REG_CF2` for additional settings.

| Bit    | Name             | Description                                                        |
| ------ |:----------------:| ------------------------------------------------------------------:|
| 7      | CFG_USE_MODS     | Should Alt, Sym and the Shift keys modify the keys being reported. |
| 6      | CFG_REPORT_MODS  | Should Alt, Sym and the Shift keys be reported as well.            |
| 5      | CFG_PANIC_INT    | Currently not implemented.                                         |
| 4      | CFG_KEY_INT      | Should an interrupt be generated when a key is pressed.            |
| 3      | CFG_NUMLOCK_INT  | Should an interrupt be generated when Num Lock is toggled.         |
| 2      | CFG_CAPSLOCK_INT | Should an interrupt be generated when Caps Lock is toggled.        |
| 1      | CFG_OVERFLOW_INT | Should an interrupt be generated when a FIFO overflow happens.     |
| 0      | CFG_OVERFLOW_ON  | When a FIFO overflow happens, should the new entry still be pushed, overwriting the oldest one. If 0 then new entry is lost. |

Defaut value:
`CFG_OVERFLOW_INT | CFG_KEY_INT | CFG_USE_MODS`

### Interrupt status register (REG_INT = 0x03)

When an interrupt happens, the register can be read to check what caused the interrupt. It's 1 byte in size.

| Bit    | Name             | Description                                                 |
| ------ |:----------------:| -----------------------------------------------------------:|
| 7      | N/A              | Currently not implemented.                                  |
| 6      | INT_TOUCH        | The interrupt was generated by a trackpad motion.           |
| 5      | INT_GPIO         | The interrupt was generated by a input GPIO changing level. |
| 4      | INT_PANIC        | Currently not implemented.                                  |
| 3      | INT_KEY          | The interrupt was generated by a key press.                 |
| 2      | INT_NUMLOCK      | The interrupt was generated by Num Lock.                    |
| 1      | INT_CAPSLOCK     | The interrupt was generated by Caps Lock.                   |
| 0      | INT_OVERFLOW     | The interrupt was generated by FIFO overflow.               |

After reading the register, it has to manually be reset to `0x00`.

For `INT_GPIO` check the bits in `REG_GIN` to see which GPIO triggered the interrupt. The GPIO interrupt must first be enabled in `REG_GIC`.

### Key status register (REG_KEY = 0x04)

This register contains information about the state of the fifo as well as modified keys. It is 1 byte in size.

| Bit    | Name             | Description                                     |
| ------ |:----------------:| -----------------------------------------------:|
| 7      | N/A              | Currently not implemented.                      |
| 6      | KEY_NUMLOCK      | Is Num Lock on at the moment.                   |
| 5      | KEY_CAPSLOCK     | Is Caps Lock on at the moment.                  |
| 0-4    | KEY_COUNT        | Number of items in the FIFO waiting to be read. |

### Backlight control register (REG_BKL = 0x05)

Internally a PWM signal is generated to control the keyboard backlight, this register allows changing the brightness of the backlight. It is 1 byte in size, `0x00` being off and `0xFF` being the brightest.

Default value: `0xFF`.

### Debounce configuration register (REG_DEB = 0x06)

Currently not implemented.

Default value: 10

### Poll frequency configuration register (REG_FRQ = 0x07)

Currently not implemented.

Default value: 5

### Chip reset register (REG_RST = 0x08)

Reading or writing to this register will cause a SW reset of the chip.

### FIFO access register (REG_FIF = 0x09)

This register can be used to read the top of the key FIFO. It returns two bytes, a key state and a key code.

Possible key states:

| Value  | State                   |
| ------ |:-----------------------:|
| 1      | Pressed                 |
| 2      | Pressed and Held        |
| 3      | Released                |

### Secondary backlight control register (REG_BK2 = 0x0A)

Internally a PWM signal is generated to control a secondary backlight (for example, a screen), this register allows changing the brightness of the backlight. It is 1 byte in size, `0x00` being off and `0xFF` being the brightest.

Default value: `0xFF`.

### GPIO direction register (REG_DIR = 0x0B)

This register controls the direction of the GPIO expander pins, each bit corresponding to one pin. It is 1 byte in size.

The actual pin[7..0] to MCU pin assignment depends on the board, see `<board>.h` of the board for the assignments.

Any bit set to `1` means the GPIO is configured as input, any bit set to `0` means the GPIO is configured as output.

Default value: `0xFF` (all GPIOs configured as input)

### GPIO input pull enable register (REG_PUE = 0x0C)

If a GPIO is configured as an input (using `REG_DIR`), a optional pull-up/pull-down can be enabled.

This register controls the pull enable status, each bit corresponding to one pin. It is 1 byte in size.

The actual pin[7..0] to MCU pin assignment depends on the board, see `<board>.h` of the board for the assignments.

Any bit set to `1` means the input pull for that pin is enabled, any bit set to `0` means the input pull for that pin is disabled.

The direction of the pull is done in `REG_PUD`.

When a pin is configured as output, its bit in this register has no effect.

Default value: 0 (all pulls disabled)

### GPIO input pull direction register (REG_PUD = 0x0D)

If a GPIO is configured as an input (using `REG_DIR`), a optional pull-up/pull-down can be configured.

The pull functionality is enabled using `REG_PUE` and the direction of the pull is configured using this register, each bit corresponding to one pin. This register is 1 byte in size.

The actual pin[7..0] to MCU pin assignment depends on the board, see `<board>.h` of the board for the assignments.

Any bit set to `1` means the input pull is set to pull-up, any bit set to `0` means the input pul is set to pull-down.

When a pin is configured as output, its bit in this register has no effect.

Default value: `0xFF` (all pulls set to pull-up, if enabled in `REG_PUE` and set to input in `REG_DIR`)

### GPIO value register (REG_GIO = 0x0E)

This register contains the values of the GPIO Expander pins, each bit corresponding to one pin. It is 1 byte in size.

The actual pin[7..0] to MCU pin assignment depends on the board, see `<board>.h` of the board for the assignments.

If a pin is configured as an output (via `REG_DIR`), writing to this register will change the value of that pin.

Reading from this register will return the values for both input and output pins.

Default value: Depends on pin values

### GPIO interrupt config register (REG_GIC = 0x0F)

If a GPIO is configured as an input (using `REG_DIR`), an interrupt can be configured to trigger when the pin's value changes.

This register controls the interrupt, each bit corresponding to one pin.

The actual pin[7..0] to MCU pin assignment depends on the board, see `<board>.h` of the board for the assignments.

Any bit set to `1` means the input pin will trigger and interrupt when changing value, any bit set to `0` means no interrupt for that pin is triggered.

When an interrupt happens, the GPIO that triggered the interrupt can be determined by reading `REG_GIN`. Additionally, the `INT_GPIO` bit will be set in `REG_INT`.

Default value: `0x00`

### GPIO interrupt status register (REG_GIN = 0x10)

When an interrupt happens, the register can be read to check which GPIO caused the interrupt, each bit corresponding to one pin. This register is 1 byte in size.

The actual pin[7..0] to MCU pin assignment depends on the board, see `<board>.h` of the board for the assignments.

After reading the register, it has to manually be reset to `0x00`.

Default value: `0x00`

### Key hold threshold configuration (REG_HLD = 0x11)

This register can be read and written to, it is 1 byte in size.

The value of this register (expressed in units of 10ms) is used to determine if a "press and hold" state should be entered.

If a key is held down longer than the value, it enters the "press and hold" state.

Default value: 30 (300ms)

### Device I2C address (REG_ADR = 0x12)

The address that the device shows up on the I2C bus under. This register can be read and written to, it is 1 byte in size.

The change is applied as soon as a new value is written to the register. The next communication must be performed on the new address.

The address is not saved after a reset.

Default value: `0x1F`

### Interrupt duration (REG_IND = 0x13)

The value of this register determines how long the INT/IRQ pin is held LOW after an interrupt event happens.This register can be read and written to, it is 1 byte in size.

The value of this register is expressed in ms.

Default value: 1 (1ms)

### The configuration register 2 (REG_CF2 = 0x14)

This register can be read and written to, it's 1 byte in size.

This register is a bit map of various settings that can be changed to customize the behaviour of the firmware.

See `REG_CFG` for additional settings.

| Bit    | Name             | Description                                                        |
| ------ |:----------------:| ------------------------------------------------------------------:|
| 7      | N/A              | Currently not implemented.                                         |
| 6      | N/A              | Currently not implemented.                                         |
| 5      | N/A              | Currently not implemented.                                         |
| 4      | N/A              | Currently not implemented.                                         |
| 3      | N/A              | Currently not implemented.                                         |
| 2      | CF2_USB_MOUSE_ON | Should trackpad events be sent over USB HID.                       |
| 1      | CF2_USB_KEYB_ON  | Should key events be sent over USB HID.                            |
| 0      | CF2_TOUCH_INT    | Should trackpad events generate interrupts.                        |

Default value: `CF2_TOUCH_INT | CF2_USB_KEYB_ON | CF2_USB_MOUSE_ON`

### Trackpad X Position(REG_TOX = 0x15)

This is a read-only register, it is 1 byte in size.

Trackpad X-axis position delta since the last time this register was read.

The value reported is signed and can be in the range of (-128 to 127).

When the value of this register is read, it is afterwards reset back to 0.

It is recommended to read the value of this register often, or data loss might occur.

Default value: 0

### Trackpad Y position (REG_TOY = 0x16)

This is a read-only register, it is 1 byte in size.

Trackpad Y-axis position delta since the last time this register was read.

The value reported is signed and can be in the range of (-128 to 127).

When the value of this register is read, it is afterwards reset back to 0.

It is recommended to read the value of this register often, or data loss might occur.

Default value: 0

## Version history

	v1.0:
	- Initial release

See here for the legacy project's history: https://github.com/solderparty/bbq10kbd_i2c_sw#version-history

## Thanks

A special thanks to the forks by [grymoire](https://github.com/grymoire/i2c_puppet-Linux) and [DJFliX](https://github.com/DJFliX/bbq20kbd) whose work influenced the results of this one.
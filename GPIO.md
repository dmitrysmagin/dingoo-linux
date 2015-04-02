Summary of GPIO usage.

# Introduction #

Mapping the GPIO usage is usually a tought task. In the A320 we have had help from the fact that:

  * You can make programs for the original firmware using the provided SDK.
  * The OS does not use the MMU to protect the special registers, so you can write a program that can read the state of every register (and even change it).

Having a "known good" set of values for certain registers helped a lot setting up the linux kernel for the A320.

# General information about GPIO on the JZ4740 #

The JZ4740 has four 32 bit GPIO ports. Each port is controlled by eight registers:

  * PXPIN: state of the pin (read only).
  * PXDAT: output value.
  * PXIM:  interrupt mask (1=mask interrupt).
  * PXPE:  pull resistor control (1=disable).
  * PXFUN: function control (0=GPIO/interrupt, 1=special function).
  * PXSEL: function select  (0=GPIO/special function 0, 1=interrupt/special function 1).
  * PXDIR: pin direction (0=input/low level int/falling edge int, 1=output/high level int/raising edge int).
  * PXTRG: trigger selection (0=level trigger, 1=edge trigger).
  * PXFLG: interrupt flag.

For PXDAT, PXIM, PXPE, PXFUN, PXSEL, PXDIR and PXTRG there are corresponding "set" and "clear" registers. For PXFLG there is only a "clear" register.

For example, if you want to set some bits of PXDAT, you must write a value with those bits set to PXDATS. If you want to clear some bits, you must write a value with those bits set to PXDATC.

# Port A #

This por is entirely used for SDRAM. These are the values read by an application running on the original firmware.

| PXDAT | 0x00000000 | 0000 0000 0000 0000 0000 0000 0000 0000 |
|:------|:-----------|:----------------------------------------|
| PXIM  | 0xFFFFFFFF | 1111 1111 1111 1111 1111 1111 1111 1111 |
| PXPE  | 0x000000FF | 0000 0000 0000 0000 0000 0000 1111 1111 |
| PXFUN | 0xFFFFFFFF | 1111 1111 1111 1111 1111 1111 1111 1111 |
| PXSEL | 0x00000000 | 0000 0000 0000 0000 0000 0000 0000 0000 |
| PXDIR | 0x00000000 | 0000 0000 0000 0000 0000 0000 0000 0000 |
| PXTRG | 0x00000000 | 0000 0000 0000 0000 0000 0000 0000 0000 |

# Port B #

Most of this port is also used for SDRAM.

| PXDAT | 0x00040000 | 0000 0000 0000 0100 0000 0000 0000 0000 |
|:------|:-----------|:----------------------------------------|
| PXIM  | 0xDFFFFFFF | 1101 1111 1111 1111 1111 1111 1111 1111 |
| PXPE  | 0x02418000 | 0000 0010 0100 0001 1000 0000 0000 0000 |
| PXFUN | 0x9FF9FFFF | 1001 1111 1111 1001 1111 1111 1111 1111 |
| PXSEL | 0x20000000 | 0010 0000 0000 0000 0000 0000 0000 0000 |
| PXDIR | 0x20000000 | 0010 0000 0000 0000 0000 0000 0000 0000 |
| PXTRG | 0x20000000 | 0010 0000 0000 0000 0000 0000 0000 0000 |

|  0 | SDRAM A0 |
|:---|:---------|
|  1 | SDRAM A1 |
|  2 | SDRAM A2 |
|  3 | SDRAM A3 |
|  4 | SDRAM A4 |
|  5 | SDRAM A5 |
|  6 | SDRAM A6 |
|  7 | SDRAM A7 |
|  8 | SDRAM A8 |
|  9 | SDRAM A9 |
| 10 | SDRAM A10 |
| 11 | SDRAM A11 |
| 12 | SDRAM A12 |
| 13 | SDRAM A13 |
| 14 | SDRAM A14 |
| 15 | NAND CL |
| 16 | NAND AL |
| 17 | **GPIO output CS# signal to LCD** |
| 18 | **GPIO output RESET# signal to LCD** |
| 19 | SDRAM DCS# |
| 20 | SDRAM RAS# |
| 21 | SDRAM CAS# |
| 22 | SDRAM RDWE#/BUFD# |
| 23 | SDRAM CKE |
| 24 | SDRAM CK0 |
| 25 | NAND CS1# |
| 26 | NAND CS2# |
| 27 | NAND CS3# |
| 28 | NAND CS4# |
| 29 | **GPIO input card detection (1=inserted)** |
| 30 | **GPIO input battery charge status (0=charging, 1=charge finished)** |
| 31 | SDRAM WE0# |

# Port C #

Most of this port is used for LCD/TV out.

| PXDAT | 0x00280000 | 0000 0000 0010 1000 0000 0000 0000 0000 |
|:------|:-----------|:----------------------------------------|
| PXIM  | 0x7FFDFFFF | 0111 1111 1111 1101 1111 1111 1111 1111 |
| PXPE  | 0x703CFFFF | 0111 0000 0011 1100 1111 1111 1111 1111 |
| PXFUN | 0x3714FFFF | 0011 0111 0001 0100 1111 1111 1111 1111 |
| PXSEL | 0x00020000 | 0000 0000 0000 0010 0000 0000 0000 0000 |
| PXDIR | 0x08280000 | 0000 1000 0010 1000 0000 0000 0000 0000 |
| PXTRG | 0x00020000 | 0000 0000 0000 0010 0000 0000 0000 0000 |

Pin usage:

|  0 | LCD D0 |
|:---|:-------|
|  1 | LCD D1 |
|  2 | LCD D2 |
|  3 | LCD D3 |
|  4 | LCD D4 |
|  5 | LCD\_D5 |
|  6 | LCD\_D6 |
|  7 | LCD\_D7 |
|  8 | LCD\_D8 |
|  9 | LCD\_D9 |
| 10 | LCD\_D10 |
| 11 | LCD\_D11 |
| 12 | LCD\_D12 |
| 13 | LCD\_D13 |
| 14 | LCD\_D14 |
| 15 | LCD\_D15 |
| 16 | GPIO input unused |
| 17 | **GPIO input START button** |
| 18 | LCD PCLK (not connected to LCD, connected to CH7024 XCLK input) |
| 19 | **GPIO output RS# signal to LCD** |
| 20 | LCD VSYNC (connected to LCD WR# signal) |
| 21 | **GPIO output=1, connected to CH7024 LCD\_DE signal, used when in TV out mode, GPIO output=1 to keep the CH7024 disabled when not in TV out mode)** |
| 22 | GPIO input unused |
| 23 | GPIO input unused |
| 24 | SDRAM WE1# |
| 25 | SDRAM WE2# |
| 26 | SDRAM WE3# |
| 27 | **GPIO output internal speakers audio amplifiers shutdown (0=no sound)** |
| 28 | NAND FRE# |
| 29 | NAND FWE# |
| 30 | **GPIO input, connected to NAND read/busy signal** |
| 31 | **There is not a PC31 pin, this is used to select between JTAG and UART functions, which share the same set of pins. PXSEL=1 selects UART** |

# Port D #

| PXDAT | 0x80100098 | 1000 0000 0001 0000 0000 0000 1001 1000 |
|:------|:-----------|:----------------------------------------|
| PXIM  | 0xC7B13F98 | 1100 0111 1011 0001 0011 1111 1001 1000 |
| PXPE  | 0xF1A02100 | 1111 0001 1010 0000 0010 0001 0000 0000 |
| PXFUN | 0x27803F00 | 0010 0111 1000 0000 0011 1111 0000 0000 |
| PXSEL | 0x3FCEC067 | 0011 1111 1100 1110 1100 0000 0110 0111 |
| PXDIR | 0xC0300090 | 1100 0011 0000 0000 0000 0000 1001 0000 |
| PXTRG | 0x384EC067 | 0011 1000 0100 1110 1100 0000 0110 0111 |

|  0 | **GPIO input A button** |
|:---|:------------------------|
|  1 | **GPIO input B button** |
|  2 | **GPIO input Y button** |
|  3 | GPIO input, unused but tied to a test point according to schematic |
|  4 | GPIO output=1, unused but tied to a test point according to schematic |
|  5 | **GPIO input DPAD LEFT** |
|  6 | **GPIO input DPAD UP** |
|  7 | **GPIO output=1, earphones audio MUTE control (0=mute, though actually just makes the sound bad)** |
|  8 | miniSD CMD |
|  9 | miniSD CLK |
| 10 | miniSD D0 |
| 11 | miniSD D1 |
| 12 | miniSD D2 |
| 13 | miniSD D3 |
| 14 | **GPIO input LEFT SHOULDER button** |
| 15 | **GPIO input RIGHT SHOULDER button** |
| 16 | GPIO input, unused according to schematic (but seems that there's an external driver present because readback fails if configured as output) |
| 17 | **GPIO input SELECT button** |
| 18 | **GPIO input DPAD RIGHT** |
| 19 | **GPIO input X button** |
| 20 | **GPIO output=1, battery charge control (1=enable)** (grounds a resistor which controls the charge current, note that this pin is also SSI data output and thus with some work could be used as a PWM output to control charge current) |
| 21 | GPIO output=0, connectedto the mistery W35 chip |
| 22 | **GPIO input HOLD position of power slider** |
| 23 | I2C SDA, connected to TV and FM chips |
| 24 | I2C SCK, connected to TV and FM chips |
| 25 | UART0 TXD, serial console |
| 26 | UART0 RXD, serial console |
| 27 | **GPIO input DPAD DOWN** |
| 28 | **GPIO input USB power detection (1=power present)** |
| 29 | **POWER BUTTON, but FUN=1 (jz4740.h says this pin does not have special function, but seems to be wrong since it's clearly WAKE\_UP)** |
| 30 | GPIO output=0, though depicted in the schematic as input from the mistery W35 chip |
| 31 | **GPIO output LCD backlight (1=max, use as PWM7 to obtain different levels)** |
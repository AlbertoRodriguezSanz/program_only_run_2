# AksIM2-PIC18f26k83-Firmware
This repository contains the adapted firmware from the current PIC18F2580 microcontroller for the new PIC18F26K83 microcontroller implementing the updated versions of the ECAN and SPI modules working in the "Run" device operation mode, which does not include any power saving modes of operation or features. 

## Setup
A breadboard with two PIC18F26K83 microcontrollers connected through two MCP2561 CAN transceivers is used as a testbench. 

![CAN_SPI_Combined_Breadboard](https://github.com/AlbertoRodriguezSanz/CAN-SPI-combined-test/assets/95371514/404d348b-14a0-4436-b22a-9cd55a8368b2)


For this test the PICkit4 in-circuit debugger/programmer will be used to load the firmware into the microcontroller. This needs to be connected to the microcontroller with the following pins.
- MCLR (needs to be connected through two series pull-up resistors: 10kΩ and 100-470Ω to the power supply)
- PGD
- PGC
- VDD
- VSS

The microcontrollers are connected through two MCP2561 CAN transceivers. For such short distances, the use of termination resistors is not required. The two connectors represent the PICKIT4 pins required for programming each of the two microcontrollers. 

![CAN_SPI_COMBINED_BREADBOARD_SCHEMATIC](https://github.com/AlbertoRodriguezSanz/CAN-SPI-combined-test/assets/95371514/73dd19ac-99d1-4d03-aa4f-65dd76d71c20)


## Device configuration

- Configuration bits (CONFIG1-5H/L registers)
  - Oscillator source at power-up: High frequency internall oscillator with no clock division applied
  - Master Clear and Low Voltage Programming: MCLR and LVP are enabled, making the MCLR pin work for as a master clear for programming.
  - Brown-out Reset: Disabled, device will not reset when voltage drops under a certain threshold.
  - Watchdog Timer: Disabled, the Windowed Watchdog Timer will not be enabled for this test, to check for timing inaccuracies while executing instructions.
- Clock Manager (OSCCON1 and OSCFRQ registers)
  -   Clock Frequency: 16MHz, proceeding from a 64MHz base High-Frequency Oscillator Clock
- Pin Manager:
  -  CAN Transmit (CANTX0) -> RB2 (output)
  -  CAN Receive (CANRX0) -> RB3 (input)
  -  LED pin -> RC2 (output)
  -  Slave Select -> RA5 (Master -> output / Slave -> input)
  -  SPI clock -> RC3 (Master -> output / Slave -> input)
  -  SPI Data Out (SDO) -> RC5 (output)
  -  SPI Data In (SDI) -> RC4 (input)


## ECAN module configuration

Both microcontrollers implement these module parameters through the Microchip Code Configurator plugin (MCC):
- Clock Settings
  - Clock Source: Use system clock as CAN system clock
  - Clock Frequency: 16MHz
- Bit Rate Settings
  - CAN bus speed: 1Mbps
  - Time Quanta: 8
  - Sync Jumpt Width: 1xTQ
  - Sample Point: 75%
  - Phase Segment 1: 4xTQ
  - Phase Segment 2: 2xTQ
  - Propagation Segment: 1xTQ
- Transmit-Receive Settings
  - Operation Mode: Mode 0 (Legacy)

No filters or masks are setup for the CAN bus communications beforehand. Messages' ID are filtered in the `main.c` file as in the previous firmware for the PIC18F2580 microcontroller.
 
## SPI module configuration

Unlike previous tests the SPI module is not implemented through the Foundation Service Libraries (FSL) integrated in the MCC. The specifics for each microcontroller are described on the respective README files.
  
## Requirements

Install MPLAB X IDE tool for Windows, Linux or MAC from the following link ([download link](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs)).

## Operation

Once MPLAB is opened, load the project through *File > Open Project* and then select the file `Program_Run_Only_Mode.mc3`, where X denotates the firmware for each of the microcontrollers.
This will open the work environment, where `main.c` is the code file that will be compiled into the PIC. The project properties are accessed through *Production > Set Project Configuration > Customize...*, where the PICkit4 needs to be selected in the *Connected Hardware Tool* menu.

![Screenshot from 2023-09-01 14-19-52](https://github.com/AlbertoRodriguezSanz/CAN-Bus-Test/assets/95371514/248a38f8-ebf5-4f62-97c1-47c6fd496216)

Modify the following options from the default parameters for the PICkit4 programmer from the Option categories dropdown menu.
-Power
  - Power target circuit from PICkit4.
  - Voltage Level: 5V.
- PICkit4 Tool Options
  - Program Speed: Low (Otherwise an error will pop up when trying to load the firmware).

Then, follow the next steps:
* Compile: `Production> Build Main Project`
* Program: `Production> Make and Program Device Main Project`




## Interfacing with the AksIM-2 encoder

A 1 Mbps CAN channel is used to interface with the receiver code running on the PIC. Encoder data (in joint space, expressed in degrees) can be retrieved in two operation modes: continuous stream (push mode) and on demand (pull mode). In push mode, encoder reads are streamed after the start command is issued, using the specified delay, until a stop command is received. All commands (as well as the streamed data) return an acknowledge message with the corresponding operation code, i.e., the returned message ID is op code + canId.

| command                                    | request payload                   | response payload  | op code |
|--------------------------------------------|-----------------------------------|-------------------|---------|
| *continuous data stream*<br>*in push mode* | doesn't apply                     | *value* (4 bytes) | 0x80    |
| start push mode                            | 0x01 (byte 0)<br>*delay* (byte 1) | empty             | 0x100   |
| stop push mode                             | 0x02                              | empty             | 0x100   |
| poll current value                         | 0x03                              | *value* (4 bytes) | 0x180   |

Example of the expected bit timing diagram for both devices operating at a bus clock of 4MHz exchanging 4 bytes of data uninterrupted probed from the Slave device. 

![aksim_test_spi](https://github.com/AlbertoRodriguezSanz/aksim_2_pic_firmware/assets/95371514/79aa7ae9-2297-4425-98a8-715deb917421)

- Channel 1 (Yellow): Slave Select (SS)
- Channel 2 (Green): SPI Clock (SCK)
- Channel 3 (Purple): Slave Serial Data Out (SDO, MISO)
- Channel 4 (Blue): Slave Serial Data In (SDI, MOSI)

## See also

* Víctor César Sanz Labella, *Diseño del circuito electrónico de control y programación de una mano subactuada para el robot humanoide TEO*, bachelor's thesis, Universidad Carlos III de Madrid, 2014. ([cui-pic-firmware](https://github.com/roboticslab-uc3m/cui-pic-firmware))




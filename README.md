# AksIM2-PIC18f26k83-Firmware
This repository contains the adapted firmware from the current PIC18F2580 microcontroller for the new PIC18F26K83 microcontroller implementing the updated versions of the ECAN and SPI modules working in the "Run" device operation mode, which does not include any power saving modes of operation or features. 



## Setup
A breadboard with two PIC18F26K83 microcontrollers connected through two MCP2561 CAN transceivers is used as a testbench.
![broadboard_can_bus_top_viewjpg](https://github.com/AlbertoRodriguezSanz/CAN-Bus-Test/assets/95371514/c0f4a20e-199d-4b0a-b0b2-8a69f7578277)

The following parameters are configured with the Microchip Code Configurator plugin (MCC):
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
 
For this test the PICkit4 in-circuit debugger/programmer will be used to load the firmware into the microcontroller. This needs to be connected to the microcontroller with the following pins.
- MCLR
- PGD
- PGC
- VDD
- VSS
![pickit4 connections](https://github.com/AlbertoRodriguezSanz/CAN-Bus-Test/assets/95371514/aca34265-d625-4ffe-b99b-b4cd80b32269)

On the protoboard the MCLR pin needs to include two resistors of 10kΩ (R1) and 470Ω (R2) máx.
![mclr connection](https://github.com/AlbertoRodriguezSanz/CAN-Bus-Test/assets/95371514/b95aa3bf-8a20-4b94-be2d-364be542fe47)

  
## Requirements

Install MPLAB X IDE tool for Windows, Linux or MAC from the following link ([download link](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide#tabs)).

## How to download the firmware to the PIC

Once MPLAB is opened, load the project through *File > Open Project* and then select the file `CAN_bus_x.mc3`, where X denotates the firmware for each of the microcontrollers.
This will open the work environment, where `main.c` is the code file that will be compiled into the PIC. The project properties are accessed through *Production > Set Project Configuration > Customize...*, where the PICkit4 needs to be selected in the *Connected Hardware Tool* menu.
![Screenshot from 2023-09-01 14-19-52](https://github.com/AlbertoRodriguezSanz/CAN-Bus-Test/assets/95371514/248a38f8-ebf5-4f62-97c1-47c6fd496216)

Modify the following options from the default parameters for the PICkit4 programmer from the Option categories dropdown menu.
-Power
  - Power target circuit from PICkit4.
  - Voltage Level: 5V.
- PICkit4 Tool Options
  - Program Speed: Low (Otherwise an error will pop up when trying to load the firmware).

First, modify the `canId` variable corresponding to the ID of that encoder. The correspondence is detailed in [this diagram](https://robots.uc3m.es/teo-developer-manual/diagrams.html#joint-indexes). A value of 100 must be added to the ID of the iPOS node. Example: for the elbow of the left arm joint ID 24, use `canId = 124`.

Then, follow the next steps:
* Compile: `Project> Build All`
* Select the programmer: `Programmer> Select Programmer> MPLAB ICD 2`
* Connect the programmer to the PIC: `Programmer> Connect`
* Program: `Programmer> Program`

## Interfacing with the AksIM-2 encoder

A 1 Mbps CAN channel is used to interface with the receiver code running on the PIC. Encoder data (in joint space, expressed in degrees) can be retrieved in two operation modes: continuous stream (push mode) and on demand (pull mode). In push mode, encoder reads are streamed after the start command is issued, using the specified delay, until a stop command is received. All commands (as well as the streamed data) return an acknowledge message with the corresponding operation code, i.e., the returned message ID is op code + canId.

| command                                    | request payload                   | response payload  | op code |
|--------------------------------------------|-----------------------------------|-------------------|---------|
| *continuous data stream*<br>*in push mode* | doesn't apply                     | *value* (4 bytes) | 0x80    |
| start push mode                            | 0x01 (byte 0)<br>*delay* (byte 1) | empty             | 0x100   |
| stop push mode                             | 0x02                              | empty             | 0x100   |
| poll current value                         | 0x03                              | *value* (4 bytes) | 0x180   |

Example of the expected bit timing diagram for both devices operating at a bus clock of 4MHz exchanging 4 bytes of data uninterrupted probed from the Slave device. 

![SPI_full_duplex_4_bytes_wirepuller](https://github.com/AlbertoRodriguezSanz/SPI-Master-Full-Duplex-Test/assets/95371514/8cec0679-3de8-4592-b545-275d96177c6c)


- Channel 1 (Yellow): Slave Select (SS)
- Channel 2 (Green): SPI Clock (SCK)
- Channel 3 (Purple): Slave Serial Data Out (SDO, MISO)
- Channel 4 (Blue): Slave Serial Data In (SDI, MOSI)

## See also

* Víctor César Sanz Labella, *Diseño del circuito electrónico de control y programación de una mano subactuada para el robot humanoide TEO*, bachelor's thesis, Universidad Carlos III de Madrid, 2014. ([cui-pic-firmware](https://github.com/roboticslab-uc3m/cui-pic-firmware))




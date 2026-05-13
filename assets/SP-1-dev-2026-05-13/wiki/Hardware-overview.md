
![SP-1 internals](https://github.com/timknapen/SP-1-dev/blob/main/img/pcb_web.jpg)

## Main components

- [nRF52840](https://www.nordicsemi.com/Products/nRF52840) - Nordic ARM Cortex M4 32 bit MCU at 64 MHz, 1 MB flash and 256 kB RAM
- [TAS2505](https://www.ti.com/product/TAS2505) - TI audio amplifier for speaker. See also [TAS2505 Application Reference Guide](https://www.ti.com/lit/ug/slau472c/slau472c.pdf)
- [CS42L42](https://www.cirrus.com/products/cs42l42) - Cirrus Logic audio codec and amp for headphones. 
- [THGBMNG5D1LBAIL](https://media.digikey.com/pdf/Data%20Sheets/Toshiba%20PDFs/THGBMNG5D1LBAIL.pdf) - Toshiba / Kioxia 4GB eMMC 5.0 compatible flash memory.
- [CYBT-353027-02](https://www.infineon.com/part/CYBT-353027-02) - Infineon Bluetooth classic/BLE Audio module
- [BQ24232](https://www.ti.com/product/BQ24232) - Texas Instruments USB-friendly lithium-ion battery charger and power-path management

## Overview

The main mcu in stem player is the wonderful [nRF52840](https://www.nordicsemi.com/Products/nRF52840), which surprisingly isn't used here for its well known BLE capabilities. There is no antenna connected to the MCU on the stem player PCB. All bluetooth functionality is provided by the [CYBT-353027-02](Bluetooth-Module) module. 
The nRF52840 has an external crystal for the high frequency clock but synthesizes the LFCLK itself.

### Audio

All audio runs over I2S at 48kHz, 24 bit stereo. The main clock for this comes from the 3.07 MHz oscillator marked A0XRD. [TAS2505](i2c) drives the speaker and is setup and controlled via I2C. The Cirrus [CS42L42](i2c) chip drives the headphones, provides the I2S LRCLK and is also setup via I2C. The [CYBT-353027-02](Bluetooth-Module) Bluetooth/BLE module is configured via UART, but also receives audio via I2S.

### General IO

There is only one button that is directly read by the MCU via GPIO: the Function button. All other buttons are part of a resistor ladder read by 2 ADC pins on the MCU. The play button and the 4 track buttons form the first resistor ladder group. The second group consists of the volume up and down buttons and the forward / reverse buttons. All 8 LEDs are driven directly by the MCU.

### Memory

A 4 GB flash memory chip stores all the audio data and is connected via 1 bit eMMC: only DAT0, CMD and CLK are used to transfer data to the MCU. 

### USB

The USB connector is wired for battery charging and the data lines are connected to the MCU too. We can use it for CDC ACM aka serial communication.

### MIDI / PO sync

A second headphone jack can be used for MIDI or Pocket Operator sync.


## Pinout

The full pinout of the MCU, ready for development is here: [stem player pins](https://github.com/timknapen/SP-1-dev/blob/main/src/stemplayer_pins.h)
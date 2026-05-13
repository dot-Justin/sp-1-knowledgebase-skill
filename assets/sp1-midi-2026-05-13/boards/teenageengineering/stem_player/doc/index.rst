Stem Player SP-1
================

Overview
--------
This board definition provides support for the Teenage Engineering Stem Player (SP-1).
It is based on the Nordic nRF52840 SoC.

Hardware
--------
- nRF52840 MCU
- CS42L42 Headphone Codec (I2C 0x48)
- TAS2505 Speaker Amplifier (I2C 0x18)
- CYBT-353027-02 Bluetooth Module (UART0)
- Toshiba 4GB eMMC (1-bit custom interface)
- BQ24232 Battery Charger

Supported Features
------------------
- Console (USB CDC ACM)
- 8x PWM LEDs
- 1x GPIO Button (Function)
- 7x ADC Channels (4 Faders, 2 Button Ladders, 1 Battery Sense)
- I2C (Codec and Amp)
- I2S (Audio Out)

Known Issues
------------
- TAS2505 may NACK I2C transactions if a full SWD debug cable is attached.
- Audio clocking strictly requires the external 3.072 MHz oscillator and CS42L42 to be initialized before I2S starts.

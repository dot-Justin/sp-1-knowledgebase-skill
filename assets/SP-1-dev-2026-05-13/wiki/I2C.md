
I2C is used to interface with the [TAS2505](i2c#tas2505) and [CS42L42](i2c#cs42l42) amplifiers.
To use these amplifiers, you will need to first configure them.

TAS2505 is at I2C address `0x18` and CS42L42 is at `0x48`

The stem player PCB has 4.7K pull up resistors on both SDA and SCL and has been tested to work with nrfx_twim at up to 400kHz.

# CS42L42 

CS42L42 is the amp that drives the headphones in the stem player and provides the I2S LRCLK.
To use CS42L42 or play any audio over I2S, you will need to configure it via I2C.

CS42L42 is at I2C address `0x48`.

CS42L42 has the ability to act as a mic input, and HSIN- and HSIN+ seem to be correctly setup for this, but there is no audio data path from CS42L42 back to the MCU on the stem player. (ASP_SDOUT is unconnected)

## Minimal examples

All the examples follow the [Cirrus Logic CS42L42 datasheet](https://statics.cirrus.com/pubs/proDatasheet/CS42L42_DS1083F8.pdf). Registers and setup operations are well documented in this datasheet. 

#### CS42L42 setup example

This examples shows how to start up CS42L42 specifically for SP-1.
See p.91: 5.1 Power-up sequence

```C
// 1 Apply all relevant power supplies, then assert RST before applying SCLK
// and LRCK to the CS42L42.
nrf_gpio_cfg_output(PIN_CS42_RST);
nrf_gpio_pin_write(PIN_CS42_RST, 0);
nrf_delay_ms(1);

// Enable Master clock / Bit clock from oscillator
nrf_gpio_pin_write(PIN_I2S_OSC_EN, 1);

nrf_gpio_pin_write(PIN_CS42_RST, 1); // disable reset

// 2 Wait 2.5 ms.
nrf_delay_ms(3);                    

// set address
i2c_set_address(I2C_ADDR_CS42L42);

i2c_write(0x00, 0x15);
i2c_write(0x08, 0x10); // 0x1508   PLL Control 3
i2c_write(0x00, 0x15);
i2c_write(0x04, 0x80); // 0x1504   PLL Division Fractional Bytes 0–2
i2c_write(0x00, 0x15);
i2c_write(0x05, 0x3e); // 0x1505   PLL Division Integer
i2c_write(0x00, 0x15);
i2c_write(0x0a, 0x7d); // 0x150a   PLL Calibration Ratio


// Master clock
i2c_write(0x00, 0x10);
i2c_write(0x09, 0x00); // 0x1009   MCLK Control
i2c_write(0x00, 0x12);
i2c_write(0x01, 0x01); // 0x1201   MCLK Source Select
i2c_write(0x00, 0x12);
i2c_write(0x0a, 0x01); // 0x120a   Input ASRC Clock Select
i2c_write(0x00, 0x12);
i2c_write(0x0b, 0x01); // 0x120b   Output ASRC Clock Select

i2c_write(0x00, 0x15);
i2c_write(0x01, 0x01); // 0x1501   PLL Control 1

i2c_write(0x00, 0x11);
i2c_write(0x07, 0x01); // 0x1107   Oscillator Switch Control
/* SCLK must be running first!  */

// see p.94: 4.2 Confirm the RCO is powered down
// Oscillator Switch Status. Read 0x1109 
// (repeat until value is 0x02: SCLK selected for internal MCLK)
uint8_t response;
uint8_t retries = 0;
do {
    nrf_delay_ms(1);
    i2c_write(0x00, 0x11);
    i2c_read(0x09, &response, 1); // 0x1109
    retries++;
    if (retries >= 10) {
        NRF_LOG_ERROR("CS42L42 oscillator switch timed out");
        break;
    }
} while (response != 0x02);
i2c_write(0x00, 0x10);
i2c_write(0x07, 0x13); // 0x1007    Serial Port SRC Control

i2c_write(0x00, 0x12);
i2c_write(0x03, 0x1f); // 0x1203    FSYNC Pulse Width, Lower Byte

i2c_write(0x00, 0x12);
i2c_write(0x05, 0x3f); // 0x1205    FSYNC Period, Lower Byte

i2c_write(0x00, 0x12);
i2c_write(0x07, 0x34); // 0x1207    ASP Clock Configuration 1

i2c_write(0x00, 0x12);
i2c_write(0x08, 0x1a); // 0x1208    ASP Frame Configuration

i2c_write(0x00, 0x2a);
i2c_write(0x02, 0x02); // 0x2a02    config Channel 1 size to 24 bits per sample.

i2c_write(0x00, 0x2A);
i2c_write(0x05, 0x42); // 0x2a05    Channel 2 Phase and Resolution

i2c_write(0x00, 0x26);
i2c_write(0x01, 0x4C); // 0x2601    SRC Input Sample Rate

i2c_write(0x00, 0x26);
i2c_write(0x09, 0x4C); // 0x2609    SRC Output Sample Rate

i2c_write(0x00, 0x2A);
i2c_write(0x01, 0x0C); // 0x2a01    ASP Receive Enable

i2c_write(0x00, 0x24);
i2c_write(0x0E, 0x01); // 0x240e    Equalizer Input Mute Control

i2c_write(0x00, 0x23);
i2c_write(0x01, 0x00); // 0x2301    Mixer Channel A Input Volume

i2c_write(0x00, 0x23);
i2c_write(0x03, 0x00); // 0x2303    Mixer Channel B Input Volume

i2c_write(0x00, 0x11);
i2c_write(0x01, 0x96); // 0x1101    Power up the codec

nrf_delay_ms(10);      // ex. 5-1 The headphone amplifier is operational after 10 ms.


i2c_write(0x00, 0x11);
i2c_write(0x21, 0x41); // 0x1121    Headset switch control

i2c_write(0x00, 0x1B);
i2c_write(0x74, 0x03); // 0x1b74    Miscellaneous detect control

i2c_write(0x00, 0x11);
i2c_write(0x29, 0x01); // 0x1129    Headset clamp disable

i2c_write(0x00, 0x20);
i2c_write(0x01, 0x0D); // 0x2001    HP Control : mute all

i2c_write(0x00, 0x1f);
i2c_write(0x06, 0x84); // 0x1f06    DAC Control 2

i2c_write(0x00, 0x23);
i2c_write(0x01, 0x00); // 0x2301    Channel A Input Volume = 0

i2c_write(0x00, 0x23);
i2c_write(0x03, 0x00); // 0x2303    Channel B Input Volume = 0 


// enable HP detect
i2c_write(0x00, 0x1b);
i2c_write(0x73, 0xc2); // 0x1b73    Tip Sense Control 2

i2c_write(0x00, 0x1b);
i2c_write(0x75, 0x9f); // 0x1b75    Mic detect control 1

```

#### CS42L42 mute - unmute example

This is an example of how you can mute and unmute CS42L42, for example when playback starts or stops, or when you switch audio between the speaker and headphone out.

```C
i2c_set_address(I2C_ADDR_CS42L42);

if(mute){ //mute
    i2c_write(0x00, 0x20);
    i2c_write(0x01, 0x0D); // 0x2001 HP Control : mute all
}else{ // unmute
    i2c_write(0x00, 0x20);
    i2c_write(0x01, 0x01); // 0x2001 unmute headphones
}
```

#### CS42L42 request headphone status example

You can ask CS42L42 if headphones are plugged in.  
See datasheet p.153: 7.9.8 Detect Status 1

```C
uint8_t response;

i2c_set_address(I2C_ADDR_CS42L42);

i2c_write(0x00, 0x1b);
i2c_read(0x77, &response, 1); // 0x1b77 Detect Status 1

bool headphone_connected = (response >> 7) & 0x01;
```


## Reference

[CS42L42 datasheet](https://statics.cirrus.com/pubs/proDatasheet/CS42L42_DS1083F8.pdf)

# TAS2505

TAS2505 is used as the speaker amplifier and is at I2C address `0x18`.

**Note 1:** The I2S LRCLK is generated by CS42L42, derived from the 3.072 MHz oscillator. Without setting up CS42L42 first and enabling the oscillator, TAS2505 will not produce any sound!

**Note 2:** The reset pin `PIN_TAS_RST` of the TAS2505 is on one of the nRF52840 NFC pins. To make sure the NFC pins are available as GPIO, compile with flag `-DCONFIG_NFCT_PINS_AS_GPIOS`
and add the following to skd_config.h:

```C
#define NFCT_ENABLED 0
#define CONFIG_NFCT_PINS_AS_GPIOS 1
```

## Examples

All the examples follow the [TAS2505 Application Reference Guide](https://www.ti.com/lit/ug/slau472c/slau472c.pdf) in which all registers are documented. 


#### TAS2505 setup example

```C
// Get TAS out of reset
nrf_gpio_cfg_output(PIN_TAS_RST);
nrf_gpio_pin_write(PIN_TAS_RST, 0);
nrf_delay_ms(1); 
nrf_gpio_pin_write(PIN_TAS_RST, 1);
// After RST is released, no register writes should be performed within 1 ms.
nrf_delay_ms(5); 

i2c_set_address(I2C_ADDR_TAS);

i2c_write(0x00, 0x00); 
i2c_write(0x01, 0x01); // 0x00 / 0x01 software reset

i2c_write(0x00, 0x01); 
i2c_write(0x02, 0x04); // 0x01 / 0x02 LDO Control

i2c_write(0x00, 0x00);
i2c_write(0x04, 0x07); 
i2c_write(0x05, 0x94); 
i2c_write(0x06, 0x07); // P0 R6
i2c_write(0x07, 0x00); // P0 R7
i2c_write(0x08, 0x00); // P0 R8

// delay 15 ms for PLL to lock -  see TAS3205 app reference 5.1
nrf_delay_ms(15); 

i2c_write(0x0b, 0x82); // NDAC powered up, NDAC = 2
i2c_write(0x0c, 0x87); // MDAC Powered up, MDAC = 7
i2c_write(0x0d, 0x00); // DAC OSR(9:0)-> DOSR=1024
i2c_write(0x0e, 0x80); // DAC OSR(9:0)-> DOSR=128
i2c_write(0x1b, 0x20); // Word length  = 24bit, BCLK is input

i2c_write(0x1c, 0x00); // Data offset setting  = 0 BLCKs

i2c_write(0x3c, 0x03); // DAC Signal Processing Block PRB_P3

/* 
Todo: setup DAC coefficients
see 5.5 and 5.6 for examples
see 6.1.4 fro DAC programmable coefficients
*/

i2c_write(0x00, 0x01);
i2c_write(0x01, 0x10); // REF, POR and LDO BGAP Control
i2c_write(0x0A, 0x00); // Common Mode Control
i2c_write(0x0C, 0x04); // HP Routing Selection
i2c_write(0x18, 0x80); // AINL Volume Control
i2c_write(0x00, 0x01);
i2c_write(0x2E, 0x7F); // Speaker Volume Control

```

#### TAS2505 mute - unmute example

This is an example of how you can mute and unmute TAS2505, for example when playback starts or stops, or when you switch audio between the speaker and headphone out.

```C

```


## Reference

[TAS2505 datasheet](https://www.ti.com/product/TAS2505)

[TAS2505 Application Reference Guide](https://www.ti.com/lit/ug/slau472c/slau472c.pdf)

[nRF5 SDK v17.0.2 - TWIM driver](https://docs.nordicsemi.com/bundle/sdk_nrf5_v17.0.2/page/group_nrfx_twim.html)



SP-1 uses I2S to transfer audio from the main MCU to the two amplifiers and the Bluetooth module.


These are the pins used for I2S on the nRF52840 (see stemplayer_pins.h)

```C
// I2S
#define PIN_I2S_DOUT    NRF_GPIO_PIN_MAP(1, 9)  // P1.09
#define PIN_I2S_LRCLK   NRF_GPIO_PIN_MAP(0, 11) // P0.11
#define PIN_I2S_SCLK    NRF_GPIO_PIN_MAP(0, 12) // P0.12

#define PIN_I2S_OSC_EN  NRF_GPIO_PIN_MAP(0, 13) // P0.13 - OSC en
```


## I2S configuration

SP-1 uses 24 bit, 48Khz stereo audio over I2S. The I2S peripheral should be configured in slave mode because the master clock comes from the 3.072 MHz oscillator and LRCLK comes from the CS42L42. 

The 3.072 MHz oscillator needs to be enabled by setting `PIN_I2S_OSC_EN` high and CS42L42 has to be configured to generate the LRCLK, see [i2c](i2c#cs42l42).


Example code of the i2s peripheral configuration (for the nRF5 legacy driver, nrfx uses the same defines internally):

```C
  nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
  
  // mode
  config.mode = NRF_I2S_MODE_SLAVE;

  // pins - defined in stemplayer_pins.h
  config.sdin_pin =     NRFX_I2S_PIN_NOT_USED;
  config.sdout_pin =    PIN_I2S_DOUT; 
  config.sck_pin =      PIN_I2S_SCLK;
  config.lrck_pin =     PIN_I2S_LRCLK;

  // clock setup
  config.mck_setup =    NRF_I2S_MCK_DISABLED;
  config.ratio =        NRF_I2S_RATIO_64X;

  // data format
  config.format =       NRF_I2S_FORMAT_I2S;
  config.sample_width = NRF_I2S_SWIDTH_24BIT;
  config.channels =     NRF_I2S_CHANNELS_STEREO;
  config.alignment =    NRF_I2S_ALIGN_LEFT;
```

I have been using buffers of 256 samples (128 frames) for i2s on SP-1.


# Reference

[nRF5 SDK v17.0.2 - i2s nrfx driver](https://docs.nordicsemi.com/bundle/sdk_nrf5_v17.0.2/page/group_nrfx_i2s.html)

[nRF5 SDK v17.0.2 - i2s Legacy Driver](https://docs.nordicsemi.com/bundle/sdk_nrf5_v17.0.2/page/group_nrf_drv_i2s.html)

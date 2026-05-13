
Setup of [BQ24232](https://www.ti.com/product/BQ24232) battery charger.


The BQ24232 can manage battery charging and powering the system while powered from USB or battery. In the stem player, it is configured to allow a max. 500mA current draw from USB.

Pins used for the battery charger:

```C
// BQ24232 battery charger
#define PIN_BQ_ISET     NRF_GPIO_PIN_MAP(1, 00) // P1.00
#define PIN_BQ_nCHG     NRF_GPIO_PIN_MAP(0, 22) // P0.22
#define PIN_BQ_nPGOOD   NRF_GPIO_PIN_MAP(0, 24) // P0.24
#define PIN_BQ_nCE      NRF_GPIO_PIN_MAP(0, 21) // P0.21

#define PIN_BATT_LEVEL  NRF_GPIO_PIN_MAP(0, 28) // P0.28 / AIN4
```

## Interfacing with  BQ24232

### Inputs
- `nCE` is an active low input and enables charging.

### Outputs
- `nPGOOD` is an open drain output, and is low when input voltage (USB) is good.
- `nCHG` is an open drain output, and is low when the battery is charging.
- `ISET` is the current monitoring output. While charging, the voltage at ISET reflects the actual charging current and can be used to monitor charge current. This pin is connected to the MCU via a voltage divider.


## Battery level

The positive voltage of the battery is directly tied to `PIN_BATT_LEVEL` over a voltage divider. 
You can use `NRF_SAADC_INPUT_AIN4` to read the battery voltage with SAADC.

## Reference

[BQ24232 datasheet](https://www.ti.com/product/BQ24232) 
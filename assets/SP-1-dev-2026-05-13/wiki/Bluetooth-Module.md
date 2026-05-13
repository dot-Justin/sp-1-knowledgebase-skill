
The [CYBT-353027-02](https://www.infineon.com/part/CYBT-353027-02) Bluetooth module is connected to the UART and I2S lines. 

Pinout of the UART interface: (see stemplayer_pins.h)

```C
// CYBT-353027-02 BT module
#define PIN_UART_CTS    NRF_GPIO_PIN_MAP(1, 1)  // P1.01
#define PIN_UART_RXD    NRF_GPIO_PIN_MAP(1, 2)  // P1.02 - CYBT RXD
#define PIN_UART_RTS    NRF_GPIO_PIN_MAP(1, 3)  // P1.03 
#define PIN_UART_TXD    NRF_GPIO_PIN_MAP(1, 4)  // P1.04 - CYBT TXD

#define PIN_CY_SPI_CSN  NRF_GPIO_PIN_MAP(1, 5)  // P1.05 
#define PIN_CY_nRST     NRF_GPIO_PIN_MAP(0, 10) // P0.10

```

I haven't worked on the Bluetooth interface yet. More information to come. Help is welcome!

## Reference

[CYBT-353027-02 datasheet](https://www.infineon.com/part/CYBT-353027-02)
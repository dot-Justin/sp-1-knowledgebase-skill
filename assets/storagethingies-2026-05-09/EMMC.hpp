#pragma once

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

namespace hardware {

/**
 * Driver for the Toshiba THGBMNG5D1LBAIL 4GB eMMC Flash.
 * The Stem Player uses a highly custom 1-bit interface.
 */
class EMMC {
public:
    bool init();

    // Power up the eMMC (VCCQ) and take it out of reset
    void power_on();

    // Put the eMMC into sleep/standby mode
    void power_off();

    // Issue a raw CMD to the eMMC
    bool send_command(uint8_t cmd_idx, uint32_t arg, uint32_t* response);

    // Read a block of data using the custom 1-bit SPIM/PWM/PPI hack
    bool read_blocks(uint32_t start_sector, uint32_t num_blocks, uint8_t* buffer);

private:
    // P0.14
    const struct gpio_dt_spec vccq_pin = GPIO_DT_SPEC_GET(DT_PATH(emmc), vccq_gpios);
    // P1.08
    const struct gpio_dt_spec rst_pin = GPIO_DT_SPEC_GET(DT_PATH(emmc), rst_gpios);
};

} // namespace hardware

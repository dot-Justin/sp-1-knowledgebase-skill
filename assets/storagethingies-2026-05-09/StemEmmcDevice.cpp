#define DT_DRV_COMPAT teenageengineering_stem_emmc

#include "storage/emmc/EmmcDriver.hpp"
#include "storage/emmc/StemEmmcDevice.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <new>

LOG_MODULE_REGISTER(stem_emmc_device, LOG_LEVEL_INF);

struct StemEmmcConfig {
    gpio_dt_spec supply_pin;
    gpio_dt_spec reset_pin;
};

struct StemEmmcData {
    alignas(hardware::EmmcDriver) uint8_t storage[sizeof(hardware::EmmcDriver)];
};

hardware::EmmcDriver* get_driver(const struct device* dev) {
    auto* data = static_cast<StemEmmcData*>(dev->data);
    if (data == nullptr) {
        return nullptr;
    }

    return std::launder(reinterpret_cast<hardware::EmmcDriver*>(data->storage));
}

int stem_emmc_device_init(const struct device* dev) {
    auto* data = static_cast<StemEmmcData*>(dev->data);
    const auto* config = static_cast<const StemEmmcConfig*>(dev->config);
    if (data == nullptr || config == nullptr) {
        return -EINVAL;
    }

    auto* driver = new (data->storage) hardware::EmmcDriver();
    driver->configure_control_gpios(config->supply_pin, config->reset_pin);
    LOG_INF("stem-emmc wrapper ready dev=%p data=%p driver=%p",
            dev,
            data,
            driver);
    return 0;
}

bool stem_emmc_card_init_impl(const struct device* dev) {
    const auto* config = static_cast<const StemEmmcConfig*>(dev->config);
    auto* driver = get_driver(dev);
    if (driver == nullptr) {
        LOG_ERR("stem-emmc driver instance missing dev=%p data=%p",
                dev,
                dev != nullptr ? dev->data : nullptr);
        return false;
    }

    const bool supply_ready = (config != nullptr) && gpio_is_ready_dt(&config->supply_pin);
    const bool reset_ready = (config != nullptr) && gpio_is_ready_dt(&config->reset_pin);
    LOG_INF("stem-emmc card init: driver=%p initialized=%d supply_ready=%d reset_ready=%d",
            driver,
            driver->is_initialized() ? 1 : 0,
            supply_ready ? 1 : 0,
            reset_ready ? 1 : 0);

    return driver->is_initialized() || driver->init();
}

bool stem_emmc_read_blocks_sync_impl(const struct device* dev,
    uint32_t block_addr,
    uint8_t* buffer,
    uint32_t num_blocks) {
    auto* driver = get_driver(dev);
    return driver != nullptr && driver->read_blocks_sync(block_addr, buffer, num_blocks);
}

bool stem_emmc_read_blocks_impl(const struct device* dev,
                                uint32_t block_addr,
                                uint8_t* buffer,
                                uint32_t num_blocks) {
    auto* driver = get_driver(dev);
    return driver != nullptr && driver->read_blocks(block_addr, buffer, num_blocks);
}

bool stem_emmc_start_read_blocks_async_impl(const struct device* dev,
                                            uint32_t block_addr,
                                            uint8_t* buffer,
                                            uint32_t num_blocks) {
    auto* driver = get_driver(dev);
    return driver != nullptr &&
           driver->start_read_blocks_async(block_addr, buffer, num_blocks);
}

bool stem_emmc_poll_async_read_complete_impl(const struct device* dev, bool* success) {
    auto* driver = get_driver(dev);
    return driver != nullptr && driver->poll_async_read_complete(success);
}

bool stem_emmc_async_read_in_flight_impl(const struct device* dev) {
    auto* driver = get_driver(dev);
    return driver != nullptr && driver->async_read_in_flight();
}

bool stem_emmc_wait_for_async_read_signal_impl(const struct device* dev, k_timeout_t timeout) {
    auto* driver = get_driver(dev);
    return driver != nullptr && driver->wait_for_async_read_signal(timeout);
}

void stem_emmc_abort_async_read_impl(const struct device* dev) {
    auto* driver = get_driver(dev);
    if (driver != nullptr) {
        driver->abort_async_read();
    }
}

uint32_t stem_emmc_capacity_blocks_impl(const struct device* dev) {
    auto* driver = get_driver(dev);
    return (driver != nullptr) ? driver->capacity_blocks() : 0U;
}

static const struct stem_emmc_api stem_emmc_driver_api = {
    .card_init = stem_emmc_card_init_impl,
    .read_blocks = stem_emmc_read_blocks_impl,
    .start_read_blocks_async = stem_emmc_start_read_blocks_async_impl,
    .poll_async_read_complete = stem_emmc_poll_async_read_complete_impl,
    .async_read_in_flight = stem_emmc_async_read_in_flight_impl,
    .wait_for_async_read_signal = stem_emmc_wait_for_async_read_signal_impl,
    .abort_async_read = stem_emmc_abort_async_read_impl,
    .capacity_blocks = stem_emmc_capacity_blocks_impl,
    .read_blocks_sync = stem_emmc_read_blocks_sync_impl,
};

#define STEM_EMMC_DEFINE(inst)                                                                   \
    static const StemEmmcConfig stem_emmc_config_##inst = {                                      \
        .supply_pin = GPIO_DT_SPEC_INST_GET(inst, supply_gpios),                                 \
        .reset_pin = GPIO_DT_SPEC_INST_GET(inst, reset_gpios),                                   \
    };                                                                                           \
    static StemEmmcData stem_emmc_data_##inst;                                                   \
    DEVICE_DT_INST_DEFINE(inst,                                                                   \
                          stem_emmc_device_init,                                                  \
                          NULL,                                                                   \
                          &stem_emmc_data_##inst,                                                 \
                          &stem_emmc_config_##inst,                                               \
                          POST_KERNEL,                                                            \
                          CONFIG_APPLICATION_INIT_PRIORITY,                                       \
                          &stem_emmc_driver_api);

extern "C" {
DT_INST_FOREACH_STATUS_OKAY(STEM_EMMC_DEFINE)
}

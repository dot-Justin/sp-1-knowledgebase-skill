#pragma once

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <cstdint>

struct stem_emmc_api {
    bool (*card_init)(const struct device* dev);
    bool (*read_blocks)(const struct device* dev,
                        uint32_t block_addr,
                        uint8_t* buffer,
                        uint32_t num_blocks);
    bool (*start_read_blocks_async)(const struct device* dev,
                                    uint32_t block_addr,
                                    uint8_t* buffer,
                                    uint32_t num_blocks);
    bool (*poll_async_read_complete)(const struct device* dev, bool* success);
    bool (*async_read_in_flight)(const struct device* dev);
    bool (*wait_for_async_read_signal)(const struct device* dev, k_timeout_t timeout);
    void (*abort_async_read)(const struct device* dev);
    uint32_t (*capacity_blocks)(const struct device* dev);
    bool (*read_blocks_sync)(const struct device* dev, uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
};

inline const stem_emmc_api* stem_emmc_get_api(const struct device* dev) {
    return (dev == nullptr) ? nullptr : static_cast<const stem_emmc_api*>(dev->api);
}

inline bool stem_emmc_card_init(const struct device* dev) {
    const auto* api = stem_emmc_get_api(dev);
    return api != nullptr && api->card_init != nullptr && api->card_init(dev);
}

inline bool stem_emmc_read_blocks(const struct device* dev,
                                  uint32_t block_addr,
                                  uint8_t* buffer,
                                  uint32_t num_blocks) {
    const auto* api = stem_emmc_get_api(dev);
    return api != nullptr && api->read_blocks != nullptr &&
           api->read_blocks(dev, block_addr, buffer, num_blocks);
}

inline bool stem_emmc_start_read_blocks_async(const struct device* dev,
                                              uint32_t block_addr,
                                              uint8_t* buffer,
                                              uint32_t num_blocks) {
    const auto* api = stem_emmc_get_api(dev);
    return api != nullptr && api->start_read_blocks_async != nullptr &&
           api->start_read_blocks_async(dev, block_addr, buffer, num_blocks);
}

inline bool stem_emmc_poll_async_read_complete(const struct device* dev, bool* success) {
    const auto* api = stem_emmc_get_api(dev);
    return api != nullptr && api->poll_async_read_complete != nullptr &&
           api->poll_async_read_complete(dev, success);
}

inline bool stem_emmc_async_read_in_flight(const struct device* dev) {
    const auto* api = stem_emmc_get_api(dev);
    return api != nullptr && api->async_read_in_flight != nullptr &&
           api->async_read_in_flight(dev);
}

inline bool stem_emmc_wait_for_async_read_signal(const struct device* dev, k_timeout_t timeout) {
    const auto* api = stem_emmc_get_api(dev);
    return api != nullptr && api->wait_for_async_read_signal != nullptr &&
           api->wait_for_async_read_signal(dev, timeout);
}

inline void stem_emmc_abort_async_read(const struct device* dev) {
    const auto* api = stem_emmc_get_api(dev);
    if (api != nullptr && api->abort_async_read != nullptr) {
        api->abort_async_read(dev);
    }
}

inline uint32_t stem_emmc_capacity_blocks(const struct device* dev) {
    const auto* api = stem_emmc_get_api(dev);
    return (api != nullptr && api->capacity_blocks != nullptr) ? api->capacity_blocks(dev) : 0U;
}

inline bool stem_emmc_read_blocks_sync(const struct device* dev,
    uint32_t block_addr,
    uint8_t* buffer,
    uint32_t num_blocks) {
    const auto* api = stem_emmc_get_api(dev);
    return api != nullptr && api->read_blocks_sync != nullptr &&
    api->read_blocks_sync(dev, block_addr, buffer, num_blocks);
}
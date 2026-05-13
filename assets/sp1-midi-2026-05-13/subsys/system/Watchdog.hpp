#pragma once

#include <cstdint>

namespace core::watchdog {

bool is_running();
void start();
int register_channel(const char* name, uint32_t timeout_ms);
void unregister_channel(int channel_id);
void feed();
void feed(int channel_id);

}  // namespace core::watchdog

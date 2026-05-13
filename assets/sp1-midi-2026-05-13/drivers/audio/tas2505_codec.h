#pragma once

#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

void tas2505_codec_log_state(const struct device *dev, const char *tag);

#ifdef __cplusplus
}
#endif

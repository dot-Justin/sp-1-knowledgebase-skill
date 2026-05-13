#pragma once

#include <zephyr/drivers/charger.h>

#ifdef __cplusplus
extern "C" {
#endif

enum bq24232_charger_property {
	BQ24232_CHARGER_PROP_ISET_OVERRIDE = CHARGER_PROP_CUSTOM_BEGIN,
};

#ifdef __cplusplus
}
#endif

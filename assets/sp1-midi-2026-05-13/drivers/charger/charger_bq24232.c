/*
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_bq24232

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/charger.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "charger_bq24232.h"

LOG_MODULE_REGISTER(bq24232_charger, CONFIG_CHARGER_LOG_LEVEL);

struct bq24232_config {
	struct gpio_dt_spec pgood_gpio;
	struct gpio_dt_spec chg_gpio;
	struct gpio_dt_spec ce_gpio;
	struct gpio_dt_spec iset_override_gpio;
};

struct bq24232_data {
	bool charge_enabled;
	bool iset_override_enabled;
};

static int bq24232_set_charge_enable(const struct device *dev, bool enable)
{
	const struct bq24232_config *const config = dev->config;
	struct bq24232_data *const data = dev->data;
	int ret;

	if (enable) {
		ret = gpio_pin_configure_dt(&config->ce_gpio, GPIO_OUTPUT_ACTIVE);
	} else {
		ret = gpio_pin_configure_dt(&config->ce_gpio, GPIO_INPUT | GPIO_PULL_DOWN);
	}
	if (ret < 0) {
		return ret;
	}

	data->charge_enabled = enable;
	return 0;
}

static int bq24232_set_iset_override(const struct device *dev, bool enable)
{
	const struct bq24232_config *const config = dev->config;
	struct bq24232_data *const data = dev->data;
	int ret;

	if (enable) {
		ret = gpio_pin_configure_dt(&config->iset_override_gpio, GPIO_OUTPUT_ACTIVE);
	} else {
		ret = gpio_pin_configure_dt(&config->iset_override_gpio,
					    GPIO_INPUT | GPIO_PULL_DOWN);
	}
	if (ret < 0) {
		return ret;
	}

	data->iset_override_enabled = enable;
	return 0;
}

static int bq24232_read_online(const struct device *dev, enum charger_online *online)
{
	const struct bq24232_config *const config = dev->config;
	const int val = gpio_pin_get_dt(&config->pgood_gpio);

	if (val < 0) {
		return val;
	}

	*online = (val != 0) ? CHARGER_ONLINE_FIXED : CHARGER_ONLINE_OFFLINE;
	return 0;
}

static int bq24232_read_status(const struct device *dev, enum charger_status *status)
{
	const struct bq24232_config *const config = dev->config;
	const struct bq24232_data *const data = dev->data;
	enum charger_online online = CHARGER_ONLINE_OFFLINE;
	int ret = bq24232_read_online(dev, &online);

	if (ret < 0) {
		return ret;
	}

	if (!data->charge_enabled) {
		*status = CHARGER_STATUS_NOT_CHARGING;
		return 0;
	}

	if (online == CHARGER_ONLINE_OFFLINE) {
		*status = CHARGER_STATUS_DISCHARGING;
		return 0;
	}

	ret = gpio_pin_get_dt(&config->chg_gpio);
	if (ret < 0) {
		return ret;
	}

	*status = (ret != 0) ? CHARGER_STATUS_CHARGING : CHARGER_STATUS_NOT_CHARGING;
	return 0;
}

static int bq24232_get_prop(const struct device *dev,
			    const charger_prop_t prop,
			    union charger_propval *val)
{
	const struct bq24232_data *const data = dev->data;

	switch (prop) {
	case CHARGER_PROP_ONLINE:
		return bq24232_read_online(dev, &val->online);
	case CHARGER_PROP_PRESENT:
		val->present = true;
		return 0;
	case CHARGER_PROP_STATUS:
		return bq24232_read_status(dev, &val->status);
	case CHARGER_PROP_CHARGE_TYPE:
		if (!data->charge_enabled) {
			val->charge_type = CHARGER_CHARGE_TYPE_NONE;
		} else if (data->iset_override_enabled) {
			val->charge_type = CHARGER_CHARGE_TYPE_STANDARD;
		} else {
			val->charge_type = CHARGER_CHARGE_TYPE_FAST;
		}
		return 0;
	case CHARGER_PROP_HEALTH:
		val->health = CHARGER_HEALTH_GOOD;
		return 0;
	case BQ24232_CHARGER_PROP_ISET_OVERRIDE:
		val->custom_bool = data->iset_override_enabled;
		return 0;
	default:
		return -ENOTSUP;
	}
}

static int bq24232_set_prop(const struct device *dev,
			    const charger_prop_t prop,
			    const union charger_propval *val)
{
	switch (prop) {
	case BQ24232_CHARGER_PROP_ISET_OVERRIDE:
		return bq24232_set_iset_override(dev, val->custom_bool);
	default:
		return -ENOTSUP;
	}
}

static int bq24232_charge_enable(const struct device *dev, const bool enable)
{
	return bq24232_set_charge_enable(dev, enable);
}

static int bq24232_init(const struct device *dev)
{
	const struct bq24232_config *const config = dev->config;
	int ret;

	if (!gpio_is_ready_dt(&config->pgood_gpio) ||
	    !gpio_is_ready_dt(&config->chg_gpio) ||
	    !gpio_is_ready_dt(&config->ce_gpio) ||
	    !gpio_is_ready_dt(&config->iset_override_gpio)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->pgood_gpio, GPIO_INPUT);
	if (ret < 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&config->chg_gpio, GPIO_INPUT);
	if (ret < 0) {
		return ret;
	}

	ret = bq24232_set_iset_override(dev, false);
	if (ret < 0) {
		return ret;
	}

	return bq24232_set_charge_enable(dev, false);
}

static DEVICE_API(charger, bq24232_driver_api) = {
	.get_property = bq24232_get_prop,
	.set_property = bq24232_set_prop,
	.charge_enable = bq24232_charge_enable,
};

#define BQ24232_DEFINE(inst)                                                                    \
	static const struct bq24232_config bq24232_config_##inst = {                           \
		.pgood_gpio = GPIO_DT_SPEC_INST_GET(inst, pgood_gpios),                        \
		.chg_gpio = GPIO_DT_SPEC_INST_GET(inst, chg_gpios),                            \
		.ce_gpio = GPIO_DT_SPEC_INST_GET(inst, ce_gpios),                              \
		.iset_override_gpio = GPIO_DT_SPEC_INST_GET(inst, iset_override_gpios),        \
	};                                                                                      \
                                                                                            \
	static struct bq24232_data bq24232_data_##inst;                                       \
                                                                                            \
	DEVICE_DT_INST_DEFINE(inst, bq24232_init, NULL, &bq24232_data_##inst,                 \
			      &bq24232_config_##inst, POST_KERNEL, CONFIG_CHARGER_INIT_PRIORITY, \
			      &bq24232_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BQ24232_DEFINE)

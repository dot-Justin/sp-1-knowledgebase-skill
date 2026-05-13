/*
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_tas2505

#include <errno.h>

#include <zephyr/audio/codec.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "tas2505_codec.h"

LOG_MODULE_REGISTER(tas2505_codec, CONFIG_AUDIO_CODEC_LOG_LEVEL);

struct tas_biquad_q23 {
	uint8_t reg;
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;
};

static const struct tas_biquad_q23 stock_speaker_eq_q23[] = {
	{0x0C, 0x7B, 0xEB, 0x19}, {0x10, 0x84, 0x14, 0xE7}, {0x14, 0x7B, 0xEB, 0x19},
	{0x18, 0x7B, 0xDA, 0x72}, {0x1C, 0x88, 0x08, 0x7D}, {0x20, 0x69, 0x43, 0xC5},
	{0x24, 0x9A, 0xB2, 0xD2}, {0x28, 0x62, 0xEA, 0xBF}, {0x2C, 0x65, 0x4D, 0x2E},
	{0x30, 0xB3, 0xD1, 0x7B}, {0x34, 0x5B, 0xD8, 0xC0}, {0x38, 0xB4, 0x2B, 0x3E},
	{0x3C, 0x48, 0x7C, 0x93}, {0x40, 0x6D, 0x32, 0x5F}, {0x44, 0x93, 0x5C, 0x2D},
	{0x48, 0x60, 0x54, 0x42}, {0x4C, 0xB1, 0xFB, 0x66}, {0x50, 0x57, 0x7C, 0x90},
	{0x54, 0x4E, 0x04, 0x9A}, {0x58, 0xC8, 0x2F, 0x2D}, {0x5C, 0x25, 0x68, 0x53},
	{0x60, 0x25, 0x68, 0x53}, {0x64, 0x25, 0x68, 0x53}, {0x68, 0xF1, 0xD3, 0xE7},
	{0x6C, 0x06, 0xB6, 0xE3},
};

struct tas2505_config {
	struct i2c_dt_spec i2c;
	struct gpio_dt_spec reset_gpio;
};

struct tas2505_data {
	uint8_t current_page;
	bool configured;
	bool muted;
	uint8_t volume;
	audio_codec_error_callback_t error_cb;
};

static inline void tas2505_reset_page_cache(struct tas2505_data *data)
{
	data->current_page = 0xFFU;
}

static bool tas2505_reset_pulse(const struct device *dev)
{
	const struct tas2505_config *const cfg = dev->config;

	if (!gpio_is_ready_dt(&cfg->reset_gpio)) {
		return false;
	}

	if (gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_ACTIVE) < 0) {
		return false;
	}
	k_sleep(K_MSEC(2));
	if (gpio_pin_set_dt(&cfg->reset_gpio, 0) < 0) {
		return false;
	}
	k_sleep(K_MSEC(2));
	return true;
}

static void tas2505_delay_ms(int total_ms)
{
	while (total_ms-- > 0) {
		k_sleep(K_MSEC(1));
	}
}

static bool tas2505_select_page(const struct device *dev, uint8_t page)
{
	const struct tas2505_config *const cfg = dev->config;
	struct tas2505_data *const data = dev->data;
	uint8_t buf[2] = {0x00, page};

	if (data->current_page == page) {
		return true;
	}

	if (i2c_write_dt(&cfg->i2c, buf, sizeof(buf)) != 0) {
		LOG_ERR("TAS2505 page select %u failed", page);
		return false;
	}

	k_busy_wait(20);
	data->current_page = page;
	return true;
}

static bool tas2505_write_reg(const struct device *dev, uint8_t page, uint8_t reg, uint8_t val)
{
	const struct tas2505_config *const cfg = dev->config;
	uint8_t buf[2] = {reg, val};

	if (!tas2505_select_page(dev, page)) {
		return false;
	}

	if (i2c_write_dt(&cfg->i2c, buf, sizeof(buf)) != 0) {
		LOG_ERR("TAS2505 write P%u/R%u=0x%02x failed", page, reg, val);
		return false;
	}

	return true;
}

static bool tas2505_read_reg(const struct device *dev, uint8_t page, uint8_t reg, uint8_t *val)
{
	const struct tas2505_config *const cfg = dev->config;

	if (!tas2505_select_page(dev, page)) {
		return false;
	}

	if (i2c_write_read_dt(&cfg->i2c, &reg, sizeof(reg), val, sizeof(*val)) != 0) {
		LOG_ERR("TAS2505 read P%u/R%u failed", page, reg);
		return false;
	}

	return true;
}

static bool tas2505_set_volume_raw(const struct device *dev, uint8_t volume)
{
	struct tas2505_data *const data = dev->data;

	tas2505_reset_page_cache(data);
	return tas2505_write_reg(dev, 1, 46, volume);
}

static bool tas2505_set_mute_raw(const struct device *dev, bool mute)
{
	struct tas2505_data *const data = dev->data;

	tas2505_reset_page_cache(data);
	if (mute) {
		return tas2505_write_reg(dev, 1, 46, 0x7F) &&
		       tas2505_write_reg(dev, 0, 63, 0x30) &&
		       tas2505_write_reg(dev, 1, 45, 0x00);
	}

	return tas2505_write_reg(dev, 1, 46, 0x00) &&
	       tas2505_write_reg(dev, 1, 48, 0x50) &&
	       tas2505_write_reg(dev, 1, 45, 0x02) &&
	       tas2505_write_reg(dev, 0, 63, 0xB0) &&
	       tas2505_write_reg(dev, 0, 65, 0x0A) &&
	       tas2505_write_reg(dev, 0, 64, 0x04);
}

static bool tas2505_run_init_script(const struct device *dev)
{
	struct tas2505_data *const data = dev->data;

	tas2505_reset_page_cache(data);
	bool ok = true;

	ok = ok && tas2505_write_reg(dev, 0, 1, 0x01);
	tas2505_delay_ms(2);

	ok = ok && tas2505_write_reg(dev, 1, 2, 0x04);
	ok = ok && tas2505_write_reg(dev, 0, 4, 0x07);
	ok = ok && tas2505_write_reg(dev, 0, 5, 0x94);
	ok = ok && tas2505_write_reg(dev, 0, 6, 0x07);
	ok = ok && tas2505_write_reg(dev, 0, 7, 0x00);
	ok = ok && tas2505_write_reg(dev, 0, 8, 0x00);
	tas2505_delay_ms(15);

	ok = ok && tas2505_write_reg(dev, 0, 11, 0x82);
	ok = ok && tas2505_write_reg(dev, 0, 12, 0x87);
	ok = ok && tas2505_write_reg(dev, 0, 13, 0x00);
	ok = ok && tas2505_write_reg(dev, 0, 14, 0x80);
	ok = ok && tas2505_write_reg(dev, 0, 27, 0x20);
	ok = ok && tas2505_write_reg(dev, 0, 28, 0x00);
	ok = ok && tas2505_write_reg(dev, 0, 60, 0x03);

	if (ok && !tas2505_select_page(dev, 44)) {
		LOG_ERR("TAS2505 failed to select page 44 for coefficients");
		ok = false;
	}

	for (size_t i = 0; i < ARRAY_SIZE(stock_speaker_eq_q23) && ok; ++i) {
		uint8_t buf[4] = {
			stock_speaker_eq_q23[i].reg,
			stock_speaker_eq_q23[i].b0,
			stock_speaker_eq_q23[i].b1,
			stock_speaker_eq_q23[i].b2,
		};
		const struct tas2505_config *const cfg = dev->config;
		if (i2c_write_dt(&cfg->i2c, buf, sizeof(buf)) != 0) {
			LOG_ERR("TAS2505 coeff write R%u failed", stock_speaker_eq_q23[i].reg);
			ok = false;
		}
	}

	ok = ok && tas2505_write_reg(dev, 1, 1, 0x10);
	ok = ok && tas2505_write_reg(dev, 1, 10, 0x00);
	ok = ok && tas2505_write_reg(dev, 1, 12, 0x04);
	ok = ok && tas2505_write_reg(dev, 1, 24, 0x80);
	ok = ok && tas2505_write_reg(dev, 1, 46, 0x7F);

	return ok;
}

static int tas2505_configure(const struct device *dev, struct audio_codec_cfg *cfg)
{
	struct tas2505_data *const data = dev->data;

	if (cfg->dai_type != AUDIO_DAI_TYPE_I2S) {
		return -ENOTSUP;
	}

	if (cfg->dai_route != AUDIO_ROUTE_PLAYBACK &&
	    cfg->dai_route != AUDIO_ROUTE_PLAYBACK_CAPTURE) {
		return -ENOTSUP;
	}

	if (cfg->dai_cfg.i2s.channels != 2U ||
	    cfg->dai_cfg.i2s.frame_clk_freq != AUDIO_PCM_RATE_48K) {
		return -ENOTSUP;
	}

	LOG_INF("TAS2505 init: configuring speaker amplifier");
	if (!tas2505_reset_pulse(dev)) {
		LOG_ERR("TAS2505 reset pulse failed");
		return -EIO;
	}
	if (!tas2505_run_init_script(dev)) {
		LOG_ERR("TAS2505 init FAILED");
		return -EIO;
	}

	data->configured = true;
	data->muted = true;
	if (!tas2505_set_volume_raw(dev, data->volume)) {
		return -EIO;
	}
	if (!tas2505_set_mute_raw(dev, data->muted)) {
		return -EIO;
	}

	LOG_INF("TAS2505 init complete — speaker muted, ready for unmute");
	return 0;
}

static void tas2505_start_output(const struct device *dev)
{
	struct tas2505_data *const data = dev->data;

	if (!data->configured) {
		return;
	}

	if (tas2505_set_mute_raw(dev, false)) {
		data->muted = false;
	}
}

static void tas2505_stop_output(const struct device *dev)
{
	struct tas2505_data *const data = dev->data;

	if (!data->configured) {
		return;
	}

	if (tas2505_set_mute_raw(dev, true)) {
		data->muted = true;
	}
}

static int tas2505_set_property(const struct device *dev,
				audio_property_t property,
				audio_channel_t channel,
				audio_property_value_t val)
{
	struct tas2505_data *const data = dev->data;

	if (channel != AUDIO_CHANNEL_ALL &&
	    channel != AUDIO_CHANNEL_FRONT_LEFT &&
	    channel != AUDIO_CHANNEL_FRONT_RIGHT) {
		return -ENOTSUP;
	}

	switch (property) {
	case AUDIO_PROPERTY_OUTPUT_MUTE:
		if (!tas2505_set_mute_raw(dev, val.mute)) {
			return -EIO;
		}
		data->muted = val.mute;
		return 0;

	case AUDIO_PROPERTY_OUTPUT_VOLUME:
		if (!tas2505_set_volume_raw(dev, (uint8_t)val.vol)) {
			return -EIO;
		}
		data->volume = (uint8_t)val.vol;
		return 0;

	default:
		return -ENOTSUP;
	}
}

static int tas2505_apply_properties(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static int tas2505_clear_errors(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static int tas2505_register_error_callback(const struct device *dev,
					   audio_codec_error_callback_t cb)
{
	struct tas2505_data *const data = dev->data;
	data->error_cb = cb;
	return 0;
}

static int tas2505_route_input(const struct device *dev, audio_channel_t channel, uint32_t input)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(channel);
	ARG_UNUSED(input);
	return -ENOTSUP;
}

static int tas2505_route_output(const struct device *dev, audio_channel_t channel, uint32_t output)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(channel);
	ARG_UNUSED(output);
	return -ENOTSUP;
}

void tas2505_codec_log_state(const struct device *dev, const char *tag)
{
	struct tas2505_data *const data = dev->data;
	uint8_t p0r25 = 0, p0r26 = 0, p0r27 = 0, p0r28 = 0, p0r44 = 0, p0r46 = 0;
	uint8_t p0r60 = 0, p0r63 = 0, p0r64 = 0, p0r65 = 0, p1r1 = 0, p1r2 = 0;
	uint8_t p1r12 = 0, p1r45 = 0, p1r46 = 0, p1r48 = 0;

	tas2505_reset_page_cache(data);
	const bool ok =
		tas2505_read_reg(dev, 0, 25, &p0r25) &&
		tas2505_read_reg(dev, 0, 26, &p0r26) &&
		tas2505_read_reg(dev, 0, 27, &p0r27) &&
		tas2505_read_reg(dev, 0, 28, &p0r28) &&
		tas2505_read_reg(dev, 0, 44, &p0r44) &&
		tas2505_read_reg(dev, 0, 46, &p0r46) &&
		tas2505_read_reg(dev, 0, 60, &p0r60) &&
		tas2505_read_reg(dev, 0, 63, &p0r63) &&
		tas2505_read_reg(dev, 0, 64, &p0r64) &&
		tas2505_read_reg(dev, 0, 65, &p0r65) &&
		tas2505_read_reg(dev, 1, 1, &p1r1) &&
		tas2505_read_reg(dev, 1, 2, &p1r2) &&
		tas2505_read_reg(dev, 1, 12, &p1r12) &&
		tas2505_read_reg(dev, 1, 45, &p1r45) &&
		tas2505_read_reg(dev, 1, 46, &p1r46) &&
		tas2505_read_reg(dev, 1, 48, &p1r48);

	if (!ok) {
		LOG_WRN("TAS2505 state dump failed [%s]", tag ? tag : "?");
		return;
	}

	LOG_INF("TAS2505[%s]: P0/R25=0x%02x R26=0x%02x R27=0x%02x R28=0x%02x R44=0x%02x R46=0x%02x R60=0x%02x R63=0x%02x R64=0x%02x R65=0x%02x P1/R01=0x%02x R02=0x%02x R12=0x%02x R45=0x%02x R46=0x%02x R48=0x%02x",
		tag ? tag : "?", p0r25, p0r26, p0r27, p0r28, p0r44, p0r46, p0r60, p0r63,
		p0r64, p0r65, p1r1, p1r2, p1r12, p1r45, p1r46, p1r48);
}

static int tas2505_init(const struct device *dev)
{
	const struct tas2505_config *const cfg = dev->config;
	struct tas2505_data *const data = dev->data;

	if (!i2c_is_ready_dt(&cfg->i2c)) {
		return -ENODEV;
	}
	if (!gpio_is_ready_dt(&cfg->reset_gpio)) {
		return -ENODEV;
	}

	data->current_page = 0xFFU;
	data->configured = false;
	data->muted = true;
	data->volume = 0x7FU;
	data->error_cb = NULL;
	return 0;
}

static const struct audio_codec_api tas2505_api = {
	.configure = tas2505_configure,
	.start_output = tas2505_start_output,
	.stop_output = tas2505_stop_output,
	.set_property = tas2505_set_property,
	.apply_properties = tas2505_apply_properties,
	.clear_errors = tas2505_clear_errors,
	.register_error_callback = tas2505_register_error_callback,
	.route_input = tas2505_route_input,
	.route_output = tas2505_route_output,
};

#define TAS2505_INIT(inst)                                                                     \
	static const struct tas2505_config tas2505_config_##inst = {                          \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                            \
		.reset_gpio = GPIO_DT_SPEC_INST_GET(inst, reset_gpios),                       \
	};                                                                                    \
                                                                                              \
	static struct tas2505_data tas2505_data_##inst;                                       \
                                                                                              \
	DEVICE_DT_INST_DEFINE(inst, tas2505_init, NULL, &tas2505_data_##inst,                \
			      &tas2505_config_##inst, POST_KERNEL, 60, &tas2505_api);

DT_INST_FOREACH_STATUS_OKAY(TAS2505_INIT)

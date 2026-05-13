/*
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT cirrus_cs42l42

#include <errno.h>

#include <zephyr/audio/codec.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "cs42l42_codec.h"

LOG_MODULE_REGISTER(cs42l42_codec, CONFIG_AUDIO_CODEC_LOG_LEVEL);

enum {
	CS42L42_REG_DEVID_AB = 0x1001,
	CS42L42_REG_DEVID_CD = 0x1002,
	CS42L42_REG_DEVID_E = 0x1003,
	CS42L42_REG_CLOCK_CTL = 0x1007,
	CS42L42_REG_MCLK_CTL = 0x1009,
	CS42L42_REG_PWR_CTL1 = 0x1101,
	CS42L42_REG_PWR_CTL7 = 0x1107,
	CS42L42_REG_CODEC_CTL = 0x1121,
	CS42L42_REG_MISC_DET_CTL = 0x1B74,
	CS42L42_REG_MIC_DET_CTL1 = 0x1B75,
	CS42L42_REG_HS_CLAMP_DISABLE = 0x1129,
	CS42L42_REG_MCLK_SRC_SEL = 0x1201,
	CS42L42_REG_FSYNC_PW_LOWER = 0x1203,
	CS42L42_REG_FSYNC_PW_UPPER = 0x1204,
	CS42L42_REG_FSYNC_PERIOD_LOWER = 0x1205,
	CS42L42_REG_FSYNC_PERIOD_UPPER = 0x1206,
	CS42L42_REG_ASP_CLK_CFG = 0x1207,
	CS42L42_REG_ASP_FRM_CFG = 0x1208,
	CS42L42_REG_FS_RATE_EN = 0x1209,
	CS42L42_REG_IN_ASRC_CLK = 0x120A,
	CS42L42_REG_OUT_ASRC_CLK = 0x120B,
	CS42L42_REG_PLL_DIV_CFG1 = 0x120C,
	CS42L42_REG_PLL_LOCK_STATUS = 0x130E,
	CS42L42_REG_TSRS_PLUG_STATUS = 0x130F,
	CS42L42_REG_PLL_CTL1 = 0x1501,
	CS42L42_REG_PLL_DIV_FRAC0 = 0x1502,
	CS42L42_REG_PLL_DIV_FRAC1 = 0x1503,
	CS42L42_REG_PLL_DIV_FRAC2 = 0x1504,
	CS42L42_REG_PLL_DIV_INT = 0x1505,
	CS42L42_REG_PLL_CTL3 = 0x1508,
	CS42L42_REG_PLL_CAL_RATIO = 0x150A,
	CS42L42_REG_PLL_CTL4 = 0x151B,
	CS42L42_REG_TIPSENSE_CTL = 0x1B73,
	CS42L42_REG_HS_BIAS_CTL = 0x1C03,
	CS42L42_REG_HP_CTL = 0x2001,
	CS42L42_REG_MIXER_CHA_VOL = 0x2301,
	CS42L42_REG_MIXER_CHB_VOL = 0x2303,
	CS42L42_REG_DAC_CTL = 0x240E,
	CS42L42_REG_SP_RX_CH_SEL = 0x2501,
	CS42L42_REG_HP_VOL_A = 0x2601,
	CS42L42_REG_HP_VOL_B = 0x2609,
	CS42L42_REG_ASP_TX_SIZE_ENABLE = 0x2901,
	CS42L42_REG_ASP_TX_CHANNEL_ENABLE = 0x2902,
	CS42L42_REG_ASP_TX_CHANNEL_AP_RES = 0x2903,
	CS42L42_REG_ASP_RX_ENABLE = 0x2A01,
	CS42L42_REG_ASP_RX_CH1_AP_RES = 0x2A02,
	CS42L42_REG_ASP_RX_CH2_AP_RES = 0x2A05,
	CS42L42_REG_FINAL_HEADPHONE_VOL = 0x1F06,
};

enum {
	CS42L42_EXPECTED_DEVID_AB = 0x42,
	CS42L42_EXPECTED_DEVID_CD = 0xA4,
	CS42L42_MASK_TS_RISE_DBNC = 0x07,
	CS42L42_MASK_TS_FALL_DBNC = 0x38,
	CS42L42_MASK_TS_INV = BIT(7),
	CS42L42_MASK_HS_CLAMP_DISABLE = 0x01,
	CS42L42_MASK_MCLK_DIV = BIT(1),
	CS42L42_MASK_MCLK_SRC_SEL = BIT(0),
	CS42L42_MASK_ASP_HYBRID_MODE = BIT(4),
	CS42L42_MASK_ASP_SCPOL = 0x0C,
	CS42L42_MASK_ASP_LCPOL = 0x03,
	CS42L42_MASK_ASP_STP = BIT(4),
	CS42L42_MASK_ASP_5050 = BIT(3),
	CS42L42_MASK_ASP_FSD = 0x07,
	CS42L42_MASK_FS_EN = 0x0F,
	CS42L42_MASK_INTERNAL_FS = BIT(1),
	CS42L42_MASK_CLK_IASRC_SEL = 0x01,
	CS42L42_MASK_CLK_OASRC_SEL = 0x01,
	CS42L42_MASK_SCLK_PREDIV = 0x03,
	CS42L42_MASK_PLL_MODE = 0x03,
	CS42L42_MASK_TIPSENSE_CTL = 0xC0,
	CS42L42_MASK_TIPSENSE_INV = 0x20,
	CS42L42_MASK_TIPSENSE_DEBOUNCE = 0x03,
	CS42L42_MASK_DETECT_MODE = 0x18,
	CS42L42_MASK_LATCH_TO_VP = 0x80,
	CS42L42_MASK_EVENT_STAT_SEL = 0x40,
	CS42L42_MASK_HS_DET_LEVEL = 0x3F,
	CS42L42_MASK_HS_BIAS_RAMP = 0x03,
	CS42L42_MASK_HP_CTL = 0x0E,
	CS42L42_MASK_MIXER_VOL = 0x3F,
	CS42L42_MASK_SP_RX_CH_B_SELECT = 0x0C,
	CS42L42_MASK_ASP_RX_CHANNEL_ENABLE = 0x3C,
	CS42L42_MASK_ASP_RX_CHANNEL_AP = 0x40,
	CS42L42_MASK_ASP_RX_CHANNEL_RES = 0x03,
	CS42L42_MASK_ASP_TX_ENABLE = 0x01,
	CS42L42_MASK_ASP_TX_CHANNELS = 0x03,
	CS42L42_MASK_ASP_TX_CHANNEL2_AP = BIT(6),
	CS42L42_MASK_ASP_TX_CHANNEL_RES = 0x0F,
	CS42L42_MASK_TS_PLUG_STATUS = 0x0C,
};

enum {
	CS42L42_ASP_CLK_CFG_HYBRID_I2S = 0x1C,
	CS42L42_ASP_FRAME_CFG_I2S = 0x0A,
	CS42L42_FSYNC_PW_LOWER_48K = 0x1F,
	CS42L42_FSYNC_PW_UPPER_48K = 0x00,
	CS42L42_FSYNC_PERIOD_LOWER_48K = 0x3F,
	CS42L42_FSYNC_PERIOD_UPPER_48K = 0x00,
	CS42L42_FS_RATE_ENABLE_96K = 0x03,
	CS42L42_HS_DET_LEVEL_15 = 0x0F,
	CS42L42_HS_BIAS_RAMP_SLOW = 0x02,
	CS42L42_TIPSENSE_CTL_VALUE = 0xC2,
	CS42L42_ASP_RX_CH1_AP_RES_24 = 0x02,
	CS42L42_ASP_RX_CH2_AP_RES_24 = 0x42,
	CS42L42_ASP_RX_ENABLE_STEREO = 0x0C,
	CS42L42_SP_RX_CH_B_SECOND_SLOT = 0x04,
	CS42L42_ASP_TX_ENABLE_VALUE = 0x01,
	CS42L42_ASP_TX_CHANNELS_STEREO = 0x03,
	CS42L42_ASP_TX_CHANNEL_AP_RES_24 = 0x4A,
	CS42L42_PLL_DIV_INT_3072 = 0x3E,
	CS42L42_PLL_DIV_FRAC0_3072 = 0x80,
	CS42L42_PLL_DIV_FRAC1_3072 = 0x00,
	CS42L42_PLL_DIV_FRAC2_3072 = 0x00,
	CS42L42_PLL_DIV_OUT_3072 = 0x10,
	CS42L42_PLL_CAL_RATIO_3072 = 0x7D,
	CS42L42_PLL_MODE_3072 = 0x03,
	CS42L42_MUTED_HP_CTL_VALUE = 0x0E,
	CS42L42_UNMUTED_HP_CTL_VALUE = 0x02,
	CS42L42_TS_PLUG_STATE = 0x03,
	CS42L42_DETECT_OVERRIDE_VALUE = 0x03,
	CS42L42_DETECT_ENABLE_VALUE = 0x8F,
	CS42L42_DETECT_STATUS_LATCH_VALUE = 0xC2,
	CS42L42_FINAL_HP_VOL_DEFAULT = 0x86,
};

struct cs42l42_script_reg_write {
	uint16_t reg;
	uint8_t value;
};

static const struct cs42l42_script_reg_write setup_script_pre_power[] = {
	{CS42L42_REG_MISC_DET_CTL, CS42L42_DETECT_OVERRIDE_VALUE},
};

static const struct cs42l42_script_reg_write setup_script_main[] = {
	{CS42L42_REG_PLL_CTL3, CS42L42_PLL_DIV_OUT_3072},
	{CS42L42_REG_PLL_DIV_FRAC2, CS42L42_PLL_DIV_FRAC2_3072},
	{CS42L42_REG_PLL_DIV_INT, CS42L42_PLL_DIV_INT_3072},
	{CS42L42_REG_PLL_CAL_RATIO, CS42L42_PLL_CAL_RATIO_3072},
	{CS42L42_REG_MCLK_CTL, 0x00},
	{CS42L42_REG_MCLK_SRC_SEL, 0x01},
	{CS42L42_REG_IN_ASRC_CLK, 0x01},
	{CS42L42_REG_OUT_ASRC_CLK, 0x01},
	{CS42L42_REG_PLL_CTL1, 0x01},
};

static const struct cs42l42_script_reg_write setup_script_post_power[] = {
	{CS42L42_REG_PWR_CTL7, 0x01},
};

static const struct cs42l42_script_reg_write setup_script_route[] = {
	{CS42L42_REG_CLOCK_CTL, 0x13},
	{CS42L42_REG_FSYNC_PW_LOWER, 0x1F},
	{CS42L42_REG_FSYNC_PERIOD_LOWER, 0x3F},
	{CS42L42_REG_ASP_CLK_CFG, 0x34},
	{CS42L42_REG_ASP_FRM_CFG, 0x1A},
	{CS42L42_REG_ASP_RX_CH1_AP_RES, 0x02},
	{CS42L42_REG_ASP_RX_CH2_AP_RES, 0x42},
	{CS42L42_REG_HP_VOL_A, 0x4C},
	{CS42L42_REG_HP_VOL_B, 0x4C},
	{CS42L42_REG_ASP_RX_ENABLE, 0x0C},
	{CS42L42_REG_DAC_CTL, 0x01},
	{CS42L42_REG_MIXER_CHA_VOL, 0x00},
	{CS42L42_REG_MIXER_CHB_VOL, 0x00},
	{CS42L42_REG_PWR_CTL1, 0x96},
	{CS42L42_REG_CODEC_CTL, 0x41},
	{CS42L42_REG_MIC_DET_CTL1, CS42L42_DETECT_ENABLE_VALUE},
	{CS42L42_REG_HS_CLAMP_DISABLE, 0x01},
	{CS42L42_REG_HP_CTL, CS42L42_MUTED_HP_CTL_VALUE},
};

static const struct cs42l42_script_reg_write setup_script_final[] = {
	{CS42L42_REG_FINAL_HEADPHONE_VOL, CS42L42_FINAL_HP_VOL_DEFAULT},
	{CS42L42_REG_TIPSENSE_CTL, CS42L42_DETECT_STATUS_LATCH_VALUE},
};

struct cs42l42_config {
	struct i2c_dt_spec i2c;
	struct gpio_dt_spec reset_gpio;
};

struct cs42l42_data {
	uint8_t current_page;
	bool configured;
	bool muted;
	uint8_t volume;
	audio_codec_error_callback_t error_cb;
};

static inline void cs42l42_reset_page_cache(struct cs42l42_data *data)
{
	data->current_page = 0xFFU;
}

static bool cs42l42_reset_pulse(const struct device *dev)
{
	const struct cs42l42_config *const cfg = dev->config;

	if (!gpio_is_ready_dt(&cfg->reset_gpio)) {
		return false;
	}

	if (gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_ACTIVE) < 0) {
		return false;
	}
	k_sleep(K_MSEC(1));
	if (gpio_pin_set_dt(&cfg->reset_gpio, 0) < 0) {
		return false;
	}
	k_sleep(K_MSEC(10));
	return true;
}

static bool cs42l42_select_page(const struct device *dev, uint8_t page)
{
	const struct cs42l42_config *const cfg = dev->config;
	struct cs42l42_data *const data = dev->data;

	if (page == data->current_page) {
		return true;
	}

	const uint8_t buf[2] = {0x00, page};
	if (i2c_write_dt(&cfg->i2c, buf, sizeof(buf)) != 0) {
		LOG_ERR("CS42L42 set page 0x%02x failed", page);
		return false;
	}

	k_busy_wait(20);
	data->current_page = page;
	return true;
}

static bool cs42l42_write_reg(const struct device *dev, uint16_t reg, uint8_t val)
{
	const struct cs42l42_config *const cfg = dev->config;
	const uint8_t page = (uint8_t)(reg >> 8);
	const uint8_t offset = (uint8_t)(reg & 0xFF);
	const uint8_t buf[2] = {offset, val};

	if (!cs42l42_select_page(dev, page)) {
		return false;
	}

	if (i2c_write_dt(&cfg->i2c, buf, sizeof(buf)) != 0) {
		LOG_ERR("CS42L42 write 0x%04x=0x%02x failed", reg, val);
		return false;
	}

	return true;
}

static bool cs42l42_read_reg(const struct device *dev, uint16_t reg, uint8_t *val)
{
	const struct cs42l42_config *const cfg = dev->config;
	const uint8_t page = (uint8_t)(reg >> 8);
	const uint8_t offset = (uint8_t)(reg & 0xFF);

	if (!cs42l42_select_page(dev, page)) {
		return false;
	}

	if (i2c_write_read_dt(&cfg->i2c, &offset, sizeof(offset), val, sizeof(*val)) != 0) {
		LOG_ERR("CS42L42 read 0x%04x failed", reg);
		return false;
	}

	return true;
}

static bool cs42l42_update_reg(const struct device *dev, uint16_t reg, uint8_t mask, uint8_t value)
{
	uint8_t current = 0U;

	if (!cs42l42_read_reg(dev, reg, &current)) {
		return false;
	}

	const uint8_t updated = (uint8_t)((current & ~mask) | (value & mask));
	if (updated == current) {
		return true;
	}

	return cs42l42_write_reg(dev, reg, updated);
}

static void cs42l42_delay_ms(int ms)
{
	for (int i = 0; i < ms; ++i) {
		k_sleep(K_MSEC(1));
	}
}

static bool cs42l42_probe_id(const struct device *dev, const char *tag)
{
	uint8_t ab = 0U;
	uint8_t cd = 0U;
	uint8_t e = 0U;

	bool ok = cs42l42_read_reg(dev, CS42L42_REG_DEVID_AB, &ab);
	if (ok) {
		ok = cs42l42_read_reg(dev, CS42L42_REG_DEVID_CD, &cd);
	}
	if (ok) {
		ok = cs42l42_read_reg(dev, CS42L42_REG_DEVID_E, &e);
	}

	LOG_INF("DEVID[%s]: ok=%d AB=0x%02x CD=0x%02x E=0x%02x",
		tag, ok ? 1 : 0, ab, cd, e);

	if (!ok) {
		return false;
	}

	if (ab != CS42L42_EXPECTED_DEVID_AB || cd != CS42L42_EXPECTED_DEVID_CD) {
		LOG_ERR("Unexpected CS42L42 DEVID: 0x%02x 0x%02x 0x%02x", ab, cd, e);
		return false;
	}

	return true;
}

static bool cs42l42_run_script(const struct device *dev,
			       const struct cs42l42_script_reg_write *script,
			       size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		if (!cs42l42_write_reg(dev, script[i].reg, script[i].value)) {
			return false;
		}
	}

	return true;
}

static bool cs42l42_init_stage(const struct device *dev)
{
	struct cs42l42_data *const data = dev->data;

	cs42l42_reset_page_cache(data);
	if (!cs42l42_probe_id(dev, "init")) {
		return false;
	}

	if (!cs42l42_run_script(dev, setup_script_pre_power, ARRAY_SIZE(setup_script_pre_power))) {
		return false;
	}
	cs42l42_delay_ms(1);

	if (!cs42l42_run_script(dev, setup_script_main, ARRAY_SIZE(setup_script_main))) {
		return false;
	}
	cs42l42_delay_ms(1);

	return cs42l42_run_script(dev, setup_script_post_power, ARRAY_SIZE(setup_script_post_power));
}

static bool cs42l42_start_stream_script(const struct device *dev)
{
	struct cs42l42_data *const data = dev->data;
	uint8_t pll_lock = 0U;

	cs42l42_reset_page_cache(data);
	if (!cs42l42_run_script(dev, setup_script_route, ARRAY_SIZE(setup_script_route))) {
		return false;
	}

	cs42l42_delay_ms(22);
	if (!cs42l42_run_script(dev, setup_script_final, ARRAY_SIZE(setup_script_final))) {
		return false;
	}

	if (!cs42l42_read_reg(dev, CS42L42_REG_PLL_LOCK_STATUS, &pll_lock)) {
		return false;
	}

	LOG_INF("CS42L42 playback clocks active (PLL status=0x%02x)", pll_lock);
	return true;
}

static bool cs42l42_set_volume_raw(const struct device *dev, uint8_t volume)
{
	struct cs42l42_data *const data = dev->data;

	cs42l42_reset_page_cache(data);
	return cs42l42_write_reg(dev, CS42L42_REG_HP_VOL_A, volume) &&
	       cs42l42_write_reg(dev, CS42L42_REG_HP_VOL_B, volume);
}

static bool cs42l42_set_mute_raw(const struct device *dev, bool mute)
{
	struct cs42l42_data *const data = dev->data;

	cs42l42_reset_page_cache(data);
	return cs42l42_write_reg(dev, CS42L42_REG_HP_CTL,
				 mute ? CS42L42_MUTED_HP_CTL_VALUE : CS42L42_UNMUTED_HP_CTL_VALUE);
}

static int cs42l42_configure(const struct device *dev, struct audio_codec_cfg *cfg)
{
	struct cs42l42_data *const data = dev->data;

	if (cfg->dai_type != AUDIO_DAI_TYPE_I2S) {
		return -ENOTSUP;
	}

	if (cfg->dai_route != AUDIO_ROUTE_PLAYBACK &&
	    cfg->dai_route != AUDIO_ROUTE_PLAYBACK_CAPTURE) {
		return -ENOTSUP;
	}

	if (cfg->dai_cfg.i2s.word_size != 24U ||
	    cfg->dai_cfg.i2s.channels != 2U ||
	    cfg->dai_cfg.i2s.frame_clk_freq != AUDIO_PCM_RATE_48K) {
		return -ENOTSUP;
	}

	LOG_INF("CS42L42 configure: stock paged-script bring-up");
	if (!cs42l42_reset_pulse(dev)) {
		return -EIO;
	}
	if (!cs42l42_init_stage(dev)) {
		return -EIO;
	}
	if (!cs42l42_start_stream_script(dev)) {
		return -EIO;
	}
	if (!cs42l42_set_volume_raw(dev, data->volume)) {
		return -EIO;
	}
	if (!cs42l42_set_mute_raw(dev, data->muted)) {
		return -EIO;
	}

	data->configured = true;
	return 0;
}

static void cs42l42_start_output(const struct device *dev)
{
	struct cs42l42_data *const data = dev->data;

	if (!data->configured) {
		return;
	}

	if (cs42l42_set_mute_raw(dev, false)) {
		data->muted = false;
	}
}

static void cs42l42_stop_output(const struct device *dev)
{
	struct cs42l42_data *const data = dev->data;

	if (!data->configured) {
		return;
	}

	if (cs42l42_set_mute_raw(dev, true)) {
		data->muted = true;
	}
}

static int cs42l42_set_property(const struct device *dev,
				audio_property_t property,
				audio_channel_t channel,
				audio_property_value_t val)
{
	struct cs42l42_data *const data = dev->data;

	switch (property) {
	case AUDIO_PROPERTY_OUTPUT_MUTE:
		if (channel != AUDIO_CHANNEL_ALL &&
		    channel != AUDIO_CHANNEL_HEADPHONE_LEFT &&
		    channel != AUDIO_CHANNEL_HEADPHONE_RIGHT) {
			return -ENOTSUP;
		}
		if (!cs42l42_set_mute_raw(dev, val.mute)) {
			return -EIO;
		}
		data->muted = val.mute;
		return 0;

	case AUDIO_PROPERTY_OUTPUT_VOLUME:
		if (channel != AUDIO_CHANNEL_ALL &&
		    channel != AUDIO_CHANNEL_HEADPHONE_LEFT &&
		    channel != AUDIO_CHANNEL_HEADPHONE_RIGHT) {
			return -ENOTSUP;
		}
		if (!cs42l42_set_volume_raw(dev, (uint8_t)val.vol)) {
			return -EIO;
		}
		data->volume = (uint8_t)val.vol;
		return 0;

	default:
		return -ENOTSUP;
	}
}

static int cs42l42_apply_properties(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static int cs42l42_clear_errors(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static int cs42l42_register_error_callback(const struct device *dev,
					   audio_codec_error_callback_t cb)
{
	struct cs42l42_data *const data = dev->data;

	data->error_cb = cb;
	return 0;
}

static int cs42l42_route_input(const struct device *dev, audio_channel_t channel, uint32_t input)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(channel);
	ARG_UNUSED(input);
	return -ENOTSUP;
}

static int cs42l42_route_output(const struct device *dev, audio_channel_t channel, uint32_t output)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(channel);
	ARG_UNUSED(output);
	return -ENOTSUP;
}

bool cs42l42_codec_is_headphone_connected(const struct device *dev)
{
	struct cs42l42_data *const data = dev->data;
	uint8_t value = 0U;

	cs42l42_reset_page_cache(data);
	if (!cs42l42_read_reg(dev, CS42L42_REG_TSRS_PLUG_STATUS, &value)) {
		return false;
	}

	return (uint8_t)((value & CS42L42_MASK_TS_PLUG_STATUS) >> 2) == CS42L42_TS_PLUG_STATE;
}

void cs42l42_codec_log_state(const struct device *dev, const char *tag)
{
	struct cs42l42_data *const data = dev->data;
	uint8_t clock_ctl = 0U;
	uint8_t mclk_ctl = 0U;
	uint8_t mclk_src = 0U;
	uint8_t asp_clk_cfg = 0U;
	uint8_t asp_frm_cfg = 0U;
	uint8_t fs_rate_en = 0U;
	uint8_t pll_ctl1 = 0U;
	uint8_t pll_ctl3 = 0U;
	uint8_t pll_ctl4 = 0U;
	uint8_t pll_lock = 0U;
	uint8_t pwr_ctl1 = 0U;
	uint8_t codec_ctl = 0U;
	uint8_t asp_rx_enable = 0U;
	uint8_t hp_ctl = 0U;

	cs42l42_reset_page_cache(data);
	const bool ok =
		cs42l42_read_reg(dev, CS42L42_REG_CLOCK_CTL, &clock_ctl) &&
		cs42l42_read_reg(dev, CS42L42_REG_MCLK_CTL, &mclk_ctl) &&
		cs42l42_read_reg(dev, CS42L42_REG_MCLK_SRC_SEL, &mclk_src) &&
		cs42l42_read_reg(dev, CS42L42_REG_ASP_CLK_CFG, &asp_clk_cfg) &&
		cs42l42_read_reg(dev, CS42L42_REG_ASP_FRM_CFG, &asp_frm_cfg) &&
		cs42l42_read_reg(dev, CS42L42_REG_FS_RATE_EN, &fs_rate_en) &&
		cs42l42_read_reg(dev, CS42L42_REG_PLL_CTL1, &pll_ctl1) &&
		cs42l42_read_reg(dev, CS42L42_REG_PLL_CTL3, &pll_ctl3) &&
		cs42l42_read_reg(dev, CS42L42_REG_PLL_CTL4, &pll_ctl4) &&
		cs42l42_read_reg(dev, CS42L42_REG_PLL_LOCK_STATUS, &pll_lock) &&
		cs42l42_read_reg(dev, CS42L42_REG_PWR_CTL1, &pwr_ctl1) &&
		cs42l42_read_reg(dev, CS42L42_REG_CODEC_CTL, &codec_ctl) &&
		cs42l42_read_reg(dev, CS42L42_REG_ASP_RX_ENABLE, &asp_rx_enable) &&
		cs42l42_read_reg(dev, CS42L42_REG_HP_CTL, &hp_ctl);

	if (!ok) {
		LOG_WRN("CS42L42 state dump failed [%s]", tag ? tag : "?");
		return;
	}

	LOG_INF("CS42L42[%s]: CLK=0x%02x MCLK=0x%02x SRC=0x%02x ASPCLK=0x%02x ASPFRM=0x%02x FS=0x%02x PLL1=0x%02x PLL3=0x%02x PLL4=0x%02x LOCK=0x%02x PWR1=0x%02x CODEC=0x%02x ASPRX=0x%02x HP=0x%02x",
		tag ? tag : "?",
		clock_ctl, mclk_ctl, mclk_src, asp_clk_cfg, asp_frm_cfg, fs_rate_en,
		pll_ctl1, pll_ctl3, pll_ctl4, pll_lock, pwr_ctl1, codec_ctl, asp_rx_enable,
		hp_ctl);
}

static int cs42l42_init(const struct device *dev)
{
	const struct cs42l42_config *const cfg = dev->config;
	struct cs42l42_data *const data = dev->data;

	if (!i2c_is_ready_dt(&cfg->i2c)) {
		return -ENODEV;
	}
	if (!gpio_is_ready_dt(&cfg->reset_gpio)) {
		return -ENODEV;
	}

	data->current_page = 0xFFU;
	data->configured = false;
	data->muted = true;
	data->volume = 0x4CU;
	data->error_cb = NULL;

	return 0;
}

static const struct audio_codec_api cs42l42_api = {
	.configure = cs42l42_configure,
	.start_output = cs42l42_start_output,
	.stop_output = cs42l42_stop_output,
	.set_property = cs42l42_set_property,
	.apply_properties = cs42l42_apply_properties,
	.clear_errors = cs42l42_clear_errors,
	.register_error_callback = cs42l42_register_error_callback,
	.route_input = cs42l42_route_input,
	.route_output = cs42l42_route_output,
};

#define CS42L42_INIT(inst)                                                                    \
	static const struct cs42l42_config cs42l42_config_##inst = {                         \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                           \
		.reset_gpio = GPIO_DT_SPEC_INST_GET(inst, reset_gpios),                      \
	};                                                                                   \
                                                                                             \
	static struct cs42l42_data cs42l42_data_##inst;                                     \
                                                                                             \
	DEVICE_DT_INST_DEFINE(inst, cs42l42_init, NULL, &cs42l42_data_##inst,               \
			      &cs42l42_config_##inst, POST_KERNEL, 60, &cs42l42_api);

DT_INST_FOREACH_STATUS_OKAY(CS42L42_INIT)

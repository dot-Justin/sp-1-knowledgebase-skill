# Audio Codecs (CS42L42 + TAS2505)

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Driver code current as of `sp1-midi` 2026-05-09.

The SP-1 has two audio chips: **Cirrus Logic CS42L42** drives the TRRS headphone jack (DAC + ADC for headset microphone), and **TI TAS2505** drives the internal mono speaker (amplifier with built-in DSP).

Both chips sit on **I²C0** (SDA P1.07, SCL P1.11) at **400 kHz**. Both share the same I²S0 audio bus from the nRF (BCLK P0.12, LRCK P0.11, SDOUT P1.09).

For the on-disk audio format see `09-audio-format-spec.md`. For the I²S bit-level timing see `02-hardware-overview.md`.

---

## Bus topology

```
        ┌────────────────┐
        │   nRF52840     │
        └─┬─────────────┬┘
          │ I²C0        │ I²S0
   SDA P1.07            │ BCLK P0.12, LRCK P0.11, SDOUT P1.09
   SCL P1.11            │
          │             │
    ┌─────┴───────┐     │
    │             │     │
    ▼             ▼     ▼
┌─────────┐  ┌─────────┐
│ CS42L42 │  │ TAS2505 │  ← I²S input, mono speaker output
│ 0x48    │  │ 0x18    │
│ TRRS    │  └─────────┘
│ jack    │
└─────────┘
```

Both codecs share the I²S bus. The nRF52840 transmits all stem audio (post-mixing, post-effects) on `SDOUT`, and **each codec selects which channels of the I²S stream to consume** via its register-level channel-select configuration. The CS42L42 takes a stereo pair (L+R for headphones) and the TAS2505 takes a mono summation (for the speaker). The CS42L42's ADC (used for headset microphones on the TRRS jack) feeds back over I²C status registers, not over the I²S data wire — `SDIN` is not used.

## I²S parameters

| Parameter | Value |
| --- | --- |
| Sample rate | 48 kHz |
| Bit depth | 24-bit |
| Slot size | 32-bit |
| Channels on wire | 8 (4 stereo pairs, multiplexed by LRCK + frame counter) |
| Master / slave | nRF52840 is master |
| BCLK frequency | 48000 × 64 = 3.072 MHz |
| Reference clock | 3.072 MHz external oscillator on P0.13 |

## Reset lines

Both codecs are held in reset by GPIO at boot, released by firmware after I²C is up and ready. Both are active-low.

| Codec | Reset pin | DTS reference |
| --- | --- | --- |
| CS42L42 | P0.15 | `reset-gpios = <&gpio0 15 GPIO_ACTIVE_LOW>` |
| TAS2505 | P0.09 | `reset-gpios = <&gpio0 9 GPIO_ACTIVE_LOW>` (NFC1 reclaimed as GPIO) |

[code: `sp1-midi/boards/.../stem_player.dts` lines 280, 286]

## CS42L42 — Headphone codec

### Identity

- **I²C address:** 0x48
- **Datasheet:** Cirrus Logic CS42L42 (publicly available from Cirrus)
- **Function in SP-1:** TRRS jack DAC + headset microphone ADC + jack detect + button detect

### Register addressing

The CS42L42 uses **16-bit register addresses** (unlike the more common 7-bit-address model). Writes are: I²C address byte → register MSB → register LSB → data byte. The driver in `sp1-midi/drivers/audio/cs42l42_codec.c` handles this transparently.

### Key registers (from `sp1-midi/drivers/audio/cs42l42_codec.c`)

[code: `sp1-midi/drivers/audio/cs42l42_codec.c`]

#### Identification

| Register | Address | Function |
| --- | --- | --- |
| `DEVID_AB` | `0x1001` | Device ID (A, B nibbles) |
| `DEVID_CD` | `0x1002` | Device ID (C, D nibbles) |
| `DEVID_E` | `0x1003` | Device ID (E nibble) |

Reading these is the first step in driver bring-up — confirms the chip is alive and is the expected part.

#### Clocking / power

| Register | Address | Function |
| --- | --- | --- |
| `CLOCK_CTL` | `0x1007` | Master clock control |
| `MCLK_CTL` | `0x1009` | MCLK source/divider |
| `PWR_CTL1` | `0x1101` | Power control 1 (block enables) |
| `PWR_CTL7` | `0x1107` | Power control 7 |
| `CODEC_CTL` | `0x1121` | Codec master control |
| `MCLK_SRC_SEL` | `0x1201` | Clock source selection |

#### Audio serial port (ASP)

| Register | Address | Function |
| --- | --- | --- |
| `FSYNC_PW_LOWER` | `0x1203` | Frame sync pulse width (low byte) |
| `FSYNC_PW_UPPER` | `0x1204` | Frame sync pulse width (high byte) |
| `FSYNC_PERIOD_LOWER` | `0x1205` | Frame sync period (low byte) |
| `FSYNC_PERIOD_UPPER` | `0x1206` | Frame sync period (high byte) |
| `ASP_CLK_CFG` | `0x1207` | ASP clock configuration |
| `ASP_FRM_CFG` | `0x1208` | ASP frame configuration |
| `FS_RATE_EN` | `0x1209` | Sample rate enable |
| `IN_ASRC_CLK` | `0x120A` | Input asynchronous SRC clock |
| `OUT_ASRC_CLK` | `0x120B` | Output asynchronous SRC clock |
| `PLL_DIV_CFG1` | `0x120C` | PLL divider configuration |
| `SP_RX_CH_SEL` | `0x2501` | Which I²S channels to consume |

#### PLL

| Register | Address | Function |
| --- | --- | --- |
| `PLL_CTL1` | `0x1501` | PLL control 1 |
| `PLL_DIV_FRAC0..2` | `0x1502..0x1504` | PLL fractional divider |
| `PLL_DIV_INT` | `0x1505` | PLL integer divider |
| `PLL_CTL3` | `0x1508` | PLL control 3 |
| `PLL_CAL_RATIO` | `0x150A` | PLL calibration ratio |
| `PLL_CTL4` | `0x151B` | PLL control 4 |
| `PLL_LOCK_STATUS` | `0x130E` | Read this to confirm the PLL is locked before enabling audio |

#### Headphone path

| Register | Address | Function |
| --- | --- | --- |
| `HP_CTL` | `0x2001` | Headphone control |
| `MIXER_CHA_VOL` | `0x2301` | Mixer channel A volume |
| `MIXER_CHB_VOL` | `0x2303` | Mixer channel B volume |
| `HP_VOL_A` | `0x2601` | Headphone amplifier volume A |
| `DAC_CTL` | `0x240E` | DAC control |

#### Jack / headset detection

| Register | Address | Function |
| --- | --- | --- |
| `TSRS_PLUG_STATUS` | `0x130F` | Plug detect status; `cs42l42_codec_is_headphone_connected()` reads this |
| `TIPSENSE_CTL` | `0x1B73` | Tip sense control |
| `MIC_DET_CTL1` | `0x1B75` | Mic detect control |
| `MISC_DET_CTL` | `0x1B74` | Misc detect control |
| `HS_CLAMP_DISABLE` | `0x1129` | Headset clamp disable |
| `HS_BIAS_CTL` | `0x1C03` | Headset bias control |

### Driver API (public surface from `cs42l42_codec.h`)

```c
bool cs42l42_codec_is_headphone_connected(const struct device *dev);
void cs42l42_codec_log_state(const struct device *dev, const char *tag);
```

Only two public functions are surfaced. The rest of the bring-up (PLL config, ASP config, volume) happens inside the driver via the Zephyr `audio_codec` API.

### Bring-up notes

1. Release reset (P0.15 high)
2. Configure clocking and PLL via `PLL_*` and `MCLK_*` registers
3. Wait for PLL lock (read `PLL_LOCK_STATUS = 0x130E`)
4. Configure ASP to match the I²S parameters (24-bit, 48 kHz, master driving BCLK/LRCK)
5. Configure SP_RX_CH_SEL to pick which I²S channels feed the DAC
6. Power up DAC and headphone amp (`PWR_CTL1`, `HP_CTL`, `DAC_CTL`)
7. Enable detection (`TSRS_PLUG_STATUS`, `MIC_DET_CTL1`)

The CS42L42 datasheet has a thorough bring-up sequence; the `sp1-midi` driver implements it.

---

## TAS2505 — Speaker amplifier

### Identity

- **I²C address:** 0x18
- **Datasheet:** TI TAS2505 (publicly available)
- **Function in SP-1:** Mono internal-speaker class-D amplifier with built-in 24-bit DSP

### Register addressing

The TAS2505 uses a **page + register** model. To write a register at page P, register R:

1. Write `0x00 = P` (page-select register on every page)
2. Write `R = value`

The driver caches the current page to avoid redundant page-select writes [code: `sp1-midi/drivers/audio/tas2505_codec.c` — `current_page`, `tas2505_reset_page_cache`, `tas2505_select_page`].

Each page is 128 registers (R = 0–127). Pages used by `sp1-midi`:

- **Page 0:** common configuration (clocks, ADC, status)
- **Page 1:** speaker / amplifier control (including the volume register at P1/R46)
- **Other pages:** DSP coefficient memory

### Volume control

```c
// Single-byte register write to page 1, register 46
tas2505_write_reg(dev, 1, 46, volume);  // 0x00 = max, 0x7F = mute
```

[code: `sp1-midi/drivers/audio/tas2505_codec.c` line 140]

The 7-bit value is a gain step; consult the TAS2505 datasheet for the dB mapping. `0x7F` mutes the amplifier.

### DSP coefficient table

The driver loads a default DSP coefficient table at init. The table appears to be a set of biquad filter coefficients (or similar fixed-point IIR data) targeting specific register addresses on the DSP pages. From the driver source:

```c
// First few entries — register address followed by 24-bit value in 3 bytes
{0x0C, 0x7B, 0xEB, 0x19},
{0x10, 0x84, 0x14, 0xE7},
{0x14, 0x7B, 0xEB, 0x19},
{0x18, 0x7B, 0xDA, 0x72},
{0x1C, 0x88, 0x08, 0x7D},
// ... (continues; see source)
```

[code: `sp1-midi/drivers/audio/tas2505_codec.c` lines 27–35]

Format: `{ register, byte0, byte1, byte2 }` per entry, where the three bytes form a 24-bit DSP coefficient. The TAS2505 datasheet describes the coefficient memory layout; ericlewis or TimK would be the authoritative source for what these specific coefficients implement (likely speaker EQ and possibly a low-frequency limiter to protect the small internal driver).

### Driver API (public surface from `tas2505_codec.h`)

```c
void tas2505_codec_log_state(const struct device *dev, const char *tag);
```

Just diagnostic. The rest of the driver speaks via the Zephyr `audio_codec` interface.

### Bring-up notes

1. Release reset (P0.09 high — remember P0.09 is NFC1 repurposed as GPIO, requires `nfct-pins-as-gpios;` in DTS)
2. Page-select to page 0
3. Configure clocking from BCLK and LRCK (TAS2505 needs the I²S clocks)
4. Page-select to page 1
5. Configure speaker amplifier path, set volume
6. Page-select to the DSP coefficient pages and load the EQ/limiter table
7. Enable the amplifier (mute off)

The chip has an automatic thermal shutdown; in normal operation no firmware action is needed for protection, but reading status registers periodically is a good idea for a robust app.

---

## Why two codecs?

The CS42L42 is optimized for headphone output and TRRS-style microphone input — low power, low distortion, jack detection. The TAS2505 is optimized for class-D speaker amplification with built-in DSP for speaker EQ/protection. Using them together gets the best of both: high-quality headphone output, a properly amplified internal speaker, and headset-button / microphone support.

Both chips listen to the same I²S stream simultaneously. Stock TE firmware presumably mutes the speaker when headphones are detected via the CS42L42 jack-sense; custom firmware needs to replicate this if you want the same UX.

## What the Zephyr `audio_codec` framework does for you

`sp1-midi/prj.conf` enables `CONFIG_AUDIO=y` and `CONFIG_AUDIO_CODEC=y`. The framework provides:

- Unified `audio_codec_configure()` / `audio_codec_start_output()` / `audio_codec_stop_output()` / `audio_codec_set_property()` / `audio_codec_apply_properties()` APIs
- The two drivers implement the device-specific bits
- I²C transport is handled by Zephyr's `i2c_*` APIs which the drivers consume

Application code typically does not need to touch the codecs directly after init — set the output volume via the codec API and stream audio via I²S.

## Where to go next

- For the audio format on the wire and on the eMMC → `09-audio-format-spec.md`
- For the I²S DMA setup and TX backend → `12-audio-engine-internals.md`
- For the DSP effects that run before audio reaches these codecs → `13-dsp-effects.md`
- For the Bluetooth module (not on this bus) → `07-bluetooth-module.md`

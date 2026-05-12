# Working Confirmed — What Is Definitively Working On Real Hardware

This file is the **safe-claims list.** Every item here has a citation showing it's actually working — not theoretical, not "should work," not "in development." If a feature is described in this skill but Claude doesn't see it here, treat it as unconfirmed.

For features still in development, see `known-unknowns.md`. For features that were once claimed and later disproven, see `corrections.md`.

---

## In the stock TE firmware (extracted, analyzed, reproducible)

| Capability | Evidence |
| --- | --- |
| Original device firmware dumped end-to-end | murray, voltage glitch attack on nRF52840 APPROTECT, attempt #8,504, 2025-01-25 [Lines thread summary] |
| Boots and plays original album stems | Documented since device launch; physical units owned by 100+ community members |
| Eight-channel 24-bit/48 kHz audio playback | [code: `audiothingies/StockRuntimeMixer.cpp`] |
| Four-stem mixing with per-stem faders | [code: `audiothingies/StockRuntimeMixer.cpp` — Q12 fixed-point weights] |
| Multi-rate FF (2.5x, 4x, 8x, 16x) and RW (1x, 2.5x, 4x, 8x) | [code: `audiothingies/AudioEngine.hpp` — `kFastForwardRates`, `kRewindRates`] |
| Tape FX (smooth pause/resume ramp) | [code: `audiothingies/AudioEngine.cpp`] |
| LPF (cascaded biquad with LFO) | [code: `audiothingies/effects/BiquadFilter.hpp` + Discord #firmware, ericlewis, 2026-05-09 00:32] |
| Distortion (polynomial waveshaper) | [code: `audiothingies/effects/DistortionNode.hpp` + same Discord post] |
| Gate (linear volume envelope) | Same |
| Echo/Reverb (always-feeding circular delay, per-stem) | Same |
| Custom stems on stock firmware | [Lines #799–805, TimK — confirmed playing user audio without firmware modification] |

---

## In `solderless.engineering` web utility (when online)

| Capability | Evidence |
| --- | --- |
| Browser-based firmware flashing via Web Serial | [Discord #news / multiple Lines posts in #500s] |
| Track 1 + Track 4 + USB-C plug-in → 60-second CDC bootloader window | [Lines #556, B_E_N] |
| Tested with multiple custom firmwares by TimK | [Discord #hardware, tkt1000, 2026-05-08 14:49–14:51] |
| **Status as of 2026-05-09:** offline for an update | [Discord #news, tkt1000, 2026-05-09 12:29] |

---

## In `ericlewis/sp1-midi` BSP (public, GitHub, 2026-05-09)

This is a **MIDI controller**, not a stem player. It does not play audio. But its hardware drivers and BSP infrastructure are confirmed working.

| Capability | Evidence |
| --- | --- |
| Boots on real SP-1 hardware | [code: `ericlewis/sp1-midi/app/main.cpp` + ericlewis Discord 2026-05-09 00:08] |
| USB composite device: MIDI 2.0 (UMP) + CDC ACM serial | [code: `prj.conf` + `app.overlay`] |
| CS42L42 codec driver (I2C address 0x48) | [code: `drivers/audio/cs42l42_codec.c`] |
| TAS2505 amp driver (I2C address 0x18) | [code: `drivers/audio/tas2505_codec.c`] |
| I2S audio bus pins (48 kHz, 24-bit, BCLK P0.12, LRCK P0.11, SDOUT P1.09) | [code: `boards/.../stem_player.dts`] |
| nRF I2S runs as **SLAVE**: BCLK from external 3.072 MHz osc (P0.13), LRCLK from CS42L42 | [TKT wiki: I2S, accessed 2026-05-12] |
| I2S buffer size: 256 samples = 128 frames | [TKT wiki: I2S, accessed 2026-05-12] |
| BQ24232 charger control (GPIO-based) | [code: `drivers/charger/charger_bq24232.c`] |
| eMMC reads at 32 MHz via bit-banged CMD + SPIM3 DAT0 | [code: `subsys/storage/emmc/EmmcDriver.cpp`] (single-data-line mode, eMMC 5.0) |
| PWM LED control (PWM2 for track LEDs P0.29/P0.26/P1.15/P1.14, PWM3 for play LEDs P0.01/P1.12/P0.00/P1.13) | [code: `app/Leds.cpp` + DTS] |
| ADC reading of faders (channels 1–4) and button ladders (channels 5–6) | [code: `app/MidiController.cpp` + DTS `io-channels`] |
| Battery voltage monitoring (AIN4 with divider) | [code: `subsys/power/PowerManager.cpp`] |
| USB CDC ACM console for debugging | enabled with `-DSNIPPET=cdc-acm-console` |
| MIDI 2.0 UMP output: faders → CC 1–4, vol up/down → CC 7 step, track buttons → notes 60–63, transport → MMC | [code: `app/MidiController.cpp`] |
| Watchdog + reset breadcrumbs | [code: `subsys/system/Watchdog.cpp`, `ResetBreadcrumbs.cpp`] |
| Hierarchical state machine app skeleton (Zephyr SMF: IDLE, RUNNING, SHUTTING_DOWN, DEEP_SLEEP) | [code: `app/AppStateMachine.cpp`] |
| Builds cleanly with Zephyr SDK targeting `BOARD=stem_player` | [code: `CMakeLists.txt` + README] |

**Memory footprint (BSP):** ~168 KB FLASH (~18% of 1 MB), ~43 KB RAM (~17% of 256 KB). Plenty of headroom for application code.

---

## In `audiothingies.zip` / `storagethingies.zip` (private — shared 2026-05-09 by ericlewis)

These are header-only C++17 reference implementations of the stock audio + storage stack, **suitable for porting into a Zephyr firmware**. They have not been published as a buildable BSP. Code is authoritative as a description of how the engine works.

| Component | Evidence |
| --- | --- |
| AudioEngine state machine (Play / Slow / FF / RW transports) | [code: `audiothingies/AudioEngine.cpp`] |
| Smoothed speed transitions (one-pole lowpass, coefficient `0.02f` = `0x3CA3D70A`) | [Discord #firmware, ericlewis, 2026-05-09 00:12] |
| Fractional resampling with linear interpolation | Same |
| 24-bit fixed-point phase increment for variable-speed playback | Same |
| Block-skipping for FF (half-sector at 2.5x, quarter-sector at 4x) | [Discord #firmware, ericlewis, 2026-05-09 00:22] |
| StockRuntimeMixer with Q12 fixed-point weights and 9-step cubic output gain curve | [code: `audiothingies/StockRuntimeMixer.cpp`] |
| Frame-accurate transport sync word generation | [code: `audiothingies/AudioEngine.hpp` — `current_sync_word()`] |
| LED state word generation | [code: `audiothingies/AudioEngine.hpp` — `current_track_led_word()`] |
| Solo mask + per-stem fader bypass | [code: `audiothingies/AudioEngine.cpp`] |
| Zephyr I2S TX backend | [code: `audiothingies/backends/zephyr_i2s_tx.cpp`] |
| eMMC SMF init (11 states: POWER_ON → CMD0 → CMD1_POLL → CMD2 → CMD3 → CMD9 → CMD7 → CMD8 → CMD16 → VERIFY → SETUP_ASYNC) | [code: `storagethingies/EmmcDriver.cpp`] |
| Async eMMC reads (interrupt-driven, DMA-style via SPIM3 + PWM for burst timing) | [code: same] |
| DiskManager with 10-slot circular prefetch buffer | [code: `storagethingies/DiskManager.hpp`] |
| Album metadata: up to 16 songs with artist/title strings | [code: `storagethingies/DiskManager.cpp`] |
| Frame → sector mapping with audio slot caching | [code: same] |
| All four DSP effect node implementations (biquad, distortion, chorus/flanger, delay/echo, gate) + StemEffectRack with spinlock-protected effect switching | [code: `audiothingies/effects/`] |
| Right-aligned 24-bit PCM container in `int32_t` (max ±8,388,607) | [code: `audiothingies/PcmPacking.hpp`] |

---

## In `solderless.engineering`-deployable custom firmwares (unreleased as of 2026-05-11)

These have been **demoed on real hardware** but not yet open-sourced. Listed here so Claude doesn't claim they don't exist, but also doesn't claim Claude can read their code.

| Firmware | What works | Source |
| --- | --- | --- |
| **emvee1968's custom firmware** | Basic playback, gate effect, Bluetooth pairing (vol+/vol- combo), custom stems via HT Demucs separation | [Discord #firmware, emvee1968, 2026-05-08 23:24 + 23:54] |
| ↑ same — power-off | **Does not work yet** — disconnect battery to flash | [Discord #firmware, emvee1968, 2026-05-08 23:56] |
| **virtualflannel_46386's custom OS** | FX page (beat repeat, bit crusher, send delay with performance feedback, filter with performance modulation), pitch ±12 cents, mixer with mutes + solo | [Discord #general, virtualflannel_46386, 2026-05-11 17:56; demo: YouTube `Tp_XwUd4rPs`] |
| **TimK's private custom firmware** | "Bunch of different firmwares" tested with the update utility — fully functional | [Discord #hardware, tkt1000, 2026-05-08 14:49] |

---

## In community tools

| Tool | What's confirmed | Source |
| --- | --- | --- |
| **moecal1947's Python USB uploader** | Works but slow (0.75 KB/s, ~4.5 days for 311 MB album); safe + resumable; per-block checksum verify | [Discord #general, moecal1947, 2026-05-09 09:33] |
| **zee_33's LED bit-bang firmware dump** | Wrote a 1-page custom firmware, flashed via solderless, used LED translation to read flash bytes | [Discord #firmware, zee_33, 2026-05-08 17:33 + 19:04] |
| **fishdog_'s eMMC desolder + USB adapter method** | 380–420 °C with 175 °C preheater, no reballing, solder buildup on board + USB-to-eMMC adapter from AliExpress | [Discord #hardware, fishdog_, 2026-05-10 18:55–19:25 / 2026-05-11 19:08] |
| **theunflappable's `test_bootloader.py`** | Python implementation of the bootloader protocol (cannot read/dump firmware) | [Discord #firmware, theunflappable, 2026-05-08 00:49] |
| **dotjustin's `sp-1.dotjust.in` static archive of Lines thread** | Mirror with search, image preservation, agent-friendly indexes | [Discord #general, dotjustin, 2026-05-10 22:37] |
| **Belt clip / case / etc. (3D printed)** | Released on Thingiverse and shared in Discord | [Lines #640–700 + Discord #hardware, walkerauga 2026-05-09 01:00, isacvr67_nolife 2026-05-10 20:31] |

---

## How to use this file

When asked "does X work?":

1. Search this file for X
2. If listed, cite the source
3. If not listed, check `known-unknowns.md` for X
4. If not in either, the honest answer is "I don't know — this isn't documented as working in the material I have access to." Suggest checking the live Discord.

When adding a new entry to this file: it must have a **citation**. No unsourced claims, even from Claude's general knowledge.

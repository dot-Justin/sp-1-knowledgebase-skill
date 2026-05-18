# Working Confirmed — What Is Definitively Working On Real Hardware

This file is the **safe-claims list.** Every item here has a citation showing it's actually working — not theoretical, not "should work," not "in development." If a feature is described in this skill but Claude doesn't see it here, treat it as unconfirmed.

For features still in development, see `known-unknowns.md`. For features that were once claimed and later disproven, see `corrections.md`.

---

## In the stock TE firmware (extracted, analyzed, reproducible)

| Capability | Evidence |
| --- | --- |
| Original device firmware dumped end-to-end | murray, voltage glitch attack on nRF52840 APPROTECT, attempt #8,504, 2025-01-25 [Lines thread summary] |
| Boots and plays original album stems | Documented since device launch; physical units owned by 100+ community members |
| Eight-channel 24-bit/48 kHz audio playback | [code: `assets/audiothingies-2026-05-09/StockRuntimeMixer.cpp`] |
| Four-stem mixing with per-stem faders | [code: `assets/audiothingies-2026-05-09/StockRuntimeMixer.cpp` — Q12 fixed-point weights] |
| Multi-rate FF (2.5x, 4x, 8x, 16x) and RW (1x, 2.5x, 4x, 8x) | [code: `assets/audiothingies-2026-05-09/AudioEngine.hpp` — `kFastForwardRates`, `kRewindRates`] |
| Tape FX (smooth pause/resume ramp) | [code: `assets/audiothingies-2026-05-09/AudioEngine.cpp`] |
| LPF (cascaded biquad with LFO) | [code: `assets/audiothingies-2026-05-09/effects/BiquadFilter.hpp` + Discord #firmware, ericlewis, 2026-05-09 00:32] |
| Distortion (polynomial waveshaper) | [code: `assets/audiothingies-2026-05-09/effects/DistortionNode.hpp` + same Discord post] |
| Gate (linear volume envelope) | Same |
| Echo/Reverb (always-feeding circular delay, per-stem) | Same |
| Custom stems on stock firmware | [Lines #799–805, TimK — confirmed playing user audio without firmware modification] |

---

## In `solderless.engineering` web utility (when online or via local archive)

| Capability | Evidence |
| --- | --- |
| Browser-based firmware flashing via Web Serial | [Discord #news / multiple Lines posts in #500s] |
| Browser-based album upload via Web Serial | [Source: `assets/solderless-2026-05-12/js/storage.js::uploadAlbum`] |
| Track 1 + Track 4 + USB-C plug-in → CDC bootloader window | [Lines #556, B_E_N] + [Source: `index.html` instructions lines 119–125] |
| Tested with multiple custom firmwares by TimK | [Discord #hardware, tkt1000, 2026-05-08 14:49–14:51] |
| **Live site status as of 2026-05-18:** online again, multi-app launcher (stem loader + firmware utility + device info + spoom1) | HTTP 200 from `solderless.engineering` on 2026-05-18; mirror at `assets/solderless-2026-05-18/` |
| **Two local archives available** | `assets/solderless-2026-05-12/` (earlier single-page tool, preserved for citation stability) + `assets/solderless-2026-05-18/` (current canonical multi-app mirror) |

### SP-1 host protocol — byte-level (confirmed from solderless archive 2026-05-13)

| Item | Value / Source |
| --- | --- |
| CDC baud rate | 115200 8N1 [`firmware.js` line 51] |
| Outer frame | COBS-encoded `[0x51 magic, seq, cmd, plen, payload, crc8]` + `0x00` terminator [`protocol.js` lines 91–127] |
| CRC-8 | 256-entry lookup table (initial value 0); table reproduced in `references/15-bootloader-protocol.md` [`protocol.js` lines 64–81] |
| State query | `0x52` 'R' → `0x53` 'S' with **5 raw integer bytes** (each 0-9). Bytes `[1,0,5,1,0]` → string `"10510"` = upload mode; bytes `[0,0,1,0,0]` → `"00100"` = boot mode. **Not ASCII characters** — stringify via `Array.from(bytes).join('')`, never `String.fromCharCode`. [`storage.js` lines 87–125, `gate-fix-testing` protocol.mjs, **empirically verified on Unit A 2026-05-14** as part of the dumper project's HW-02 smoke test] |
| Mode switch (boot → upload) | `0x70 [1]` ('p' + byte 1) → `0x71` 'q'; then `0x50` 'P' (no reply) reboot; reconnect and re-query state [`storage.js` lines 102–117] |
| Firmware flash erase | `0x46` 'F' → `0x47` 'G' [`firmware.js` lines 106–112] |
| Firmware flash write chunk (no reply) | `0x45` 'E' with payload `[chunk_seq (LE32) \| flash_addr (LE32) \| data (≤240 bytes)]` [`firmware.js` lines 128–143] |
| Firmware flash commit | `0x48` 'H' with payload `[last_page (LE32)]` → `0x49` 'I'; then second `0x48` with `[counter (LE32)]` [`firmware.js` lines 150–186] |
| Album upload start | `0x37` '7' (reset write counter) [`storage.js` line 620] |
| Album upload chunk (no reply) | `0x39` '9' with **exact 136-byte payload** = `chunk_counter (LE32) \| emmc_byte_offset (LE32) \| 128 bytes data` [`storage.js` lines 638–647] |
| Album upload end | `0x51` 'Q' (no reply); device reboots/commits [`storage.js` line 712] |
| Device ID | `0x54` 'T' → `0x55` 'U' (8 bytes) [`storage.js` lines 209–215] |
| Album title query | `0x58` 'X' → `0x59` 'Y' (ASCII title) [`storage.js` lines 177–187] |
| Album validity check | `0x66` 'f' → `0x67` 'g' with payload `[isValid (u8), errCode (u8)]` [`storage.js` lines 190–204] |
| Write counter query | `0x43` 'C' → `0x44` 'D' (LE32 counter) [`storage.js` lines 218–224] |
| LED set | `0x5A` 'Z' with 8-byte payload (T1, T2, T3, T4, S1, S2, S3, S4 brightness 0–255) [`stemloader.html` lines 67–75] |
| Fader/battery/ladder/button queries | `0x64`/`0x7A`/`0x74`/`0x5C` — see `references/15-bootloader-protocol.md` |

### Album image format — encoder reference (confirmed from solderless archive 2026-05-13)

| Item | Value / Source |
| --- | --- |
| Sector size | 8192 bytes [`protocol.js` line 20] |
| Frame size | 24 bytes (4 stems × 6) [`protocol.js` line 17] |
| Frames per sector | 340 [`protocol.js` line 21] |
| Block order on disk | `[0, 2, 1, 3]` [`protocol.js` line 22] |
| Block_id formula | `BLOCK_ORDER[frame_in_sector % 4]` [`wav-parser.js::encodeToSP1`] |
| 6-byte stem layout | `L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid` [`wav-parser.js::encodeToSP1` lines 115–120] |
| Per-sector trailer | Bytes 2042..2043 = tempo uint16 LE = `(48000*60)/(24*bpm)`; bytes 2044..2047 = per-stem envelopes (uint8, normalized peak) — at end of block 0 only [`wav-parser.js::encodeToSP1` lines 124–128] |
| Album header magic | `"ALBUM_PRESENT"` (13 ASCII bytes) at offset 0 of metadata sector AND at last 13 bytes of end-marker sector [`storage.js` lines 575, 691–692] |
| Album header layout | offset 13: `albumLength` u32 LE; offset 17: `numSongs` u8; offset 18: title (64B null-term, X-padded); offset 82: songs (N × 136B = u32 offset + u32 length + 64B artist + 64B title) [`storage.js` lines 599–618] |
| Metadata sector fill byte | `0x58` ('X') [`storage.js` line 601] |
| End-marker sector fill byte | `0x00`; magic at last 13 bytes [`storage.js` lines 690–692] |
| Validation limits (album_len < 2 short; > 0x7FFFF long; > 60 songs; > 63 title; > 62 artist/song title; > 999999 song_len) | [`sp-1_protocol.js` lines 27–44 ALBUM_ERR comments] |
| WAV input format required | 8 channel, 24-bit, 48 kHz signed PCM (RIFF/WAVE), audioFormat 1 or 0xFFFE w/ subFormat 1 [`wav-parser.js::wavVerify` lines 34–38] |

### Updates from solderless 2026-05-18 re-snapshot (4-app launcher rewrite)

These rows extend the table above with what the 2026-05-18 re-snapshot newly confirms or changes. Citations point at `assets/solderless-2026-05-18/...`.

| Item | Value / Source |
| --- | --- |
| **Encoder writes 8-byte per-sector trailer** (NEW vs 2026-05-12's 6-byte trailer) | bytes 2040..2041 = clock u16 LE; 2042..2043 = tempo u16 LE; 2044..2047 = per-stem envelopes u8 — at end of block 0 only. Cite: `stemloader/js/wav-converter.js::encodeToSP1` MIDI clock block |
| Empty-sector sentinel values | `clock = 0xFFFF`, `tempo = 0x0000` written when no MIDI tick falls in this sector. Cite: same file, "no clock tick" branch |
| First-tick hardcoded to clock=1 | `if (firstTick) { clock = 1; firstTick = false; }`. Cite: same file |
| Clock-modulo constant | `CLOCK_MAX = 49152` = 512 bars × 96 ticks/bar. `TICKS_PER_BAR = 96` (4 quarters × 24 PPQN). `CLOCK_INCR = 512` (added per tick alongside `samplesPerTick - elapsedSamples`). Cite: `stemloader/js/wav-converter.js` constants block |
| Max album size | `MAX_SECTORS = 0x76000` = 483,328 sectors = 3,959,422,976 B (~3.69 GiB). At 48 kHz / 340 frames-per-sector = ~141.18 sectors/sec → max album ≈ 57 min full-stem audio. Cite: same constants block |
| Encoder accepts 1-N channel WAV (was: required 8) | Missing stems play silent; channels beyond 8 are ignored. Cite: `stemloader/js/wav-converter.js::parseWAV` |
| WAVEFORMATEXTENSIBLE subFormat offset bug fix | The 2026-05-12 parser read `subFormat` at `offset+28` (wrong by the WAV spec); 2026-05-18 reads it at `+32`. Old encoder may have misclassified files emitted by tools using the extensible format. Cite: `stemloader/js/wav-converter.js::wavVerify` |
| `wav-parser.js → wav-converter.js` rename + merge of parser & encoder | Cite: `stemloader/js/wav-converter.js` (was `js/wav-parser.js`) |
| Firmware-flash protocol re-confirmed via the dedicated `utility/` app | Same R/F/E/H byte protocol as documented; new app cleanly separates flash from album upload. Cite: `utility/js/firmware.js::flashFirmware`, `utility/js/protocol.js` (flash constants block) |
| Device-info opcode set exercised in a live-poll app | `T` (device ID), `f` (album verify), `X` (album title), `C` (write counter), `z` (battery), `d` (faders), `\\` (buttons), `t` (ladders) all confirmed working in transfer mode at 10 Hz polling on real hardware. Cite: `deviceinfo/js/deviceinfo.js` |
| WebSerial round-trip latency at usable polling rates | `deviceinfo` polls faders+buttons+ladders every 100 ms (10 Hz); `doom` polls faders+buttons every 60 ms (~16 Hz). Both observed working in browser. Cite: `deviceinfo/js/deviceinfo.js`, `doom/js/doom.js` `pollLoop()` |
| Host-side architecture: iframe + postMessage Web Serial proxy | Parent owns one `navigator.serial` port; iframes use `js/serial-shim.js` to call `navigator.serial` transparently; parent routes RX to active iframe and rejects TX from inactive iframes. Cite: `index.html` lines 130-349, `js/serial-shim.js` |
| Tim Knapen's official user FAQ | First public TE-authored user help document for stem upload. Quotable: "1 minute of audio takes about 10 minutes to transfer"; channel mapping (1L/1R → stem 1, etc.); recovery procedure "long-press function button" after interrupted transfer. Cite: `stemloader/help/help.md` |

---

## In `ericlewis/sp1-midi` BSP (public, GitHub, 2026-05-09)

This is a **MIDI controller**, not a stem player. It does not play audio. But its hardware drivers and BSP infrastructure are confirmed working.

| Capability | Evidence |
| --- | --- |
| Boots on real SP-1 hardware | [code: `assets/sp1-midi-2026-05-13/app/main.cpp` + ericlewis Discord 2026-05-09 00:08] |
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

## In `assets/audiothingies-2026-05-09/` / `assets/storagethingies-2026-05-09/` (bundled — ericlewis Discord shares, 2026-05-09)

These are header-only C++17 reference implementations of the stock audio + storage stack, **suitable for porting into a Zephyr firmware**. They have not been published as a buildable BSP. Code is authoritative as a description of how the engine works.

| Component | Evidence |
| --- | --- |
| AudioEngine state machine (Play / Slow / FF / RW transports) | [code: `assets/audiothingies-2026-05-09/AudioEngine.cpp`] |
| Smoothed speed transitions (one-pole lowpass, coefficient `0.02f` = `0x3CA3D70A`) | [Discord #firmware, ericlewis, 2026-05-09 00:12] |
| Fractional resampling with linear interpolation | Same |
| 24-bit fixed-point phase increment for variable-speed playback | Same |
| Block-skipping for FF (half-sector at 2.5x, quarter-sector at 4x) | [Discord #firmware, ericlewis, 2026-05-09 00:22] |
| StockRuntimeMixer with Q12 fixed-point weights and 9-step cubic output gain curve | [code: `assets/audiothingies-2026-05-09/StockRuntimeMixer.cpp`] |
| Frame-accurate transport sync word generation | [code: `assets/audiothingies-2026-05-09/AudioEngine.hpp` — `current_sync_word()`] |
| LED state word generation | [code: `assets/audiothingies-2026-05-09/AudioEngine.hpp` — `current_track_led_word()`] |
| Solo mask + per-stem fader bypass | [code: `assets/audiothingies-2026-05-09/AudioEngine.cpp`] |
| Zephyr I2S TX backend | [code: `assets/audiothingies-2026-05-09/backends/zephyr_i2s_tx.cpp`] |
| eMMC SMF init (11 states: POWER_ON → CMD0 → CMD1_POLL → CMD2 → CMD3 → CMD9 → CMD7 → CMD8 → CMD16 → VERIFY → SETUP_ASYNC) | [code: `assets/storagethingies-2026-05-09/EmmcDriver.cpp`] |
| Async eMMC reads (interrupt-driven, DMA-style via SPIM3 + PWM for burst timing) | [code: same] |
| DiskManager with 10-slot circular prefetch buffer | [code: `assets/storagethingies-2026-05-09/DiskManager.hpp`] |
| Album metadata: up to 16 songs with artist/title strings | [code: `assets/storagethingies-2026-05-09/DiskManager.cpp`] |
| Frame → sector mapping with audio slot caching | [code: same] |
| All four DSP effect node implementations (biquad, distortion, chorus/flanger, delay/echo, gate) + StemEffectRack with spinlock-protected effect switching | [code: `assets/audiothingies-2026-05-09/effects/`] |
| Right-aligned 24-bit PCM container in `int32_t` (max ±8,388,607) | [code: `assets/audiothingies-2026-05-09/PcmPacking.hpp`] |

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

## Empirically verified on physical SP-1 hardware (2026-05-14, dotjustin)

| Item | Observation | Citation |
| --- | --- | --- |
| Factory bootloader USB IDs | VID `2367`, PID `1701`, manufacturer `teenage engineering`, product `stem player`, serial-per-device. Enumerates as `/dev/ttyACM*` (Linux). | dotjustin Unit A, T1+T4+USB-C combo, lsusb + serialport.list() output |
| Track 1 + Track 4 + USB-C → bootloader CDC enumerates | CDC ACM appears on host within ~200 ms of cable insertion (HW-01 smoke: 195 ms). T1 LED solid while in bootloader. | dotjustin Unit A 2026-05-14, sp-1-emmc-dumper project `HW-01` smoke test |
| `0x52` STATE_QUERY returns 5 raw integer bytes in boot mode | Payload bytes `[0x00, 0x00, 0x01, 0x00, 0x00]` (joined as digits → string `"00100"`). CRC-8 over body verified. ~4 ms round-trip. | dotjustin Unit A 2026-05-14, sp-1-emmc-dumper project `HW-02` smoke test |

These items appeared in earlier sections under "documented" but had no real-hardware citation until this run. They're now load-bearing for the dumper project and any other host code that opens the device.

---

## How to use this file

When asked "does X work?":

1. Search this file for X
2. If listed, cite the source
3. If not listed, check `known-unknowns.md` for X
4. If not in either, the honest answer is "I don't know — this isn't documented as working in the material I have access to." Suggest checking the live Discord.

When adding a new entry to this file: it must have a **citation**. No unsourced claims, even from Claude's general knowledge.

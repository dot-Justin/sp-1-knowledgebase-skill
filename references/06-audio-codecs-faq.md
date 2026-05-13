# FAQ — Audio Codecs

**Q: Why are there two audio chips?**
A: **CS42L42** drives the TRRS headphone jack (DAC + mic ADC + jack detect). **TAS2505** drives the internal mono speaker (class-D amp with DSP). Both listen to the same I²S stream; each picks the channels it needs. See `06-audio-codecs.md`.

**Q: What are the I²C addresses?**
A: CS42L42 = **0x48**, TAS2505 = **0x18**. Both on I²C0 at 400 kHz.

**Q: How does each codec know which I²S channels to use?**
A: The CS42L42 has `SP_RX_CH_SEL = 0x2501` for channel selection. The TAS2505 uses its register pages similarly. The nRF transmits all stem audio on SDOUT; each codec picks its subset.

**Q: What sample format does the I²S stream use?**
A: **48 kHz, 24-bit per sample, 32-bit slot size**, 8 channels (4 stereo pairs multiplexed by LRCK + frame counter). Master mode from nRF.

**Q: Where do the codec reset lines come from?**
A: CS42L42 reset = **P0.15**, TAS2505 reset = **P0.09** (NFC1 repurposed as GPIO). Both active low. [code: `stem_player.dts`]

**Q: How is the CS42L42 register address space organized?**
A: 16-bit register addresses (unusual). Writes: I²C addr → reg MSB → reg LSB → data. See `06-audio-codecs.md` for the key register table (0x1xxx clocking/power, 0x12xx audio serial port, 0x15xx PLL, 0x20xx headphone, 0x23xx mixer, 0x26xx headphone volume, 0x1B7x/0x1C0x detection).

**Q: How is the TAS2505 organized?**
A: **Page-and-register** model. Write 0x00 = page-select on each page, then the register on that page. The driver caches the current page. Volume lives at P1/R46.

**Q: What's the DSP coefficient table in the TAS2505 driver?**
A: A list of `{ register, byte0, byte1, byte2 }` entries (24-bit fixed-point) loaded at init. Almost certainly speaker EQ + bass limiter to protect the small internal driver. Ask TimK / ericlewis for what specifically each coefficient implements.

**Q: How do I check if headphones are plugged in?**
A: Call `cs42l42_codec_is_headphone_connected(dev)` — reads `TSRS_PLUG_STATUS = 0x130F`. [code: `assets/sp1-midi-2026-05-13/drivers/audio/cs42l42_codec.h`]

**Q: How does stock firmware mute the speaker when headphones are inserted?**
A: Not directly documented in public code, but presumably: jack-detect on CS42L42 → callback that mutes TAS2505 (write `0x7F` to page-1 register 46). Custom firmware needs to replicate this UX explicitly.

**Q: What about the headset microphone?**
A: CS42L42 has an internal ADC that captures the TRRS mic. Status / level data is read over I²C, not over I²S SDIN. Mic detection: `MIC_DET_CTL1 = 0x1B75`. Not currently used by `sp1-midi`.

**Q: Where does the 3.072 MHz I²S reference clock come from?**
A: An external oscillator powered/enabled by GPIO P0.13 (held high by a DTS GPIO hog at boot). 48000 × 64 = 3.072 MHz. Both codecs receive BCLK from the nRF, which derives from this oscillator.

**Q: Does the Zephyr `audio_codec` framework handle bring-up for me?**
A: It abstracts the configure/start/stop/property API. The driver does the chip-specific writes. Application code only needs to set volume and feed I²S — see `12-audio-engine-internals.md` for the I²S TX integration.

**Q: Where would I look for full register documentation?**
A: Cirrus's CS42L42 datasheet and TI's TAS2505 datasheet are both publicly available. Both are referenced in their respective driver headers.

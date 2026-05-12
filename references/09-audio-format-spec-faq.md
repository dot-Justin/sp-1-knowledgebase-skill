# FAQ — Audio Format Spec

**Q: What's the audio format?**
A: 4 stems, stereo, **24-bit signed, 48 kHz**. Stored on the eMMC in a custom 6-byte-per-stem interleaved byte layout (not standard little-endian). See `09-audio-format-spec.md`.

**Q: Is it 24-bit little-endian PCM?**
A: **No.** That's a recurring hallucination tkt1000 has called out. The actual byte order **per stem** is: `L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid` — left and right samples are interleaved at the byte level, with MSBs adjacent in bytes 1–2. See `09-audio-format-spec.md` and `corrections.md`.

**Q: How is the in-memory PCM represented?**
A: **24-bit right-aligned in `int32_t`**, with the sign bit at position 23 extending into bits 31:24. Max value `8,388,607` (= 2^23 − 1). See `audiothingies/PcmPacking.hpp` (`float_to_pcm_right24_fast`).

**Q: What sample order are the 4 stems in?**
A: Stems are stored sequentially within a frame: stem 0 (bytes 0–5), stem 1 (bytes 6–11), stem 2 (bytes 12–17), stem 3 (bytes 18–23). By convention stem 0 = drums, 1 = bass, 2 = melody, 3 = vocals — but this is convention, not enforced by the format.

**Q: How do I decode a stem sample?**
A: 
```c
int32_t left  = ((int8_t)data[1] << 16) | (data[0] << 8) | data[3];
int32_t right = ((int8_t)data[2] << 16) | (data[5] << 8) | data[4];
```
where `data` points to the 6 bytes of one stem within a frame.

**Q: How many frames per sector?**
A: **340 frames** per 8192-byte TE sector. 340 × 24 bytes = 8160 audio bytes; the remaining 32 bytes are sync words + LED data + trailer.

**Q: Why isn't the format a standard WAV?**
A: TE chose the layout to support block-skipping for tape FF/RW (see `11-block-interleaving-tape-fx.md`). A standard WAV layout would force the firmware to do real-time time-stretching DSP, exceeding the eMMC's bandwidth at high FF rates.

**Q: How do I produce a custom album?**
A: 1) Stem-separate (HT Demucs etc.), 2) resample to 48 kHz stereo, 3) quantize to 24-bit, 4) write each frame's 24 bytes in the interleaved order, 5) lay out 340 frames per sector in the 0/2/1/3 block order, 6) add side-band data (sync words for timing), 7) upload via the USB protocol. See `21-original-firmware-stems.md` and `09-audio-format-spec.md`.

**Q: What is "dual mono" mode?**
A: When the speaker is the active output (TAS2505), the engine sums L+R per stem into mono. When headphones are detected (CS42L42), application code switches to true stereo. Default in `audiothingies` is dual_mono = true.

**Q: What sample rate is the I²S bus actually running?**
A: 48 kHz LRCK, 3.072 MHz BCLK (= 48000 × 64). Reference clock is the external 3.072 MHz oscillator enabled via P0.13.

**Q: Are L/R interleaved on the I²S wire?**
A: Yes — standard I²S frame: LRCK low = left, LRCK high = right. 32-bit slot, 24-bit MSB-justified audio (lower 8 bits unused). The nRF transmits all 8 channels via TDM-style multiplexing; codecs select their channels via `SP_RX_CH_SEL`.

**Q: Is there compression?**
A: **No.** Raw uncompressed 24-bit/48 kHz PCM. A 30-minute album is ~311 MB.

**Q: What about timing / tempo / song boundaries?**
A: Timing is encoded as MIDI-clock counter values in each TE-block's sync word (4 per sector). Song boundaries are recorded in the album header. The audio data itself is just frames; no embedded timecode beyond the sync words. See `10-midi-timing-encoding.md`.

**Q: Where is the canonical decode function?**
A: `decode_te_frame_payload_i32()` in `storagethingies/DiskManager.hpp`, lines 65–82. This is the single source of truth for byte → sample.

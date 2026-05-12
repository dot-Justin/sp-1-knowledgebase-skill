# FAQ — Original-Firmware Custom Stems

**Q: Can I play my own audio on a stock SP-1 without flashing custom firmware?**
A: **Yes.** TimK confirmed in 2026: custom stems play on stock TE firmware if formatted correctly. The firmware doesn't validate content. See `21-original-firmware-stems.md` and `corrections.md`.

**Q: What's the high-level workflow?**
A: 1) Stem-separate (HT Demucs etc.), 2) resample to 48 kHz 24-bit stereo, 3) frame-pack in TE byte order, 4) sector-pack in 0/2/1/3 block layout, 5) generate sync words for timing, 6) write album header, 7) upload via USB.

**Q: Is there a single tool to do this?**
A: **Not yet publicly.** emvee1968 and virtualflannel each have internal pipelines but neither has released. ericlewis's eventual BSP may include one. For now, you assemble from documented pieces.

**Q: What stem separator should I use?**
A: HT Demucs FT is what emvee1968 uses. Spleeter, Demucs, MusicNN all work. Output 4 stems (drums, bass, melody, vocals) at any sample rate ≥48 kHz.

**Q: What sample rate / bit depth?**
A: **48 kHz, 24-bit, stereo.** Resample with ffmpeg or sox if not already.

**Q: What's the on-disk byte layout per stem?**
A: `L_mid, L_msb, R_msb, L_lsb, R_lsb, R_mid` — 6 bytes per stem, interleaved at the byte level. See `09-audio-format-spec.md`.

**Q: How are frames distributed across blocks?**
A: 0/2/1/3 interleaving — even frames in blocks 0+1 (physical first half), odd frames in blocks 2+3. See `11-block-interleaving-tape-fx.md`.

**Q: What happens if I encode without the 0/2/1/3 layout?**
A: 1.0x playback works fine. FF/RW will break (sounds like discontinuous chunks instead of fast-forward).

**Q: Do I need to generate sync words?**
A: Only if you want **tempo-synced effects** (looping, gate, delay, 2 of LPF presets) to feel musically correct. For basic playback without effect sync, leave them zero.

**Q: How long is a typical album?**
A: ~30 minutes = ~311 MB at 48 kHz/24-bit/4 stems/stereo. moecal1947's reference number [Discord #general, 2026-05-09 09:33].

**Q: How do I upload to the device?**
A: Three options: 1) `solderless.engineering` web tool (offline as of 2026-05-09), 2) moecal1947's slow Python uploader, 3) write your own client against the bootloader protocol.

**Q: How long does upload take?**
A: With moecal1947's current tool: ~4.5 days for 311 MB. Theoretical max ~1-2 minutes with proper bulk-endpoint implementation.

**Q: Will custom stems sound like stock?**
A: Same effects (filter, distortion, gate, echo, tape FX) since stock firmware applies them universally. Effect synchronization needs correct sync words. Quality depends on your source audio and stem-separation quality.

**Q: Does it matter which slot is which stem?**
A: Convention: 0 = drums, 1 = bass, 2 = melody, 3 = vocals. The firmware applies effects per stem; certain effects "feel right" on certain stems. Stick to the convention for best results.

**Q: What about the LED animations?**
A: Each TE-block has 4 LED brightness levels in the side data. Default zero = LEDs sit at firmware-default brightness. For animated LEDs, encode actual envelope levels (e.g., RMS of the audio block).

**Q: What about song metadata (artist / title)?**
A: Up to 16 songs per album, each with 65-byte artist and 65-byte title strings. Stored in the album header. See `21-original-firmware-stems.md` step 7.

**Q: How do I test my encoder?**
A: Compare bytes against a known-good album. If you can extract bytes from a real SP-1's eMMC (via test points or desolder + USB adapter), you can verify your encoder produces format-correct bytes for the same audio.

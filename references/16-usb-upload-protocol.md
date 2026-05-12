# USB Upload Protocol

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Primary source: ericlewis Discord #general 2026-05-09 + moecal1947 Discord #general 2026-05-09 + TimK Wiki `github.com/timknapen/SP-1-dev/wiki/Bootloader`.

This file covers the album upload path specifically — pushing a freshly encoded album image (311 MB) from a host PC into the SP-1's eMMC over USB CDC ACM. It builds on the bootloader protocol (`15-bootloader-protocol.md`); this file is just the album-specific subset, plus the open questions about packet framing.

## Status: working but slow + incomplete public spec

As of synthesis date (2026-05-11):

- **moecal1947** has a working uploader at ~**0.75 KB/s** = ~**4.5 days for a 311 MB album** [Discord #general, 2026-05-09 09:33]
- **ericlewis** says the theoretical max for a proper implementation is **~4 MB/s** (~1-2 minutes for a full album) [Discord #general, 2026-05-09 09:34]
- The **opcode sequence** is documented (below)
- The **byte-level framing and ACK protocol** for the bulk `0x39` packet is **not** publicly documented
- `solderless.engineering` evidently implements this — but as of 2026-05-09 it's offline and its source is closed

## The high-level flow

To upload an album, the host must:

1. Enter the bootloader (Track 1 + Track 4 + USB-C)
2. Switch to "parser 2" mode (the upload mode) via GPREGRET
3. Send each native eMMC block as 4 packets of 128 bytes each (= 512 bytes per native block)
4. Verify write counter
5. Reset and verify the album is valid

The full opcode sequence from ericlewis [Discord #general, 2026-05-09 09:45]:

```
1.  Connect USB CDC — device boots into bootloader parser
2.  0x52 — bootloader version check, confirm connection
3.  0x70 + payload 0x01 — set GPREGRET magic 0x1A96 for parser 2 on next boot
4.  0x50 — trigger reboot
5.  Wait for USB re-enumeration
6.  0x52 — firmware version, confirm parser 2 is active
7.  0x54 — device ID
8.  0x58 — current album title
9.  0x37 — stop playback, reset sector read
10. 0x39 × N — stream stem data as 136-byte packets
                (4B header + 4B sector addr + 128B data, four per 512B sector)
11. 0x43 — read write counter, verify against expected
12. 0x51 — system reset. Device reboots, re-reads album from eMMC,
           comes back in parser 2
13. 0x58 — verify album title matches
14. 0x66 — verify album valid
```

## The bulk packet (0x39) — the bottleneck

Each `0x39` packet is 136 bytes:

```
Byte 0–3:    Header (4 bytes; magic/sub-command/flags/checksum — exact format TBD)
Byte 4–7:    Sector address (32-bit little-endian, presumably; target native eMMC block)
Byte 8–135:  128 bytes of payload data
```

A full album upload requires:

```
311 MB album
  / 512 bytes per native eMMC block
  / 128 bytes per packet
  × 4 packets per native block
  = 2,431,552 packets
```

At a hypothetical USB Full Speed bulk rate (~1.0 MB/s practical), that's ~5 minutes. moecal1947's 4.5-day rate suggests his implementation is using **USB control transfers** (which have ~10x overhead vs bulk endpoints) instead of bulk transfers, or has aggressive flow-control delays between packets.

The right architecture, per moecal1947's question to ericlewis [Discord #general, 2026-05-09 09:36]:

> USB CDC or bulk transport -> RAM buffer -> CMD25 multi-block eMMC write

CDC bulk endpoints on the nRF52840 USB peripheral can sustain hundreds of KB/s. The firmware would buffer incoming packets into RAM, then issue a CMD25 (multi-block write) to the eMMC, writing many native blocks in one transaction.

`storagethingies/EmmcDriver` has no write path implemented (read-only) — see `known-unknowns.md`. So the proper-speed uploader requires writing the CMD24/CMD25 driver code in addition to the USB packet handling.

## What is publicly known about packet framing

Confirmed from ericlewis's Discord post:

- **Each packet is 136 bytes total**
- **First 4 bytes: header** (purpose details not public)
- **Next 4 bytes: sector address** (32-bit; presumably native 512-byte block addressing)
- **Last 128 bytes: data**

Inferred but not confirmed:

- **Endianness:** sector address is presumably little-endian (matches the nRF52840's native order)
- **Sequence:** four packets fill one native eMMC block at consecutive offsets within the block (presumably packets 0–3 write to byte offsets 0, 128, 256, 384 of the same native block)
- **ACK:** probably a single ACK byte per packet (e.g., `0x06` for ACK, something else for NAK); host waits for ACK before sending next packet

**Not publicly known:**

- **Exact ACK byte values**
- **Whether the 4-byte header carries a CRC** that the firmware validates
- **Whether retries are supported** if a packet is NAKed
- **Whether the firmware writes packets to a RAM buffer before issuing the eMMC write**, or writes each 128 bytes immediately (which would explain moecal1947's slow speed if true)
- **What happens if the sector address is invalid** (out of range, or in a protected region)

These are exactly the questions moecal1947 asked ericlewis on 2026-05-09. ericlewis acknowledged he probably has the framing details (*"Prob."* [Discord #general, 2026-05-09 09:49]) but did not share them. See `known-unknowns.md`.

## How to make progress without the full spec

Three paths for someone wanting to build a fast uploader today:

### Path 1: Wait for solderless.engineering to come back online

The web tool implements the protocol. Once it's back, you can either use it as-is or attempt to reverse-engineer its Web Serial JavaScript (depending on how minified the build is).

### Path 2: Ask ericlewis directly

The Discord conversation made it clear ericlewis has the details and is willing to help. moecal1947's specific question went unanswered partly because the conversation moved on; a direct ask in Discord may get the answer.

### Path 3: Black-box reverse-engineer

Connect a USB CDC ACM monitoring tool (Wireshark with usbmon on Linux) between an SP-1 in upload mode and `solderless.engineering` (when it's back online). Capture the bidirectional traffic. The byte-level packet structure will be in the capture.

Of the three, path 1 is the easiest and path 3 is the most labor-intensive.

## moecal1947's existing implementation

For reference, what moecal1947 has working as of 2026-05-09 [Discord #general]:

- Pack WAV stems into an `album.sp1` image (presumably correctly laid out in the 0/2/1/3 interleaving + sync words)
- Flash a custom firmware on the SP-1 that exposes a USB vendor interface (so the host can speak directly to a firmware module that handles block writes)
- Python script sends one 512-byte eMMC block at a time, staged in tiny USB control transfers
- Firmware writes one native eMMC block per command
- Host reads the block back and compares checksum vs source image
- Process is **safe and resumable** (per-block verification + range-based restart)
- Metadata/header block is intentionally skipped unless explicitly requested

Currently:
- ~0.75 KB/s = 4.5 days for 311 MB
- USB control transfers are the bottleneck (control transfer overhead is ~10 ms per request)

To get to ericlewis's 4 MB/s target, the firmware side needs to expose a bulk endpoint and the host needs to stream 128-byte data chunks without per-chunk ACK roundtrips, possibly with a checkpoint every N chunks.

## Album image construction

Before any upload, you need a correctly-formatted album image. This is itself a non-trivial pipeline:

1. **Source audio:** WAV files for each stem (drums, bass, melody, vocals)
2. **Resample** to 48 kHz stereo 24-bit if not already
3. **Frame-pack** in the SP-1's interleaved byte order (see `09-audio-format-spec.md`)
4. **Sector-pack** in the 0/2/1/3 block interleaving (see `11-block-interleaving-tape-fx.md`)
5. **Generate sync words** for each TE-block based on the song's BPM (see `10-midi-timing-encoding.md`)
6. **Generate LED words** (optional — can be all zeros for now; the device just shows fixed brightness)
7. **Write album header** at the start with song offsets and titles
8. **Output** the result as the album image (typically `.sp1` extension)

emvee1968 has this pipeline working for HT Demucs-separated stems but the code isn't released [Discord #firmware, 2026-05-08 23:54].

## Safety considerations

- **Verifying writes:** the `0x43` write counter + `0x66` album-valid check at the end is your protection against partial uploads.
- **Aborts:** moecal1947 reports the per-block resumable approach handles host crashes gracefully. Don't unplug mid-upload without a resumable client.
- **Bricking the device:** uploading a corrupt album won't brick the SP-1 (the bootloader region is untouched), but the device may behave erratically on next boot — symptoms could include playback glitches or refusing to play. Recovery: re-upload a known-good album.
- **Don't overwrite the bootloader region.** The bootloader's `0x39` should only accept sector addresses in the album range. If the firmware doesn't validate this, a bad sector address could brick the bootloader.

## Where to go next

- For the bootloader protocol opcodes (the full set, not just upload) → `15-bootloader-protocol.md`
- For the album header / SongInfo format → `08-emmc-storage.md`
- For the audio frame byte order → `09-audio-format-spec.md`
- For the 0/2/1/3 block layout → `11-block-interleaving-tape-fx.md`
- For the missing packet framing details → `known-unknowns.md`
- For canonical stem prep workflow → `21-original-firmware-stems.md`

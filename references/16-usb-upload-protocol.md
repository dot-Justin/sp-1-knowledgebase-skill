# USB Upload Protocol

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11, and the **solderless source archive** (ingested 2026-05-13). Primary code-of-record: `assets/solderless-2026-05-12/js/storage.js` (main implementation) + `stemloader/js/stemloader.js` (TKT alternate UI; same protocol). See `update-log.md` 2026-05-13.

This file covers the album upload path specifically — pushing a freshly encoded album image (~311 MB for a typical Donda-length album) from a host PC into the SP-1's eMMC over USB CDC ACM. The bootloader command framing, the CRC-8 table, and the COBS encoding live in `references/15-bootloader-protocol.md`; this file is the album-specific subset: which commands, in what order, with which payloads, at what speeds.

## Status: byte-level spec confirmed, throughput ~10 KB/s practical

As of synthesis date (2026-05-13):

- The **opcode sequence is documented** — `0x52` state query, `0x70 [1]` set upload mode, `0x50` reboot, `0x37` reset write counter, `0x39` × N for chunks, `0x51` end-of-upload [Source: `storage.js`].
- The **136-byte chunk payload is documented byte-by-byte** — `chunk_counter (LE32) | emmc_byte_offset (LE32) | 128 bytes data`. See "Chunk payload" below.
- The **practical throughput** of the solderless implementation is **~10 KB/s** at 115200 baud with no per-chunk handshake. A 311 MB album takes ~9 hours.
- **moecal1947's Python tool** achieves ~0.75 KB/s = ~4.5 days per 311 MB album [Discord #general, 2026-05-09 09:33] — bottlenecked by USB control-transfer overhead vs. CDC bulk semantics.
- **ericlewis's "theoretical max ~4 MB/s"** [Discord #general, 2026-05-09 09:34] assumes a hypothetical USB MSC / bulk-endpoint implementation with eMMC CMD25 multi-block writes — that has not been built publicly.
- `solderless.engineering` was offline as of 2026-05-09; **a 2026-05-12 source snapshot is archived locally** in `assets/solderless-2026-05-12/`.

## The high-level flow

To upload an album, the host must:

1. Enter the bootloader (Track 1 + Track 4 + USB-C)
2. Query state with `0x52`; if `"00100"` (boot mode), switch to upload mode with `0x70 [1]` + `0x50` reboot + reconnect
3. Query state again; confirm `"10510"` (upload mode)
4. Send `0x37` to reset the write counter
5. Stream the album image as `0x39` chunks of 128 bytes each, with a running byte offset
6. Send `0x51` to signal end-of-upload (device reboots and commits)
7. Optionally reconnect, query title (`0x58`) and verify (`0x66`) the new album

[Source: `storage.js::uploadAlbum` lines 573–716.]

## Chunk payload (the `0x39` packet)

Each `0x39` command carries a fixed **136-byte payload**:

```
Byte 0..3:     chunk_counter      (uint32 LE) — monotonic; starts at 0
Byte 4..7:     emmc_byte_offset   (uint32 LE) — absolute byte offset within eMMC
Byte 8..135:   data               (128 bytes) — chunk of the album image
```

The outer wire format (COBS-wrapped `0x51 magic | seq | cmd=0x39 | plen=136 | payload | crc8`) is described in `references/15-bootloader-protocol.md`.

**Implications:**

- The host always sends 128 data bytes per chunk. Pad the album image to a multiple of 128 bytes if needed (in practice the encoder produces 8192-byte sectors so this is automatic).
- `emmc_byte_offset` is a **byte offset**, not a sector address. The host increments it by 128 per chunk. The firmware translates byte offsets to underlying eMMC block writes internally.
- `chunk_counter` is **also incremented by the host per chunk**, but it's the host's own counter, not derived from `emmc_byte_offset`. The firmware uses it as a transmission tracker, exposed via `0x43`.
- Two different counters: `chunk_counter` (in payload) increments per chunk, while the outer-frame `seq` field (in the packet header) is set to the same value in solderless (also incremented per chunk). The firmware probably uses one or the other for sequence checking; solderless keeps them in sync.

### Fire-and-forget: no per-chunk ACK

The host **does not wait for any response** after each `0x39` packet. Solderless uses `sendNoReply` for the bulk-data path [`storage.js` line 644]. Throughput is limited by:

- Raw serial speed (115200 baud = ~11,520 bytes/sec sustained)
- Packet overhead (~5–7% for COBS + header bytes)
- The device's ability to process incoming chunks at line rate

In practice solderless achieves ~10 KB/s of payload data, which means about **80 chunks/sec at 128 bytes each**. A 311 MB album = 2,540,032 chunks ≈ 8.8 hours.

**If a chunk is dropped on the wire**, the upload silently corrupts. There is no retry, no NAK, no end-to-end checksum across the album. The host's only recourse is to verify the chunk counter at the end with `0x43` (matches expected total) and the album magic with `0x66`. If either check fails, the only option is to start over.

For a faster uploader, the architecture moecal1947 and ericlewis discussed [Discord #general, 2026-05-09 09:36–09:49] would replace this with USB bulk transfers + RAM buffering + CMD25 multi-block eMMC writes — but that requires a custom firmware-side implementation, not just a faster client.

## Album upload sequence (concrete)

[Source: `storage.js::uploadAlbum` lines 573–716. Variable names below mirror the JS.]

```
const WRITE_CMD       = 0x39
const CHUNK_SIZE      = 128
const CHUNKS_PER_SECTOR = 64       // 8192 bytes / 128 bytes
const PAYLOAD_SIZE    = 136        // chunk_counter (4) + emmc_offset (4) + data (128)

// (assumes device is already in upload mode after the state-query / mode-switch dance)

// 1. Reset write counter / start upload session.
sendCmd(0x37, [], seq=1, timeout=3000)   // response is ignored; some firmwares may not reply

// 2. Metadata sector (8192 bytes, see references/21-original-firmware-stems.md for layout)
let emmcOffset   = 0
let chunkCounter = 0

for (offset = 0; offset < 8192; offset += 128):
  payload = new Uint8Array(136)
  payload[0..4]   = LE32(chunkCounter)
  payload[4..8]   = LE32(emmcOffset)
  payload[8..136] = metaBytes.subarray(offset, offset + 128)
  sendNoReply(0x39, payload, seq=chunkCounter)
  emmcOffset   += 128
  chunkCounter += 1

// 3. For each song, encode to SP-1 audio bytes (see encodeToSP1 in references/21-original-firmware-stems.md):
for each song:
  sp1Data = encodeToSP1(channels, totalFrames, song.bpm)
  for (offset = 0; offset < sp1Data.length; offset += 128):
    payload = new Uint8Array(136)
    payload[0..4]   = LE32(chunkCounter)
    payload[4..8]   = LE32(emmcOffset)
    payload[8..136] = sp1Data.subarray(offset, offset + 128)
    sendNoReply(0x39, payload, seq=chunkCounter)
    emmcOffset   += 128
    chunkCounter += 1

// 4. End-marker sector — 8192 bytes of 0x00 with magic at last 13 bytes
endBuf = new Uint8Array(8192)         // filled with 0x00
endBuf.set("ALBUM_PRESENT", 8192-13)  // magic at bytes 8179..8191
for (offset = 0; offset < 8192; offset += 128):
  payload = new Uint8Array(136)
  payload[0..4]   = LE32(chunkCounter)
  payload[4..8]   = LE32(emmcOffset)
  payload[8..136] = endBuf.subarray(offset, offset + 128)
  sendNoReply(0x39, payload, seq=chunkCounter)
  emmcOffset   += 128
  chunkCounter += 1

// 5. End-of-upload signal — device commits and reboots
sendNoReply(0x51, [], seq=1)
```

### Total chunk count for an album with N songs of S_i sectors each

```
chunks_total = 64                                      // metadata sector
             + sum(S_i) × 64 for each song             // audio
             + 64                                      // end marker
```

If the album header is filled out with `albumLength = sum(S_i) + 1` (the +1 is the end marker, per the album encoder), the total chunks = `albumLength × 64`.

## Why 128-byte data + 8-byte header = 136?

The 136-byte payload fits cleanly into the COBS-wrapped CRC-framed packet that the firmware's USB CDC parser handles efficiently. Adding the outer framing: `0x51 magic (1) + seq (1) + cmd (1) + plen (1) + 136 + crc8 (1) = 141 bytes` of packet body, plus COBS overhead (~1 byte per 254-byte run) + 1 zero terminator ≈ 143 bytes on the wire.

The 128-byte data chunk size is also half of a typical USB CDC bulk packet (USB Full Speed has 64-byte bulk endpoints; nRF52840 supports up to 64-byte single-packet bulk). The host can pipeline two consecutive packets without exceeding the device-side packet buffer.

## Why `emmc_byte_offset` instead of a sector address

Two reasons, inferred:

1. **Resumability** — if an upload is interrupted and the host knows the last successfully-sent byte offset, restart is trivial: just send chunks from that offset. With a sector-based addressing, restart would require sector + intra-sector position tracking.
2. **Firmware abstraction** — the eMMC firmware layer presents a byte-addressable interface to the upload command, hiding native block boundaries. This decouples the protocol from the underlying eMMC's 512-byte block size and from the 8192-byte TE-sector concept.

(The eMMC itself only supports block writes, so the firmware presumably buffers 128-byte chunks until it has accumulated a complete 512-byte native block, then issues a CMD24 single-block write. This is the slow path that bottlenecks the upload to ~10 KB/s.)

## How to make a faster uploader

The bottleneck is the per-chunk eMMC write. For ericlewis's "4 MB/s" target, the firmware would need to:

1. Accept bulk USB transfers (no per-packet CDC framing overhead).
2. Buffer 64+ KB of incoming data in RAM.
3. Issue a CMD25 multi-block eMMC write to flush the buffer in one transaction.

Per moecal1947's design discussion [Discord #general, 2026-05-09 09:36]:

> USB CDC or bulk transport -> RAM buffer -> CMD25 multi-block eMMC write

`storagethingies/EmmcDriver` has read paths only — no write paths implemented. So building this faster uploader requires:

1. Implementing CMD24 (single-block) + CMD25 (multi-block) + CMD23 (block-count preset) on the eMMC driver side.
2. Replacing the per-chunk `0x39` handler with a bulk-endpoint USB MSC class implementation (or a custom vendor class).
3. Updating the host to stream a continuous byte stream over the bulk endpoint rather than discrete COBS packets.

None of this exists publicly as of synthesis date. The solderless implementation is the **fastest public option** at ~10 KB/s.

## moecal1947's existing slow implementation

For context [Discord #general, moecal1947, 2026-05-09 09:33]:

- Uses USB **control transfers** (not bulk), which carry ~10× the per-transfer overhead.
- Custom firmware on the SP-1 exposes a vendor interface; the Python host pushes one 512-byte block per control transfer.
- Per-block read-back and checksum verification (so writes are safe and resumable).
- Achieves ~0.75 KB/s = 4.5 days per 311 MB album.

This is significantly slower than what solderless can do (because solderless's `0x39` is a CDC bulk fire-and-forget at 115200 with no checkpoint roundtrips). But moecal's tool is *safe* in a way solderless isn't — solderless can silently corrupt on a dropped chunk; moecal's tool detects and recovers.

## Common errors when building your own uploader

- **Forgetting to send `0x37` first** — the firmware's write-counter starts wherever it was last; `0x39` chunks with `chunk_counter=0` would mismatch.
- **Wrong byte order in payload header** — `chunk_counter` and `emmc_byte_offset` are both **little-endian uint32**, matching nRF52840 native byte order. `setUint32(offset, value, true)` in JS.
- **Padding mismatches** — the album image must be a multiple of 128 bytes. The encoder ensures this (8192-byte sectors are multiples of 128).
- **Skipping the end-marker sector** — the firmware validates that `ALBUM_PRESENT` magic appears at the **end** of the album image (`ALBUM_ERR.MAGIC_NOT_FOUND_AT_END`). Without the trailing magic, `0x66` will fail.
- **Sending `0x39` without first transitioning to upload mode** — the bootloader's boot-mode parser doesn't accept `0x39`. State query must return `"10510"` before any `0x39`.
- **Not waiting for re-enumeration after `0x50`** — the device disconnects USB after `0x50`; reconnect with `requestPort()` after a 3+ second wait.

## Safety considerations

- **Verifying writes:** `0x43` returns the chunk counter the device received. Compare against the expected total before sending `0x51`. If mismatch, abort and restart.
- **Aborts:** unplug mid-upload → album is corrupt; restart from scratch. The solderless protocol is not resumable; moecal's is.
- **Bricking:** the bootloader and app firmware are unaffected by album writes (the bootloader's `0x39` only touches the eMMC). A corrupt album won't brick the SP-1.
- **Don't overwrite the bootloader's flash:** `0x39` writes to eMMC, not flash. There is no path from `0x39` to corrupt the bootloader.

## Where to go next

- For the bootloader protocol framing and all opcodes → `references/15-bootloader-protocol.md`
- For the album image byte layout (encoder reference) → `references/09-audio-format-spec.md`, `references/11-block-interleaving-tape-fx.md`, `references/21-original-firmware-stems.md`
- For the album header byte layout (metadata sector) → `references/21-original-firmware-stems.md`
- For the locally archived solderless source → `references/27-tools-and-utilities.md` and `sources.md`

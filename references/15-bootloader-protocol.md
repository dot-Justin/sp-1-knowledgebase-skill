# Bootloader Protocol

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11, the **solderless source archive** (ingested 2026-05-13; archive contents dated 2026-05-12), and the **solderless 2026-05-18 re-snapshot** (multi-app launcher rewrite). Primary code-of-record now: `assets/solderless-2026-05-12/js/protocol.js` + `js/firmware.js` + `js/storage.js` + `stemloader/js/sp-1_protocol.js` for line-stable existing citations; `assets/solderless-2026-05-18/utility/js/firmware.js` + `assets/solderless-2026-05-18/utility/js/protocol.js` for the cleanest standalone reference to the firmware-flash flow (the new `utility/` app is flash-only, no album mixed in). See `update-log.md` 2026-05-18 entry.

The SP-1 has a small bootloader in the first 128 KB of nRF52840 flash. **Track 1 + Track 4 + USB-C** triggers it; the device then enumerates as a USB CDC ACM device and accepts a small set of single-byte command opcodes wrapped in a COBS-framed packet protocol. The bootloader cannot read existing firmware, only write to the app slot.

This file documents the protocol at the byte level. Anyone with the solderless source can replicate it; this skill copies the constants and structure so Claude can answer questions without you needing to open the JS.

## USB device identifiers

Empirically observed on a factory Unit A 2026-05-14:

- **VID `2367`** (Teenage Engineering's registered USB Vendor ID)
- **PID `1701`** (factory bootloader)
- Manufacturer string: `teenage engineering`
- Product string: `stem player`
- Serial number: device-unique 12-hex-digit string (e.g. `C89F52B373D9`)
- Enumerates as USB CDC ACM (`/dev/ttyACMx` on Linux; `/dev/serial/by-id/usb-teenage_engineering_stem_player_<serial>-if00`)

**These are different from the IDs used by custom firmware.** `ericlewis/sp1-midi`'s `app/main.cpp` uses `0x1209/0x0001` (PID.codes test pair). The dumper project (this skill's hallucination-watchlist subject) uses `0x1209/0x0003`. The factory bootloader is the only context where you'll see `2367/1701`.

Discovery filters in host tools should match BOTH factory and custom IDs:

```js
const SP1_BOOTLOADER_FILTERS = [
  { vid: '2367', pids: ['1701'] },  // factory bootloader
  { vid: '1209', pids: ['0001'] },  // ericlewis/sp1-midi default
]
```

[Source: empirical, host: dotj-ct-dev-01, Unit A in T1+T4+USB-C bootloader mode, 2026-05-14.]

## Trigger

1. **Hold Track 1 + Track 4** (both buttons pressed simultaneously)
2. **Plug in USB-C** (data + power)
3. Wait until the Track 1 LED lights up
4. Release the buttons

The device then:

- Boots into the bootloader (not the app)
- Enumerates as a USB CDC ACM device
- Stays in bootloader mode for approximately **60 seconds** of inactivity before auto-rebooting into the app

The trigger was discovered by **B_E_N** [Lines #556, ~mid-2025]. An earlier post claimed a "step 4 — power button" that was later removed in an edit [see `corrections.md`]. The solderless utility's user-facing instructions confirm the bare two-button + USB-C combo:

> 1. make sure SP-1 is turned off (or long press •• to turn it off)
> 2. hold track 1 and 4 buttons while connecting USB-C
> 3. keep holding until track 1 LED lights up
> 4. release all buttons
> 5. click 'connect' below and select stem player

[Source: `index.html` lines 119–125, 137–143, 170–176; `stemloader.html` lines 27–29.]

## Wire format

### Baud rate

**115200 8N1** over USB CDC ACM. The solderless tool opens the port with `port.open({ baudRate: 115200 })` [Source: `firmware.js` line 51, `storage.js` lines 65/279/324, `stemloader.js` line 271]. CDC ACM disregards baud rate in practice (the underlying transport is USB packets), but conformant clients still set it.

### Packet framing (host ↔ device)

Every message in both directions is a **COBS-encoded packet** terminated by a single `0x00` byte. The decoded packet body has the structure:

```
+------+-----+-----+------+----------------+------+
| 0x51 | seq | cmd | plen | payload (plen) | crc8 |
+------+-----+-----+------+----------------+------+
   1     1     1     1         plen          1
```

- **`0x51`** ('Q' ASCII) — fixed start magic. Same byte as the `0x51` command opcode but always present at the start of every command packet body. Don't confuse the magic byte with the opcode — the magic appears in the framing, the opcode is one byte further in.
- **`seq`** — 1-byte sequence number. The host increments per command; the device echoes the host's seq in its reply. Solderless uses seq 1 to start and increments for each new request.
- **`cmd`** — 1-byte command opcode (see palette below).
- **`plen`** — 1-byte payload length.
- **`payload`** — `plen` bytes of command data.
- **`crc8`** — CRC-8 over `[0x51, seq, cmd, plen, payload...]` using a fixed 256-entry lookup table (full table reproduced below).

The complete packet body (`0x51 | seq | cmd | plen | payload | crc8`) is then **COBS-encoded** and a single `0x00` byte is appended as the frame delimiter.

[Source: `protocol.js` lines 91–127 (`cobsEncode`, `cobsDecode`, `buildPacket`, `parseResponse`).]

### CRC-8 table

Verbatim from `protocol.js` lines 64–81 (and the identical copy in `sp-1_protocol.js` lines 7–22):

```
0xea,0xd4,0x96,0xa8,0x12,0x2c,0x6e,0x50,0x7f,0x41,0x03,0x3d,0x87,0xb9,0xfb,0xc5,
0xa5,0x9b,0xd9,0xe7,0x5d,0x63,0x21,0x1f,0x30,0x0e,0x4c,0x72,0xc8,0xf6,0xb4,0x8a,
0x74,0x4a,0x08,0x36,0x8c,0xb2,0xf0,0xce,0xe1,0xdf,0x9d,0xa3,0x19,0x27,0x65,0x5b,
0x3b,0x05,0x47,0x79,0xc3,0xfd,0xbf,0x81,0xae,0x90,0xd2,0xec,0x56,0x68,0x2a,0x14,
0xb3,0x8d,0xcf,0xf1,0x4b,0x75,0x37,0x09,0x26,0x18,0x5a,0x64,0xde,0xe0,0xa2,0x9c,
0xfc,0xc2,0x80,0xbe,0x04,0x3a,0x78,0x46,0x69,0x57,0x15,0x2b,0x91,0xaf,0xed,0xd3,
0x2d,0x13,0x51,0x6f,0xd5,0xeb,0xa9,0x97,0xb8,0x86,0xc4,0xfa,0x40,0x7e,0x3c,0x02,
0x62,0x5c,0x1e,0x20,0x9a,0xa4,0xe6,0xd8,0xf7,0xc9,0x8b,0xb5,0x0f,0x31,0x73,0x4d,
0x58,0x66,0x24,0x1a,0xa0,0x9e,0xdc,0xe2,0xcd,0xf3,0xb1,0x8f,0x35,0x0b,0x49,0x77,
0x17,0x29,0x6b,0x55,0xef,0xd1,0x93,0xad,0x82,0xbc,0xfe,0xc0,0x7a,0x44,0x06,0x38,
0xc6,0xf8,0xba,0x84,0x3e,0x00,0x42,0x7c,0x53,0x6d,0x2f,0x11,0xab,0x95,0xd7,0xe9,
0x89,0xb7,0xf5,0xcb,0x71,0x4f,0x0d,0x33,0x1c,0x22,0x60,0x5e,0xe4,0xda,0x98,0xa6,
0x01,0x3f,0x7d,0x43,0xf9,0xc7,0x85,0xbb,0x94,0xaa,0xe8,0xd6,0x6c,0x52,0x10,0x2e,
0x4e,0x70,0x32,0x0c,0xb6,0x88,0xca,0xf4,0xdb,0xe5,0xa7,0x99,0x23,0x1d,0x5f,0x61,
0x9f,0xa1,0xe3,0xdd,0x67,0x59,0x1b,0x25,0x0a,0x34,0x76,0x48,0xf2,0xcc,0x8e,0xb0,
0xd0,0xee,0xac,0x92,0x28,0x16,0x54,0x6a,0x45,0x7b,0x39,0x07,0xbd,0x83,0xc1,0xff,
```

CRC initial value: `0`. Update step: `crc = table[crc ^ next_byte]`. (Standard table-lookup CRC.)

### COBS encoding

Standard COBS (Consistent Overhead Byte Stuffing). The end-of-packet delimiter is `0x00`; COBS rewrites the packet body to contain no zero bytes, prefixing each run of non-zero bytes with a length byte. See `protocol.js` lines 91–121 for the canonical encoder/decoder used by the tool.

## Command palette

The opcodes below are observed and named in the solderless source. They split into three groups: bootloader/state-transition, info/query, and bulk transfer.

| Opcode (hex) | ASCII | Direction | Reply opcode | Purpose |
| --- | --- | --- | --- | --- |
| `0x52` | `R` | host→dev | `0x53` `S` | **State query.** Returns a 5-byte ASCII state vector (see below). Used to detect mode after connection. |
| `0x70` | `p` | host→dev | `0x71` `q` | **Set mode.** Payload byte `[1]` selects upload mode. After a `0x50` reboot the device comes back in the selected mode. |
| `0x50` | `P` | host→dev | (no reply) | **Reboot.** Soft-reset; the device disconnects USB and re-enumerates. |
| `0x54` | `T` | host→dev | `0x55` `U` | **Device ID.** Returns 8 bytes (presumed FICR `DEVICEID[0..1]` LE concatenation). |
| `0x58` | `X` | host→dev | `0x59` `Y` | **Album title.** Returns current album title (ASCII, null-terminated within payload). |
| `0x66` | `f` | host→dev | `0x67` `g` | **Verify album.** Returns `[isValid (u8), errCode (u8)]`. `errCode` values per the `ALBUM_ERR` enum (see `references/09-audio-format-spec.md` / `21-original-firmware-stems.md`). |
| `0x43` | `C` | host→dev | `0x44` `D` | **Write counter.** Returns LE32 write counter (chunks written this upload session). |
| `0x7A` | `z` | host→dev | `0x7B` `{` | **Battery status.** Returns 2 bytes (raw; semantics inferred — see `references/05-power-and-battery.md`). |
| `0x64` | `d` | host→dev | `0x65` `e` | **Fader positions.** Returns 4 bytes — one byte per track fader (T1..T4). |
| `0x74` | `t` | host→dev | `0x75` `u` | **Ladder values.** Returns 4 bytes = two `uint16` LE — the two button-ladder ADC readings. |
| `0x5C` | `\` | host→dev | `0x5D` `]` | **Button states.** Returns N bytes representing current pressed/released states across the button matrix. |
| `0x5A` | `Z` | host→dev | (replies but unused) | **Set LEDs.** Payload is 8 bytes — one byte each for T1, T2, T3, T4, S1, S2, S3, S4 (brightness 0–255). |
| `0x37` | `7` | host→dev | (no reply) | **Reset write counter / start upload session.** Called once at start of album upload to reset the per-session chunk counter. |
| `0x39` | `9` | host→dev | (no reply) | **Write album chunk.** Payload is exactly 136 bytes — see "Album upload chunk" section below. |
| `0x51` | `Q` | host→dev | (no reply) | **End-of-upload / reboot to bootloader.** Sent at end of album upload; device commits and reboots. (Distinct from the framing magic `0x51` at the start of every packet body — same byte, different role.) |
| `0x46` | `F` | host→dev | `0x47` `G` | **Format / erase app slot.** Used in firmware-flash flow. |
| `0x45` | `E` | host→dev | (no reply) | **Write firmware chunk.** Payload = `[chunk_seq (LE32) | flash_addr (LE32) | data (≤240 bytes)]`. |
| `0x48` | `H` | host→dev | `0x49` `I` | **Commit / finalize firmware.** Payload = `[last_page (LE32)]` on first call, `[counter (LE32)]` on second. |

### What state-query (`0x52`) returns

The payload of `0x53 'S'` is **5 raw integer bytes (each a small int 0-9), NOT 5 ASCII characters**. Solderless and `gate-fix-testing` stringify via `Array.from(payload).join('')` — exploiting `Number.prototype.toString()` so byte value 0 → `"0"`, byte value 1 → `"1"`, etc. The two documented values:

- Bytes `[1, 0, 5, 1, 0]` → string `"10510"` — device is in **upload mode**, ready for `0x37`+`0x39` album-upload traffic.
- Bytes `[0, 0, 1, 0, 0]` → string `"00100"` — device is in **boot mode** (fresh bootloader entry, no pages written). Caller should set upload mode via `0x70 [1]` and reboot via `0x50` *if doing album upload*. For firmware-flash, boot mode IS the flash-ready state and you can go straight to `0x46`.

[Source: `storage.js` lines 87–125, `stemloader.js` lines 336–346, `gate-fix-testing/src/upload/protocol.mjs::decodeStateReply`.]

⚠ **Common AI bug** (added 2026-05-14 after the same mistake bit the dumper project): do NOT stringify these 5 bytes with `String.fromCharCode(b)`. The bootloader does not send ASCII characters; it sends raw integer bytes. Using `fromCharCode` will produce `    ` (5 control characters) instead of `"00100"`, and the boot-mode check will silently fail against real hardware. The phrase "5-byte ASCII string" in earlier versions of this skill was misleading.

Each character of the joined string represents one state flag. The exact bit assignment isn't named in the JS — the tool just string-compares. **Don't claim a richer interpretation than that** unless verified against firmware disassembly.

### Status response (`0x53` payload in firmware-flash mode)

When the device has already received its first `0x45` (i.e., mid-flash session), `0x52` returns a **4-byte payload** interpretable as LE32 — the number of pages written so far. So `0x52` is overloaded by byte length: 4 bytes → flash-in-progress page counter, 5 bytes → state-flag string. Use byte length to distinguish.

Crucially, **the firmware-flash flow does NOT require the device to already be in "flash mode"** — solderless's `firmware.js::flashFirmware` (`flashFirmware` lines 85–186) sends `0x52` once, accepts any valid `0x53` reply (including 5-byte boot-mode `"00100"`), and proceeds directly to `0x46`. Boot mode is a flash-ready state; no `0x70`/`0x50` dance is needed before flashing firmware.

## Mode switching (bootloader → upload mode)

Procedure as implemented by solderless [Source: `storage.js` lines 87–119]:

```
1. Connect CDC, send 0x52 (R)
2. Receive 0x53 (S) with state payload
3. If payload string == "10510": already in upload mode, proceed
4. If payload string == "00100": continue:
   a. Send 0x70 with payload [0x01]  ("set mode" — upload)
   b. Receive 0x71 (q) confirming mode set
   c. Send 0x50 (no reply expected) to reboot
   d. Disconnect; wait for USB re-enumeration (~3 seconds)
   e. Reconnect, send 0x52 again; expect "10510" now
```

**No GPREGRET magic value appears in the solderless source.** Earlier versions of this skill stated that GPREGRET = `0x1A96` was the host-visible mode-switch magic; that came from a Discord paraphrase. The host implementation just uses the two-command sequence above. There may be a magic value internally on the device (e.g., the bootloader checks GPREGRET on reset), but it is not exposed to the host and the host does not need to know it.

## Firmware flash sequence

[Source: `firmware.js` lines 85–186 (`flashFirmware`).]

```
1. 0x52 (R), no payload     → expect 0x53 (S) with LE32 page counter
2. 0x46 (F), no payload     → expect 0x47 (G); 5-second timeout (erases flash)
3. For each 4096-byte page (starting at FIRST_PAGE = 0x20):
   For each chunk (up to FW_CHUNK_SIZE = 240 bytes) of the page:
     0x45 (E), payload = [chunk_seq (LE32) | flash_addr (LE32) | data]   no reply
     sleep 5ms (or 100ms at the start of each new page after page 0)
4. 0x48 (H), payload = [last_page (LE32)]   → expect 0x49 (I) with counter
5. 0x48 (H), payload = [counter (LE32)]     → expect 0x49 (I) with finalize counter
6. Device reboots into new firmware
```

**Flash layout constants** [`protocol.js` lines 8–13]:

| Constant | Value | Meaning |
| --- | --- | --- |
| `FW_PAGE_SIZE` | 4096 | nRF52840 page size |
| `FW_CHUNK_SIZE` | 240 | bytes of firmware data per `0x45` packet |
| `FLASH_START` | `0x20000` | first valid app firmware address (128 KB into flash) |
| `FLASH_END` | `0xFF000` | end of app slot (last writable page) |
| `FIRST_PAGE` | `0x20` | page index for `FLASH_START` (32 = 0x20000/0x1000) |
| `LAST_PAGE` | `0xFE` | page index for `FLASH_END` (254 = 0xFE000/0x1000) |

So pages 0x00–0x1F (128 KB) are bootloader; pages 0x20–0xFE (223 pages × 4096 B = 913,408 B ≈ **892 KB**) are app; page 0xFF is reserved (likely MBR settings). **Don't overwrite the bootloader region** — if your firmware extends below 0x20000 you brick the device and recovery requires SWD.

**Why 240-byte chunks?** Likely to fit USB Full Speed bulk packets cleanly with overhead: 240 (data) + 8 (chunk_seq + flash_addr) + 5 (packet header + CRC) + COBS overhead = ~256 bytes per packet, which is one nRF52840 USB endpoint buffer.

## Album upload chunk (`0x39`)

[Source: `storage.js` lines 622–704 + `stemloader.js` lines 781–867.]

Each `0x39` packet payload is **exactly 136 bytes**:

```
+----------+------------+------------+
| u32 LE   | u32 LE     | 128 bytes  |
| chunk_n  | emmc_offset| data       |
+----------+------------+------------+
   0..3       4..7        8..135
```

- **`chunk_n`** — monotonic counter starting at 0. Increments for each `0x39` packet sent in this session. The firmware tracks this and exposes it via `0x43` for verification.
- **`emmc_offset`** — absolute byte offset within the eMMC where these 128 bytes should be written. Starts at 0 (the metadata sector) and increments by 128 per chunk.
- **`data`** — 128 bytes of album image data. Solderless slices the album image into 128-byte chunks and sends them in order; the encoder produces a 128-byte-aligned image so there is never a partial final chunk.

This is **not** a sector address. Earlier versions of this skill said byte 4–7 was "sector address (32-bit, presumably native 512-byte block addressing)" — that was speculation. Solderless source uses a plain byte offset, and the firmware presumably has the eMMC abstraction layer that translates byte offsets to native eMMC block writes internally.

### Important: 0x39 is fire-and-forget

The host **does not wait for an ACK** after each `0x39` packet. Solderless uses `sendNoReply` for all album-upload chunks [Source: `storage.js` line 644 + `stemloader.js` line 834]. The protocol is:

- Send chunk N, do not wait
- Send chunk N+1, do not wait
- Send chunk N+2, do not wait
- ...

At the end of the upload, the host can call `0x43` (read write counter) to verify the device received the expected number of chunks. There is no per-packet retry mechanism in the solderless implementation — if a chunk is dropped, the upload silently corrupts.

Throughput at 115200 baud with no per-chunk handshake: each chunk packet is ~145 bytes (1 magic + 1 seq + 1 cmd + 1 plen + 136 payload + 1 CRC + COBS overhead + 1 zero terminator). At 11520 bytes/sec raw, that's ~80 chunks/sec = **~10 KB/s practical eMMC throughput**. A 311 MB album takes ~9 hours.

## Album upload sequence (full)

[Source: `storage.js::uploadAlbum` lines 573–716.]

```
1. (Already connected and in upload mode "10510" per state query)
2. 0x37 (7), no payload, no reply expected
   → resets write counter / starts upload session
3. Build metadata sector (8192 bytes, see references/21-original-firmware-stems.md)
4. Send metadata sector as 64 chunks of 128 bytes each, with 0x39
   chunk_n = 0..63, emmc_offset = 0..8064
5. For each song:
   Build SP-1 audio bytes via encodeToSP1 (see references/09-audio-format-spec.md)
   Send the song's bytes as ceil(song_bytes/128) chunks of 128 bytes each
   chunk_n and emmc_offset continue monotonically
6. Build end-marker sector (8192 bytes of 0x00, with magic at last 13 bytes)
7. Send end-marker sector as 64 chunks of 128 bytes each
8. 0x51 (Q), no payload, no reply expected
   → tells device upload is complete; device reboots / commits
```

After step 8, the device disconnects and re-enumerates. The host can then optionally reconnect, query state with `0x52`, verify album with `0x66`, query title with `0x58`.

## What the bootloader cannot do

- **Read firmware.** No flash-read opcode exists. *"you cannot"* [Discord #firmware, ericlewis, 2026-05-08 17:13].
- **Read the eMMC.** The bootloader's `0x39` is write-only. To read album contents back, you need running app firmware that exposes a read path (not provided by stock TE firmware or `ericlewis/sp1-midi`).
- **Run user code.** Strict command set; no arbitrary code execution exposed.
- **Bypass APPROTECT.** APPROTECT still applies on next boot; your custom firmware inherits whatever protection it sets. For most custom-firmware development you don't care about APPROTECT.

## Safety considerations

A bad firmware can brick the device if it:

- Doesn't bring up USB,
- Doesn't respond to the function-button longpress for shutdown,
- Disables the audio system entirely without providing another recovery state.

Recovery: re-enter the Track 1 + Track 4 + USB-C combo from a cold start. The bootloader is in slot 0 (page 0x00–0x1F) and remains intact as long as your custom firmware doesn't extend below `FLASH_START = 0x20000`. **Don't overwrite the bootloader region.** If you do, only SWD (with mass-erase + reflash, see `references/04-debug-interfaces.md`) can recover.

tkt1000's reminder: *"If you load a bad firmware you can brick your device!"* [Discord #general, 2026-05-09 12:15].

The solderless utility's UI prints this warning verbatim:

> ⚠️ warning: the binary you upload must be compiled specifically for SP-1 or it might brick your device!

[Source: `index.html` line 128.]

## Standalone clients

`solderless.engineering` and `solderless-engineering.pages.dev` are the canonical web tool — Web Serial in Chromium-family browsers talking this protocol. The site went offline 2026-05-09 for an update [Discord #news, tkt1000] and **came back online 2026-05-18 as a complete rewrite**: a multi-app launcher with 4 sandboxed iframes (stem loader, firmware utility, device info, spoom1). The new dedicated **firmware utility** app at `assets/solderless-2026-05-18/utility/` is the cleanest public reference for the firmware-flash flow alone (no album upload mixed in). Both bundles are kept locally:

- `assets/solderless-2026-05-12/` — earlier static snapshot, preserved for citation stability of references written during the 2026-05-13 synthesis batch
- `assets/solderless-2026-05-18/` — current canonical mirror of the multi-app launcher

See `references/27-tools-and-utilities.md` for the full new-app inventory and the host-side iframe + postMessage architecture.

Other clients:

- **`theunflappable`'s `test_bootloader.py`** [Discord #firmware, 2026-05-08] — Python implementation. Does not include album upload.
- **TimK's wiki** at `github.com/timknapen/SP-1-dev/wiki/Bootloader` — canonical doc maintained by TimK. Verify current state.
- **moecal1947's Python uploader** — slow (control-transfer based; 0.75 KB/s); see `references/16-usb-upload-protocol.md`.

## Where to go next

- For the album-image bytes that get pushed → `references/16-usb-upload-protocol.md`, `references/21-original-firmware-stems.md`
- For physical bootloader-trigger details → `references/04-debug-interfaces.md`
- For the eMMC sector layout that uploaded data targets → `references/08-emmc-storage.md`
- For on-disk frame format → `references/09-audio-format-spec.md`
- For the local archive of the solderless source → `references/27-tools-and-utilities.md` and `sources.md`

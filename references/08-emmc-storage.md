# eMMC Storage

**Synthesized through:** Lines #846 (2026-05-06), Discord through 2026-05-11. Code-of-record: `storagethingies` (private, ericlewis 2026-05-09) + `sp1-midi/subsys/storage/emmc/`.

The SP-1 stores its audio album on a separate **Toshiba THGBMNG5D1LBAIL** 4 GB eMMC (eMMC 5.0). This file covers the chip, the protocol the nRF uses to talk to it, the sector layout, the prefetch model, and the album metadata format. For the byte-level audio sample layout *within* a sector see `09-audio-format-spec.md`. For the block-skipping trick that enables tape FF/RW see `11-block-interleaving-tape-fx.md`.

## The chip

| Property | Value |
| --- | --- |
| Manufacturer / part | Toshiba **THGBMNG5D1LBAIL** |
| Capacity | 4 GB |
| Standard | eMMC 5.0 (JESD84-B50) |
| Bus mode | **1-bit (DAT0 only)** — multi-bit not used |
| Clock | **32 MHz** in 1-bit mode |
| High Speed mode | **Not enabled** (spec would require HS mode for > 26 MHz — this chip works at 32 MHz without it; reason unknown, see `corrections.md`) |
| Encryption | None — *"not encrypted just formatted strangely"* [Discord #hardware, tkt1000, 2026-05-07] |

## How the nRF talks to the eMMC

The eMMC uses a 4-wire interface (CLK, CMD, DAT0, plus VCCQ supply and reset). The nRF52840 implements this with a hybrid GPIO/SPIM3 approach:

```
P0.06 CLK   — GPIO during the CMD phase (bit-bang); SPIM3 SCK during data reads
P0.07 DAT0  — GPIO during the CMD phase (bit-bang); SPIM3 MISO during data reads
P0.08 CMD   — GPIO bit-bang always (the eMMC's command line)
P0.14 VCCQ  — GPIO output, enables the eMMC's I/O voltage rail
P1.08 RST   — GPIO output, active low, asserts reset on the eMMC
```

Why this hybrid?

1. **CMD phase** uses GPIO bit-banging because the timing is forgiving and the protocol needs precise control of the CMD line direction and CRC7 generation.
2. **DATA phase** uses SPIM3 because the nRF's SPI peripheral can clock data efficiently and DMA-style into a buffer. In 1-bit eMMC mode, DAT0 is read continuously after the command — exactly the role a SPI MISO line plays.
3. The PWM peripheral is borrowed for **burst timing** during async data reads, ensuring CLK toggles consistently while the SPIM3 receives data.

This is documented in `storagethingies/EmmcDriver.cpp` and `sp1-midi/subsys/storage/emmc/EmmcDriver.cpp`. The driver header explicitly credits the implementation:

> [credit: "Tim Knapen's eMMC driver implementation for Stem Player" — reference JESD84-B50]

[code: `storagethingies/EmmcDriver.hpp`]

## Initialization sequence

The driver brings the eMMC up via an 11-step state machine (Zephyr SMF) [code: `storagethingies/EmmcDriver.cpp`]:

```
POWER_ON
   ↓ (assert VCCQ, wait for power good)
CMD0           Reset (GO_IDLE_STATE)
   ↓
CMD1_POLL      Send operating conditions, poll until ready
   ↓
CMD2           ALL_SEND_CID (read card identification)
   ↓
CMD3           SET_RELATIVE_ADDR (assign RCA)
   ↓
CMD9           SEND_CSD (read card specific data — capacity, etc.)
   ↓
CMD7           SELECT_CARD (transition to transfer state)
   ↓
CMD8           SEND_EXT_CSD (read extended CSD for device geometry)
   ↓
CMD16          SET_BLOCKLEN (set 512-byte native block size)
   ↓
VERIFY         (validation read)
   ↓
SETUP_ASYNC    (configure SPIM3 + PWM for async reads)
```

Once SETUP_ASYNC completes, the driver is ready for `read_sector()` calls. The implementation supports both **synchronous reads** (small operations) and **interrupt-driven async reads** (large prefetch operations).

## API (`storagethingies/EmmcDriver.hpp`)

```cpp
bool init();
bool read_sector(uint32_t sector_addr, uint8_t* buffer);         // 8192 bytes
bool read_blocks_sync(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
bool read_blocks(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
bool start_read_blocks_async(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
bool poll_async_read_complete(bool* success);
bool async_read_in_flight() const;
bool wait_for_async_read_signal(k_timeout_t timeout);
void abort_async_read();
```

**Read-only.** The reference code does not implement a write path. `moecal1947` is building a slow USB upload path (~0.75 KB/s via control transfers) but a proper CMD24/CMD25 write path is not in `sp1-midi` or `storagethingies`. See `known-unknowns.md` "eMMC write path."

## Sector layout

The SP-1 uses a **custom logical sector size** different from the eMMC's native block size.

| Term | Size | Notes |
| --- | --- | --- |
| **Native eMMC block** | 512 bytes | Standard eMMC 5.0 block; `CMD16` sets this |
| **TE sector** | **8192 bytes** | 16 native blocks = 1 TE sector |
| **TE block** | 2048 bytes | 4 TE blocks per sector; this is the unit of FF/RW skip granularity |
| **Frame** | 24 bytes | One audio sample point across all 4 stems × 2 channels × 3 bytes |
| **Frames per block** | 85 | 85 × 24 = 2040 bytes (the 8 byte remainder is block padding/metadata) |
| **Frames per sector** | 340 | 4 × 85 = 340 frames |

Constants in code [code: `storagethingies/DiskManager.hpp` lines 16–29]:

```cpp
static constexpr size_t kSectorSize        = 8192;
static constexpr size_t kBlockSize         = 2048;
static constexpr size_t kBlocksPerSector   = 4;
static constexpr size_t kFramesPerSector   = 340;
static constexpr size_t kFramesPerBlock    = 85;
static constexpr size_t kFrameSize         = 24;        // 8 channels × 3 bytes
static constexpr size_t kStemFrameSize     = 6;         // 2 channels × 3 bytes
static constexpr size_t kStemsPerFrame     = 4;
static constexpr size_t kEmmcBlocksPerSector = 16;      // 8192 / 512
static constexpr size_t kAudioPrefetchSlots  = 10;
static constexpr size_t kSlotTrailerWords    = 6;
```

8192 bytes per sector, but only 340 × 24 = 8160 bytes are audio. The remaining 32 bytes per sector hold **side-band data**: sync words (MIDI clock counter), LED state, and trailer metadata. See `10-midi-timing-encoding.md` for the timing data and `12-audio-engine-internals.md` for how the side data flows to the LED renderer.

### Block order on disk: 0, 2, 1, 3

The four 2 KB TE blocks within a sector are stored in physical order **0, 2, 1, 3** [code: `storagethingies/DiskManager.hpp` line 34 — `static constexpr int kBlockOrder[4] = {0, 2, 1, 3};`]. This interleaving is the trick that makes tape FF/RW work without real-time DSP throughput on the eMMC bus. See `11-block-interleaving-tape-fx.md` for the full explanation.

## Album header

The first sector(s) of the album hold the **AlbumInfo** metadata structure [code: `storagethingies/DiskManager.hpp` lines 38–51]:

```cpp
struct SongInfo {
    uint32_t start_sector;     // Offset in TE sectors (8192-byte units)
    uint32_t length_sectors;   // Length in TE sectors
    char     artist[65];
    char     title[65];
};

struct AlbumInfo {
    bool      valid = false;
    uint32_t  total_sectors = 0;
    uint8_t   song_count = 0;
    char      title[65];
    SongInfo  songs[kMaxSongs];   // kMaxSongs = 16
};
```

So an album:
- Has up to 16 songs
- Each song has a start sector and length in TE-sector units
- Strings are 65 bytes (presumably 64 chars + null terminator)

`DiskManager::parse_album_header()` extracts this from the start of the album image. `current_song()` / `song_index_for_frame()` / `song_start_frame()` give the application a song-to-frame mapping.

## Prefetch and slot model

The audio engine doesn't read the eMMC directly for every audio frame — that would be impossible at 48 kHz with the chip's bandwidth. Instead, `DiskManager` maintains a **10-slot prefetch cache** [code: `storagethingies/DiskManager.hpp` — `kAudioPrefetchSlots = 10`, `audio_slots_` array].

Each slot holds:

```cpp
struct AudioSlot {
    struct Record {
        uint8_t  payload[kSectorSize];                  // 8192 bytes of TE sector
        uint32_t block_sync_words[kBlocksPerSector];    // 4 sync words per sector
        uint32_t block_led_words[kBlocksPerSector];     // 4 LED words per sector
    } record;
    std::atomic<AudioSlotState> state;     // Empty/Queued/Reading/Ready
    std::atomic<uint32_t> te_sector;
    std::atomic<uint32_t> sample_anchor;
    std::atomic<uint32_t> read_blocks;     // 16 (full), 8 (half = 2x FF), 4 (quarter = 4x FF)
    std::atomic<int32_t>  distance_hint;   // For slot replacement decisions
};
```

The slot's `state` machine transitions through `Empty → Queued → Reading → Ready` as eMMC reads complete. The audio render path checks slots for cache hits via `try_get_frame_payload_cached()` and triggers a fault if the frame isn't yet in cache.

### Prefetch policy

The prefetch engine looks at the **playback cursor hint** (the frame the audio engine is about to render), the **direction** (forward/reverse), and the **scan rate** (which determines `read_blocks`), and decides which sector to fetch next. The 10 slots allow for ample look-ahead plus history (so a brief rewind doesn't require re-reading).

`render_protected_sectors` are sectors that the audio engine is currently rendering from — these are explicitly excluded from eviction so the audio thread doesn't see a slot get overwritten mid-render.

### Variable-block-size reads for FF/RW

Three values for `read_blocks` per slot:

| `read_blocks` | Bytes read per sector | When used |
| --- | --- | --- |
| 16 | 8192 (full sector) | 1.0x playback (Play mode) |
| 8 | 4096 (first half of sector) | 2.5x FF / RW |
| 4 | 2048 (first TE block only) | 4x and higher FF / RW |

By **reading only part of each sector at high speeds**, the eMMC bus stays comfortably within its bandwidth, and the playback engine duplicates the read frames to fill the gap. See `11-block-interleaving-tape-fx.md`.

## Render-protected sectors

The audio render thread sets two "render protected" sector IDs in `DiskManager` [code: `set_render_protected_sectors()`]. These are the sectors the renderer is actively reading from. The prefetch logic will not evict these slots even if they have low priority by distance-hint, which prevents the rare race where a slot you're reading gets reassigned mid-frame.

## Block-level direct access

`DiskManager` exposes a low-level `read_blocks()` API for use cases outside audio playback (USB mass storage exposure, firmware dumping, etc.):

```cpp
bool read_blocks(uint32_t block_addr, uint8_t* buffer, uint32_t num_blocks);
```

`block_addr` is in **native 512-byte eMMC blocks** (not TE sectors). A 311 MB album is 607,888 native blocks ≈ 37,993 TE sectors. [Discord #general, moecal1947, 2026-05-09 09:33]

## Capacity

The driver reports `emmc_capacity_blocks()` (native 512-byte blocks). For a 4 GB chip this is approximately 7,553,024 blocks. The album occupies the front portion of this; the rest is typically unused on stock firmware.

## Write path (status)

**Not implemented in the reference code.** Writing to the eMMC requires CMD24 (single-block write) or CMD25 (multi-block write), and `storagethingies` does not have a working write driver. This is the main gap blocking efficient album uploads.

Current state of the work:
- moecal1947's slow Python tool achieves ~0.75 KB/s via host-side control-transfer chunking; ~4.5 days for a 311 MB album [Discord #general, 2026-05-09 09:33]
- ericlewis stated the theoretical max for a proper implementation is ~4 MB/s [Discord #general, 2026-05-09 09:34]
- The right architecture (per moecal1947's question and ericlewis's hints): USB bulk endpoint → RAM buffer → CMD25 multi-block write
- See `16-usb-upload-protocol.md` for the opcode-level upload sequence and known gaps

## Discrepancy notes

### "Single data line" vs DAT0–DAT3

TimK said *"It's single data line eMMC 5.0"* [Discord #hardware, 2026-05-07 17:44] which describes the **nRF's** access mode (only DAT0 wired through SPIM3). But he also said *"DAT0-DAT3 are broken out to test points, so you could read much faster than the MCU!"* [Discord #hardware, 2026-05-07 17:53] — meaning the chip's full 4-bit data bus is physically routed to test points even though the nRF only uses DAT0. External tools (eMMC readers, USB-to-eMMC adapters) can use the full 4-bit interface for faster dumps; the nRF can't.

## Where to go next

- For the byte-level audio sample format inside frames → `09-audio-format-spec.md`
- For the MIDI clock side-band data in each sector → `10-midi-timing-encoding.md`
- For the 0/2/1/3 block interleaving and tape FF/RW → `11-block-interleaving-tape-fx.md`
- For the audio engine that consumes this storage → `12-audio-engine-internals.md`
- For non-destructive eMMC extraction methods → `04-debug-interfaces.md` and `24-emmc-direct-extraction.md`
- For the USB upload protocol → `16-usb-upload-protocol.md`
- For known unknowns (write path, etc.) → `known-unknowns.md`

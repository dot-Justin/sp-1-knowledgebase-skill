# AGENT-GUIDE - TE Stem Player Thread Archive

This guide is for LLM agents entering this directory. It explains what's here, how it's structured, and the fastest paths to answers.

---

## What This Archive Is

A complete export of the "TE Stem Player" thread from llllllll.co (Lines forum) - **846 posts, 179 participants, April 2024 to May 2026**. The thread documents a community reverse-engineering effort for the Teenage Engineering Stem Player prototype (TE-SP-1), a device originally made for Ye's albums. The community ultimately extracted firmware, decoded the audio format, and built a web-based updater without ever voiding a warranty.

The thread has been **closed** by Lines moderators. Ongoing discussion is at discord.gg/y4V6VfHYck.

---

## Directory Structure

```
agent/
├── AGENT-GUIDE.md          ← you are here
├── thread.md               ← full concatenated thread (~428KB, 7742 lines)
├── posts/                  ← individual posts as markdown files (001.md–846.md)
│   └── NNN.md              ← YAML frontmatter + post body
├── indexes/
│   ├── post-index.json     ← all 846 posts: metadata + excerpt (PRIMARY SEARCH INDEX)
│   ├── reply-chain-index.json  ← parent → [reply post nums] mapping
│   ├── attachment-index.json   ← all upload:// image and file refs (165 total)
│   └── topic-index.json    ← posts grouped by topic keyword (12 topics)
└── summaries/
    ├── thread-summary.md   ← full narrative: hardware, timeline, key people, links
    ├── chunk-001-100.md    ← posts 1–100
    ├── chunk-101-200.md    ← posts 101–200
    ├── chunk-201-300.md    ← posts 201–300
    ├── chunk-301-400.md    ← posts 301–400
    ├── chunk-401-500.md    ← posts 401–500
    ├── chunk-501-600.md    ← posts 501–600
    ├── chunk-601-700.md    ← posts 601–700
    ├── chunk-701-800.md    ← posts 701–800
    └── chunk-801-846.md    ← posts 801–846
```

---

## Recommended Search Strategies

### "What happened overall?"
→ Read `summaries/thread-summary.md` - hardware specs, timeline of breakthroughs, key participants, current status, all important links.

### "What happened around posts X–Y?"
→ Read the corresponding `summaries/chunk-NNN-NNN.md` file. Each covers ~100 posts with a narrative summary, key moments, and notable post numbers.

### "Which posts discuss [topic]?"
→ Read `indexes/topic-index.json`. Topics: `hardware`, `bootloader`, `firmware`, `usb_debug`, `audio`, `bluetooth`, `file_system`, `3d_printing`, `community`, `solderless_engineering`, `reverse_engineering`, `discovery`.

### "Find posts about [keyword]" / relevance filtering
→ Read `indexes/post-index.json`. Each entry has: `post_num`, `author`, `date`, `likes`, `reply_to`, `word_count`, `excerpt` (first ~220 chars of body). Scan excerpts to identify candidates, then open specific `posts/NNN.md` files.

### "What did [author] say?"
→ Filter `post-index.json` by `author` field.

### "What's the conversation around post N?"
→ Check `reply-chain-index.json`: look up N as a key (replies to N) or search entries for N in reply arrays (what N replies to). Then read the relevant `posts/NNN.md` files.

### "What images/files were shared?"
→ Read `indexes/attachment-index.json`. Each entry has: `post_num`, `author`, `date`, `name`, `token`, `type` (`image`/`file`), `attrs`. Filter by `type: "file"` for downloadable attachments (6 total).

### "Read a specific post"
→ Open `posts/NNN.md` directly. YAML frontmatter contains: `post_number`, `id`, `author`, `created_at`, `reply_to`, `likes`, `word_count`, `images`, `attachments`.

### "Read the whole thread"
→ Open `thread.md` - full concatenated export. Large file; consider reading in chunks by line offset.

---

## Post Index Entry Format

```json
{
  "post_num": 1,
  "id": 694337,
  "author": "PedalsandChill",
  "date": "2024-04-09",
  "likes": 30,
  "reply_to": null,
  "word_count": 355,
  "images": 23,
  "attachments": 77,
  "excerpt": "Please read this post! Updates and current status of the project will be added at the bottom..."
}
```

---

## Topic Index Counts

| Topic | Post Count |
|-------|-----------|
| audio | 443 |
| bluetooth | 280 |
| community | 230 |
| firmware | 178 |
| hardware | 171 |
| bootloader | 113 |
| discovery | 136 |
| usb_debug | 106 |
| file_system | 102 |
| 3d_printing | 86 |
| reverse_engineering | 83 |
| solderless_engineering | 27 |

---

## Key Facts for Quick Reference

- **Firmware dumped:** Jan 25 2025 (murray, attempt #8,504, BeagleBone PRU glitch)
- **Custom stems loaded:** Post 499 (TimK)
- **Bootloader cracked / web updater:** solderless.engineering
- **Bootloader trigger:** Hold Track 1 + Track 4 while plugging USB-C → CDC serial for 60s (post 556)
- **Audio:** 8 channels, 24-bit, 48kHz, 8K sectors, 2K chunks, 4 stereo stems
- **MIDI timing:** 96 steps/cycle, 4×24 PPQN, 0xF8 at 31,250 baud, 128μs
- **SoC:** nRF52840 (Nordic) - APPROTECT bypass via voltage glitch
- **FPC:** Omron XF3B-1945-31A (19-pin, 0.3mm)
- **GitHub:** github.com/timknapen/SP-1-dev
- **Discord:** discord.gg/y4V6VfHYck

# Lines Thread Archive (bundled subset)

This is a **subset** of the SP-1 Lines forum thread archive, included in this skill for offline agent use. The complete archive — including 813 binary attachments (images, PDFs, docs) and a rendered web frontend — lives at:

- **GitHub:** [`dot-Justin/TE-SP-1-lines-thread-archive`](https://github.com/dot-Justin/TE-SP-1-lines-thread-archive)
- **Web frontend:** [`sp-1.dotjust.in`](https://sp-1.dotjust.in/)
- **Original thread:** [`llllllll.co/t/te-stem-player/66795`](https://llllllll.co/t/te-stem-player/66795) (closed by moderators 2026-05-06)

All credit for assembling this archive goes to its author. This bundled copy is shipped with the `sp-1` skill purely for agent convenience.

## What's bundled here

| Path | Size | Purpose |
| --- | --- | --- |
| `agent/AGENT-GUIDE.md` | 7 KB | Folder structure + recommended search strategies |
| `agent/thread.md` | 4 KB | Top-level narrative summary |
| `agent/summaries/` | 54 KB | Per-100-posts narrative chunks (`chunk-001-100.md` etc.) |
| `agent/indexes/` | 200 KB | JSON indexes: post metadata, topic groupings, reply chains, participants, attachments |
| `agent/posts/` | 1.4 MB | All 846 posts as individual markdown files (`001.md` through `846.md`), with YAML frontmatter |
| `metadata/` | 210 KB | Forum participants list, thread stats, upload manifest (token → filename mapping for attachments hosted on the source repo) |

**Total: ~1.6 MB, 866 files.**

## What's NOT bundled

- **`raw/api/`** (4.4 MB) — the original Discourse API JSON responses. Redundant with `agent/posts/`. If you need byte-identical original API data, fetch from the source repo.
- **`raw/posts/`** (3.8 MB) — same content as `agent/posts/`, different format.
- **`raw/assets/uploads/`** (90 MB) — 813 binary attachments (images, PDFs, etc.). Too large for this skill bundle. Resolve attachment tokens via `metadata/upload_manifest.json`, then fetch from the source repo or browse `sp-1.dotjust.in`.
- **`site-src/`** (106 MB) — the static web frontend at `sp-1.dotjust.in`. Not relevant for agent use.

## Citation form

When citing material from this bundled subset:

```
[Lines #NNN, <author>, <YYYY-MM-DD>]
```

Same form as elsewhere in the skill. Resolve post numbers against `agent/indexes/post-index.json` to verify author and date.

If a question requires reading a binary attachment, point the user at the source repo's `raw/assets/uploads/` directory or the `sp-1.dotjust.in` frontend.

## Where to start (agent quickstart)

From `agent/AGENT-GUIDE.md`:

1. **For a question about a specific topic** (e.g., "what does the firmware say about Bluetooth?"): grep `agent/indexes/topic-index.json` for the topic, then read the linked posts.
2. **For a specific post number** (e.g., "what did Galapagoose say in post 68?"): read `agent/posts/068.md`.
3. **For a narrative overview**: read `agent/thread.md` or the appropriate `agent/summaries/chunk-NNN-MMM.md`.
4. **For tracing a reply thread**: use `agent/indexes/reply-chain-index.json` to map parent ↔ replies.
5. **For finding all posts by an author**: search `agent/indexes/post-index.json` for the author field.

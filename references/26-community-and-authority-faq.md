# FAQ — Community and Authority

**Q: Who's the lead on firmware?**
A: **TimK** (= tkt1000 on Discord = timknapen on GitHub). Owns `github.com/timknapen/SP-1-dev` and is the most active firmware authority. See `26-community-and-authority.md`.

**Q: Who's the lead on the audio engine?**
A: **ericlewis** (Discord + GitHub). Wrote `audiothingies`/`storagethingies` reference implementations and the public `sp1-midi` BSP.

**Q: Are TimK and ericlewis the same person?**
A: **No.** Different people. Both active in Discord. TimK = firmware/PCB lead; ericlewis = audio engine / storage lead. Don't conflate. See `hallucination-watchlist.md`.

**Q: Who's the authority on the bootloader?**
A: **TimK** for documentation (`github.com/timknapen/SP-1-dev/wiki/Bootloader`). **B_E_N** discovered the Track 1 + Track 4 + USB-C trigger originally [Lines #556]. **ericlewis** has the opcode-level command set documented in Discord [#general, 2026-05-09].

**Q: Who did the firmware glitch attack?**
A: **murray**. With assistance from **elew** running 5 parallel units. Attempt #8,504 succeeded on 2025-01-25.

**Q: Who decoded the audio format?**
A: **JoseJX** (Lines, original work) and **Galapagoose** (Lines, MIDI clock finalization). **ericlewis** wrote the modern reference code (`audiothingies`).

**Q: Who's writing the LED bit-bang dump firmware?**
A: **zee_33** (Discord). Their code isn't public yet; demonstrated 2026-05-08 in #firmware.

**Q: Who has working Bluetooth?**
A: **emvee1968** claims working pairing in unreleased custom firmware [Discord #firmware, 2026-05-08]. Awaiting release.

**Q: Who runs `solderless.engineering`?**
A: **TimK + loksi + tunelight + keebstudios** as a team [Discord #general, tkt1000, 2026-05-06].

**Q: Who maintains the Lines thread archive at sp-1.dotjust.in?**
A: **dotjustin** (this user). Public repo: `github.com/dot-Justin/TE-SP-1-lines-thread-archive`.

**Q: Where's the official history of the device?**
A: **maybe** (Lines handle; **sankebergel** on Discord) wrote the canonical pre-2024 timeline in Lines #743. Authoritative on F&F origins, 2018-2020 development, Kano collaboration.

**Q: Where did the Lines thread go?**
A: Closed by moderators on 2026-05-06 due to spam volume around copyright/dump requests [Discord #general, tkt1000, 2026-05-08]. Still readable (thread not deleted). Community moved to Discord.

**Q: Where's the Discord invite?**
A: Linked from `github.com/timknapen/SP-1-dev` README. Not linked from this skill directly.

**Q: How big is the community?**
A: Lines thread: 846 posts, 179 participants, 78K views. Discord: smaller but more active. Most technical contributors are 5-10 people; broader audience is hundreds.

**Q: Are there any "users-only" channels in the Discord?**
A: Channels visible to the bot (which feeds this skill's synthesis): #general, #firmware, #hardware, #introductions, #news. Whether others exist with restricted visibility is unknown.

**Q: How active is TimK on Discord?**
A: Very active. Responds within hours typically. Most willing to help with firmware / hardware questions.

**Q: How active is ericlewis on Discord?**
A: Active but says he's "quite busy" [Discord #firmware, 2026-05-09 00:20]. Brief but technically deep responses.

**Q: Should I @ TimK / ericlewis for technical questions?**
A: For specific, well-formulated questions: yes. For "can someone help me with X?": ask in the channel first, let people self-select.

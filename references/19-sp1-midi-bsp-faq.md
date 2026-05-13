# FAQ — `sp1-midi` BSP

**Q: What does `sp1-midi` do?**
A: Turns the SP-1 into a USB MIDI 2.0 controller. Faders → CC 1-4, track buttons → notes 60-63, Play → MMC Play/Stop, etc. **Doesn't play audio.** See `19-sp1-midi-bsp.md`.

**Q: Where's the repo?**
A: `github.com/ericlewis/sp1-midi`. Published 2026-05-09.

**Q: What CC numbers do faders use?**
A: Faders 1–4 → CC 1, 2, 3, 4 (channel 1). Volume up/down → CC 7 in ±10 steps.

**Q: What notes do track buttons play?**
A: Track 1 → note 60 (C4), Track 2 → 61, etc. Octave offset adjustable ±5 via the FF/RW rockers.

**Q: What does the function button do?**
A: Toggle MMC Record On/Off (0x06 / 0x07). Long-press (3 seconds) fires `INPUT_KEY_POWER` — but the BSP doesn't actually call `sys_poweroff()` from that handler; you'd add that to your app.

**Q: What does Play do?**
A: Toggle MMC Play (0x02) / Stop (0x01). Sent as SysEx envelope `F0 7F 7F 06 <cmd> F7`.

**Q: What USB IDs does it use?**
A: VID 0x1209 / PID 0x0001. **Change these for public release.**

**Q: How is the state machine structured?**
A: Zephyr SMF with 4 states: IDLE, RUNNING, SHUTTING_DOWN, DEEP_SLEEP. Skeleton in `app/AppStateMachine.cpp`.

**Q: What does PowerManager do?**
A: Monitors battery via AIN4, charger via BQ24232 GPIOs, USB attach. Exposes "is charging / on USB" queries.

**Q: Does it play audio at all?**
A: **No.** The codecs are configured but the I²S TX path isn't wired up. To play audio, integrate `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/` and add an I²S TX backend. See `12-audio-engine-internals.md`.

**Q: Can I send MIDI from a host to control LEDs / state?**
A: Yes — `MidiController::on_rx()` is the incoming handler. Currently minimal; extend it to map host CC events to LEDs etc.

**Q: How is the watchdog used?**
A: Main registers a 5000 ms channel and feeds it during init. Subsystems can register their own channels. Channel timeout resets the device.

**Q: How do I add a new state?**
A: 1) Add to `AppState` enum, 2) Add entry/run/exit handlers, 3) Add to SMF state table. See `19-sp1-midi-bsp.md` "How to extend".

**Q: Where do I add audio engine code?**
A: Drop `assets/audiothingies-2026-05-09/` and `assets/storagethingies-2026-05-09/` into a new directory under `sp1-midi/`. Add files to `APP_SOURCES` in `CMakeLists.txt`. Initialize the engine in `main()` after codecs are ready.

**Q: Is the function button the only direct-GPIO button?**
A: Yes. All other buttons are on analog ladders (read via SAADC). See `02-hardware-overview.md`.

**Q: What MMC commands does the BSP send?**
A: Stop (0x01), Play (0x02), FastForward (0x04), Rewind (0x05), RecordOn (0x06), RecordOff (0x07), Pause (0x09). [code: `MidiController.hpp` lines 30-36]

**Q: What's the manufacturer string?**
A: "Teenage Engineering" — set by ericlewis for compatibility. **Change this for non-official firmware** out of courtesy.

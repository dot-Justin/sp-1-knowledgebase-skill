# FAQ — Zephyr Build Environment

**Q: What do I need to install?**
A: Zephyr base + Zephyr SDK + CMake 3.20+ + Ninja + Python venv. Follow Zephyr's "Getting Started" first. See `18-zephyr-build-environment.md`.

**Q: What's the build command?**
A: 
```sh
cmake -B build -GNinja -DBOARD=stem_player -DSNIPPET=cdc-acm-console
ninja -C build
```
Make sure `ZEPHYR_BASE` and `ZEPHYR_SDK_INSTALL_DIR` are set.

**Q: What board name?**
A: `stem_player`. The board definition lives at `boards/teenageengineering/stem_player/` in the `sp1-midi` repo.

**Q: Why the `cdc-acm-console` snippet?**
A: Gives you a Zephyr shell over USB CDC ACM during runtime. Without it, no serial output. Strongly recommended for development.

**Q: How do I flash without SWD?**
A: Build → produces `build/zephyr/zephyr.bin` → upload via `solderless.engineering` or your own bootloader-protocol client. See `15-bootloader-protocol.md`.

**Q: How do I flash with SWD?**
A: `ninja -C build flash` after wiring an SWD header to the device. Uses West's flash backend (typically nrfjprog).

**Q: What Zephyr version?**
A: Anything with `CONFIG_USB_DEVICE_STACK_NEXT` support — the next-gen USB stack. Recent Zephyr LTS or main branch works.

**Q: Why C++17?**
A: ericlewis chose C++17 for the BSP. Compiles with `-fno-exceptions -fno-rtti`. If you fork, you can use C if you prefer.

**Q: What's the memory budget?**
A: Bare BSP ~168 KB flash / ~43 KB RAM. Plenty of headroom for adding audio engine (+~150 KB RAM for full TE-equivalent firmware).

**Q: I²S block count is 8 — why so many?**
A: `CONFIG_I2S_NRFX_TX_BLOCK_COUNT=8` — deeper than typical (2-4). Absorbs transient eMMC cache misses without audible audio underrun. See `12-audio-engine-internals.md`.

**Q: Can I disable the watchdog for debugging?**
A: Yes, but the BSP's app uses it. Either don't call `start()` (skip watchdog init) or remove the related config. Don't disable it for production firmware — it's your safety net.

**Q: What VID/PID does sp1-midi use?**
A: VID 0x1209 (PID.codes test VID), PID 0x0001. **Change this if you publish public custom firmware.** See `19-sp1-midi-bsp.md`.

**Q: Build fails with "BOARD=stem_player not found" — what now?**
A: Make sure you're running cmake from the `sp1-midi` repo root, where `boards/teenageengineering/stem_player/` is at the expected relative location. Or specify `BOARD_ROOT` explicitly.

**Q: USB enumeration fails after flashing?**
A: Try unplug/replug. If on Linux: `sudo udevadm control --reload-rules`. The device may need a USB-C cable swap or a full host reboot.

**Q: My custom firmware boots but the LEDs don't light up?**
A: PWM peripherals need to be enabled and the device-tree PWM specs need to be referenced. Check `pwm_dt_spec_get` calls in your code. Also confirm the LED PWM nodes are `okay` status in the DTS.

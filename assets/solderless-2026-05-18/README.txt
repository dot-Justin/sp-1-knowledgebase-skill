Solderless SP-1 apps local copy

This folder is a local mirror of solderless.engineering as of 2026-05-18.
The code in solderless-engineering.pages.dev should match.

What changed since the 2026-05-12 snapshot:
- The site is no longer a single-page upload tool. It is now a multi-app
  launcher hosting four apps as sandboxed iframes:
    1. stem loader  (evolved from the old upload tool)
    2. firmware utility  (NEW — browser-based custom-firmware flasher)
    3. device info  (NEW — live runtime-state viewer)
    4. spoom1  (NEW — Doom controlled by the SP-1's faders + buttons)
- The parent index.html owns one WebSerial port. Each iframe app uses
  ./js/serial-shim.js to call navigator.serial as if it had its own port;
  the shim postMessage-proxies all RX/TX to the parent. Only the active
  iframe can send TX.

What is inside:
- Main launcher:
  ./index.html
- Stem loader (album upload + in-app help):
  ./stemloader/
- Firmware utility (custom-firmware flasher):
  ./utility/
- Device info (live runtime queries):
  ./deviceinfo/
- Spoom1 (Doom):
  ./doom/
  NOTE: doom/wasm/doom.wasm was not present on the live site at scrape
  time. The page likely fetches it on demand from a CDN. The other apps
  are fully self-contained.

How to run:
1. Open Terminal.
2. cd into this folder:
   cd ~/.claude/skills/sp-1/assets/solderless-2026-05-18
3. Start a local web server:
   python3 -m http.server 8788
4. Open Chrome or Edge, not Safari.
5. Go to:
   http://127.0.0.1:8788/
6. Click 'connect' and select the SP-1 from the WebSerial port picker.
7. Use the left-rail icons to switch between the four apps.

Notes:
- WebSerial requires Chrome or Edge.
- Connect the SP-1 over USB-C.
- For the stem loader and the firmware utility, the SP-1 must be in
  bootloader mode: power off, hold Track 1 + Track 4, plug in USB-C,
  release after Track 1 LED lights.
- The stem loader expects SP-1-ready WAV files: 24 bit, 48 kHz PCM,
  1-N channels (channels 1-8 map to stems 1L/1R..4L/4R; channels beyond
  8 are ignored; missing stems play silent).
- Leave the Terminal window open while using the site.
- Stop the server with Control-C in Terminal.

Provenance:
  Scrape date: 2026-05-18
  Origin: solderless.engineering (HTTP 200, live)
  Method: wget --mirror --convert-links --page-requisites
  44 files, ~601 KB extracted.

For citation guidance see ../sources.md (look for the
solderless-2026-05-18 entry).

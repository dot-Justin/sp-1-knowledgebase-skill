Solderless SP-1 uploader local copy

This folder is a local backup of the Solderless Engineering SP-1 utilities.
The code in solderless-engineering.pages.dev is unchanged.

What is inside:
- Main Solderless utility:
  solderless-engineering.pages.dev/index.html
- Main page "upload audio" tool:
  open the main site, then choose upload audio
- TKT stemloader:
  solderless-engineering.pages.dev/stemloader/

How to run:
1. Open Terminal.
2. cd into this folder:
   cd ~/Desktop/solderless-sp1-uploader-share-2026-05-12/solderless-engineering.pages.dev
3. Start a local web server:
   python3 -m http.server 8788
4. Open Chrome or Edge, not Safari.
5. Go to:
   http://127.0.0.1:8788/
6. For the TKT stemloader directly, go to:
   http://127.0.0.1:8788/stemloader/

Notes:
- WebSerial requires Chrome or Edge.
- Connect the SP-1 over USB.
- The uploader expects SP-1-ready WAV files: 8 channel, 24 bit, 48 kHz PCM.
- The pages use the stock SP-1 boot/upload parser and write stock-format album data.
- Leave the Terminal window open while using the site.
- Stop the server with Control-C in Terminal.

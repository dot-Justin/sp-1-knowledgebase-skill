/**
 * SP-1 serial protocol: constants, CRC, COBS, packet framing, serial I/O,
 * state machine helpers, and shared utilities.
 */

// ─── Flash constants ────────────────────────────────────────────────────────

const FW_PAGE_SIZE   = 4096;
const FW_CHUNK_SIZE  = 240;
const FLASH_START    = 0x20000;
const FLASH_END      = 0xFF000;
const FIRST_PAGE     = 0x20;
const LAST_PAGE      = 0xFE;

// ─── Audio / album constants ────────────────────────────────────────────────

const FRAME_SIZE        = 24;
const STEM_FRAME_SIZE   = 6;
const BLOCK_SIZE        = 2048;
const SECTOR_SIZE       = 8192;
const FRAMES_PER_SECTOR = 340;
const BLOCK_ORDER       = [0, 2, 1, 3];

const OFFSET_MAGIC       = 0;
const OFFSET_ALBUM_LEN   = 13;
const OFFSET_NUM_SONGS   = 17;
const OFFSET_ALBUM_TITLE = 18;
const OFFSET_SONGS       = 82;
const STRING_LENGTH      = 64;
const META_SONG_LENGTH   = 136;

const ALBUM_ERR = {
  OK: 0, MAGIC_NOT_FOUND: 1, ALBUM_TOO_SHORT: 2, ALBUM_TOO_LONG: 3,
  TOO_MANY_SONGS: 4, ALBUM_TITLE_TOO_LONG: 5, ALBUM_TITLE_ALLOC_FAILED: 6,
  SONG_INDEX_ALLOC_FAILED: 7, SONG_OFFSET_INVALID: 8, SONG_LENGTH_TOO_LARGE: 9,
  SONG_EXCEEDS_ALBUM: 10, ARTIST_NAME_TOO_LONG: 11, ARTIST_NAME_ALLOC_FAILED: 12,
  SONG_TITLE_TOO_LONG: 13, SONG_TITLE_ALLOC_FAILED: 14, MAGIC_NOT_FOUND_AT_END: 15,
};

const ALBUM_ERR_MSG = {
  [ALBUM_ERR.MAGIC_NOT_FOUND]: 'no album found',
  [ALBUM_ERR.ALBUM_TOO_SHORT]: 'album too short',
  [ALBUM_ERR.ALBUM_TOO_LONG]: 'album too long',
  [ALBUM_ERR.TOO_MANY_SONGS]: 'too many songs',
  [ALBUM_ERR.ALBUM_TITLE_TOO_LONG]: 'album title too long',
  [ALBUM_ERR.ALBUM_TITLE_ALLOC_FAILED]: 'album title alloc fail',
  [ALBUM_ERR.SONG_INDEX_ALLOC_FAILED]: 'song index alloc fail',
  [ALBUM_ERR.SONG_OFFSET_INVALID]: 'invalid song offset',
  [ALBUM_ERR.SONG_LENGTH_TOO_LARGE]: 'song too large',
  [ALBUM_ERR.SONG_EXCEEDS_ALBUM]: 'song exceeds album',
  [ALBUM_ERR.ARTIST_NAME_TOO_LONG]: 'artist name too long',
  [ALBUM_ERR.ARTIST_NAME_ALLOC_FAILED]: 'artist name alloc fail',
  [ALBUM_ERR.SONG_TITLE_TOO_LONG]: 'song title too long',
  [ALBUM_ERR.SONG_TITLE_ALLOC_FAILED]: 'song title alloc fail',
  [ALBUM_ERR.MAGIC_NOT_FOUND_AT_END]: 'no magic at end',
};

function toHexString(byteArray) {
  return Array.from(byteArray, b => ('0' + (b & 0xFF).toString(16)).slice(-2)).join('');
}

// ─── CRC-8 table ────────────────────────────────────────────────────────────

const CRC8_TABLE = new Uint8Array([
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
]);

// ─── Protocol helpers ───────────────────────────────────────────────────────

function crc8(data) {
  let crc = 0;
  for (let i = 0; i < data.length; i++) crc = CRC8_TABLE[crc ^ data[i]];
  return crc;
}

function cobsEncode(data) {
  const out = [];
  let idx = 0;
  while (idx <= data.length) {
    let end = idx;
    while (end < data.length && data[end] !== 0) end++;
    out.push(end - idx + 1);
    for (let i = idx; i < end; i++) out.push(data[i]);
    if (end < data.length) idx = end + 1;
    else break;
  }
  out.push(0x00);
  return new Uint8Array(out);
}

function cobsDecode(raw) {
  let data = raw;
  if (data.length > 0 && data[data.length - 1] === 0)
    data = data.slice(0, data.length - 1);
  const out = [];
  let idx = 0;
  while (idx < data.length) {
    const code = data[idx++];
    if (code === 0) break;
    for (let i = 0; i < code - 1; i++) {
      if (idx < data.length) out.push(data[idx++]);
    }
    if (code < 0xFF && idx < data.length) out.push(0);
  }
  return new Uint8Array(out);
}

function buildPacket(cmd, payload = new Uint8Array(), seq = 1) {
  const body = new Uint8Array([0x51, seq & 0xFF, cmd, payload.length, ...payload]);
  const c = crc8(body);
  return cobsEncode(new Uint8Array([...body, c]));
}

function parseResponse(data) {
  if (!data || data.length < 3) return null;
  const decoded = cobsDecode(data);
  if (decoded.length < 5) return null;
  const [, seq, cmd, plen] = decoded;
  if (decoded.length < 5 + plen) return null;
  const payload = decoded.slice(4, 4 + plen);
  const gotCrc = decoded[4 + plen];
  const expCrc = crc8(decoded.slice(0, 4 + plen));
  return { cmd, seq, payload, crcOk: gotCrc === expCrc };
}

function le32(v) {
  return new Uint8Array([v & 0xFF, (v >>> 8) & 0xFF, (v >>> 16) & 0xFF, (v >>> 24) & 0xFF]);
}

function readLE32(b, o = 0) {
  return (b[o] | (b[o+1] << 8) | (b[o+2] << 16) | (b[o+3] << 24)) >>> 0;
}

function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }
function tick() { return new Promise(r => setTimeout(r, 20)); }
function hex(v, w = 2) { return '0x' + v.toString(16).padStart(w, '0'); }

function terminateStr(bytes) {
  const out = new Uint8Array(bytes.length + 1);
  out.set(bytes);
  return out;
}

// ─── Serial I/O ─────────────────────────────────────────────────────────────

let port = null, reader = null, writer = null;
let rxBuf = new Uint8Array();

async function serialWrite(data) { await writer.write(data); }

async function readUntilZero(timeoutMs = 3000) {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    const zi = rxBuf.indexOf(0x00);
    if (zi >= 0) {
      const pkt = rxBuf.slice(0, zi + 1);
      rxBuf = rxBuf.slice(zi + 1);
      return pkt;
    }
    try {
      const rem = Math.max(50, deadline - Date.now());
      const r = await Promise.race([
        reader.read(),
        sleep(rem).then(() => ({ value: null, done: true }))
      ]);
      if (r.done || !r.value) continue;
      const c = new Uint8Array(r.value);
      const m = new Uint8Array(rxBuf.length + c.length);
      m.set(rxBuf); m.set(c, rxBuf.length);
      rxBuf = m;
    } catch { break; }
  }
  return null;
}

function drainRx() { rxBuf = new Uint8Array(); }

async function sendCmd(cmd, payload = new Uint8Array(), seq = 1, timeout = 3000) {
  drainRx();
  await serialWrite(buildPacket(cmd, payload, seq));
  const raw = await readUntilZero(timeout);
  return raw ? parseResponse(raw) : null;
}

async function sendNoReply(cmd, payload = new Uint8Array(), seq = 1) {
  await serialWrite(buildPacket(cmd, payload, seq));
}

async function closeSerialPort() {
  try { if (reader) { await reader.cancel(); reader.releaseLock(); } } catch {}
  try { if (writer) { writer.releaseLock(); } } catch {}
  try { if (port) { await port.close(); } } catch {}
  reader = null;
  writer = null;
  port = null;
}

// ─── State machine ──────────────────────────────────────────────────────────

let currentMode = 'home';

const $ = id => document.getElementById(id);

function setState(name) {
  document.querySelectorAll('[data-state]').forEach(el => {
    el.classList.toggle('active', el.dataset.state === name);
  });

  const mainView = $('mainView');
  if (mainView) {
    const showMain = ['home', 'fw-step1', 'fw-step2', 'storage-connect', 'storage-connecting', 'storage-editor', 'info-connect', 'info-querying', 'info-display'].includes(name);
    mainView.style.display = showMain ? 'flex' : 'none';
  }

  const isHome = name === 'home';
  const infoBox = $('infoBox');
  const compatLabel = $('compatLabel');
  const deviceGrid = $('deviceGrid');
  if (infoBox) infoBox.style.display = isHome ? '' : 'none';
  if (compatLabel) compatLabel.style.display = isHome ? '' : 'none';
  if (deviceGrid) deviceGrid.style.display = isHome ? '' : 'none';

  if (name.startsWith('fw-')) currentMode = 'fw';
  else if (name.startsWith('storage-')) currentMode = 'storage';
  else if (name.startsWith('info-')) currentMode = 'info';
  else currentMode = 'home';
}

function logMsg(msg, cls = '') {
  const panel = $('logPanel');
  const span = document.createElement('span');
  if (cls) span.className = cls;
  span.textContent = msg + '\n';
  panel.appendChild(span);
  panel.scrollTop = panel.scrollHeight;
}

// ─── Log toggle ─────────────────────────────────────────────────────────────

$('logToggle').addEventListener('click', () => {
  const panel = $('logPanel');
  const visible = panel.style.display === 'block';
  panel.style.display = visible ? 'none' : 'block';
  $('logToggle').textContent = visible ? 'log' : 'hide log';
});

// ─── Easter egg ─────────────────────────────────────────────────────────────

const playBtn = $('playBtn');
const topReel = $('topReel');
const bottomReel = $('bottomReel');

function setReelSpinning(active) {
  if (playBtn) playBtn.classList.toggle('active', active);
  if (topReel) topReel.classList.toggle('spinning', active);
  if (bottomReel) bottomReel.classList.toggle('spinning', active);
}

if (playBtn) {
  playBtn.addEventListener('click', () => {
    setReelSpinning(!playBtn.classList.contains('active'));
  });
}

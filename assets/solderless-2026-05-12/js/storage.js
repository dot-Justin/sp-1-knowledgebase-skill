/**
 * Storage / album management: connection flow, album editor UI,
 * WAV file handling, and upload to device.
 * Depends on: protocol.js, wav-parser.js (loaded first)
 */

// ─── Storage: State ─────────────────────────────────────────────────────────

let storageWaitingReconnect = false;
let nextFileID = 0;
const fileMap = new Map();

function storageSetProgress(pct, text) {
  $('storageProgFill').style.width = pct + '%';
  $('storageProgPct').textContent = text || (Math.round(pct) + '%');
}

// ─── Storage: Serial events ─────────────────────────────────────────────────

let closePromise = Promise.resolve();

if ('serial' in navigator) {
  navigator.serial.addEventListener('disconnect', () => {
    if (currentMode === 'storage' && storageWaitingReconnect) {
      logMsg('device disconnected, waiting for reconnect...');
      closePromise = closeSerialPort();
    }
  });

  navigator.serial.addEventListener('connect', (e) => {
    if (currentMode === 'storage' && storageWaitingReconnect) {
      logMsg('device reconnected.');
      closePromise.then(() => storageOpenPort(e.target));
    }
  });
}

// ─── Storage: Connection ────────────────────────────────────────────────────

$('btnModeStorage').addEventListener('click', () => setState('storage-connect'));
$('btnStorageBack1').addEventListener('click', () => setState('home'));

$('btnStorageConnect').addEventListener('click', async () => {
  if (!('serial' in navigator)) {
    logMsg('webserial not supported.', 'err');
    $('storageErrText').textContent = 'webserial is not supported. use chrome or edge.';
    setState('storage-error');
    return;
  }
  try {
    port = await navigator.serial.requestPort();
    await storageOpenPort(port);
  } catch (e) {
    logMsg('connection failed: ' + e.message, 'err');
    $('storageErrText').textContent = 'connection failed. make sure device is in bootloader mode.';
    setState('storage-error');
    return;
  }
});

async function storageOpenPort(p) {
  try {
    await closeSerialPort();
    await sleep(400);
    if (!p.readable) await p.open({ baudRate: 115200 });
    reader = p.readable.getReader();
    writer = p.writable.getWriter();
    port = p;
  } catch (e) {
    logMsg('port open failed: ' + e.message, 'err');
    $('storageErrText').textContent = 'failed to open port: ' + e.message;
    setState('storage-error');
    return;
  }

  logMsg('connected, querying device state...');
  let seq = 1;

  const resp = await sendCmd(0x52, new Uint8Array(), seq++, 3000);
  if (!resp || !resp.crcOk) {
    logMsg('device did not respond.', 'err');
    $('storageErrText').textContent = 'device not responding. check connection.';
    setState('storage-error');
    return;
  }

  const stateStr = resp.payload.join('');
  logMsg('device state: ' + stateStr);

  if (stateStr === '10510') {
    storageWaitingReconnect = false;
    logMsg('device in upload mode.', 'ok');
    setState('storage-editor');
    queryDeviceInfo(document.createElement('div'), null, true);
    return;
  }

  if (stateStr === '00100') {
    logMsg('device in boot mode, transitioning to upload mode...');
    setState('storage-connecting');

    const r1 = await sendCmd(0x70, new Uint8Array([1]), seq++, 3000);
    if (!r1 || !r1.crcOk || r1.cmd !== 0x71) {
      logMsg('failed to set upload mode.', 'err');
      $('storageErrText').textContent = 'failed to set upload mode.';
      setState('storage-error');
      return;
    }
    logMsg('mode set, rebooting device...');
    storageWaitingReconnect = true;

    await sendNoReply(0x50, new Uint8Array(), seq++);

    setTimeout(() => {
      if (storageWaitingReconnect) {
        $('btnStorageReconnect').style.display = '';
      }
    }, 6000);
    return;
  }

  logMsg('device in unknown state: ' + stateStr, 'err');
  $('storageErrText').textContent = 'device in unknown state. try reconnecting.';
  setState('storage-error');
}

$('btnStorageReconnect').addEventListener('click', async () => {
  storageWaitingReconnect = false;
  $('btnStorageReconnect').style.display = 'none';
  await closeSerialPort();
  try {
    port = await navigator.serial.requestPort();
    await storageOpenPort(port);
  } catch (e) {
    logMsg('reconnect failed: ' + e.message, 'err');
    $('storageErrText').textContent = 'reconnect failed: ' + e.message;
    setState('storage-error');
  }
});

$('btnStorageDisconnect').addEventListener('click', async () => {
  storageWaitingReconnect = false;
  await closeSerialPort();
  logMsg('disconnected.');
  setState('home');
});

$('btnStorageDone').addEventListener('click', async () => {
  await closeSerialPort();
  clearAlbum();
  setState('home');
});

$('btnStorageRetry').addEventListener('click', () => {
  setState('storage-connect');
});

// ─── Device info query ───────────────────────────────────────────────────────

async function queryDeviceInfo(targetGrid, onDone, compact = false) {
  targetGrid.innerHTML = '';

  function addRow(label, value, cls = '') {
    const l = document.createElement('span');
    l.className = 'di-label';
    l.textContent = label;
    const v = document.createElement('span');
    v.className = 'di-value' + (cls ? ' ' + cls : '');
    v.textContent = value;
    targetGrid.appendChild(l);
    targetGrid.appendChild(v);
  }

  let seq = 10;

  try {
    const albumResp = await sendCmd(0x58, new Uint8Array(), seq++, 2000);
    if (albumResp && albumResp.crcOk && albumResp.cmd === 0x59) {
      const title = String.fromCharCode.apply(null, albumResp.payload);
      addRow('album', title || '(empty)');
      logMsg('album title: "' + title + '"');
      const lbl = $('albumLabel');
      if (lbl) lbl.textContent = title || '';
    } else {
      addRow('album', '—');
    }
  } catch { addRow('album', '—'); }

  try {
    const verifyResp = await sendCmd(0x66, new Uint8Array(), seq++, 3000);
    if (verifyResp && verifyResp.crcOk && verifyResp.cmd === 0x67) {
      const isValid = verifyResp.payload[0];
      const errNum = verifyResp.payload[1];
      if (isValid) {
        addRow('status', 'valid', 'ok');
        logMsg('album data: valid');
      } else {
        const errMsg = ALBUM_ERR_MSG[errNum] || ('error ' + errNum);
        addRow('status', errMsg, errNum === ALBUM_ERR.MAGIC_NOT_FOUND ? 'warn' : 'err');
        logMsg('album data: ' + errMsg);
      }
    } else {
      addRow('status', '—');
    }
  } catch { addRow('status', '—'); }

  if (!compact) {
    try {
      const idResp = await sendCmd(0x54, new Uint8Array(), seq++, 2000);
      if (idResp && idResp.crcOk && idResp.cmd === 0x55) {
        const idHex = toHexString(idResp.payload);
        addRow('device id', idHex);
        logMsg('device id: ' + idHex);
      }
    } catch {}

    try {
      const wcResp = await sendCmd(0x43, new Uint8Array(), seq++, 2000);
      if (wcResp && wcResp.crcOk && wcResp.cmd === 0x44) {
        const buf = new Uint8Array(wcResp.payload).buffer;
        const counter = new DataView(buf).getUint32(0, true);
        addRow('write counter', counter.toString());
        logMsg('write counter: ' + counter);
      }
    } catch {}

    try {
      const batResp = await sendCmd(0x7A, new Uint8Array(), seq++, 2000);
      if (batResp && batResp.crcOk && batResp.cmd === 0x7B) {
        addRow('battery', batResp.payload[0] + ', ' + batResp.payload[1] + ' (raw)');
        logMsg('battery: ' + batResp.payload[0] + ', ' + batResp.payload[1]);
      }
    } catch {}

    try {
      const faderResp = await sendCmd(0x64, new Uint8Array(), seq++, 2000);
      if (faderResp && faderResp.crcOk && faderResp.cmd === 0x65) {
        const f = faderResp.payload;
        addRow('faders', 'T1:' + f[0] + '  T2:' + f[1] + '  T3:' + f[2] + '  T4:' + f[3]);
        logMsg('faders: ' + f[0] + ' ' + f[1] + ' ' + f[2] + ' ' + f[3]);
      }
    } catch {}

    try {
      const ladderResp = await sendCmd(0x74, new Uint8Array(), seq++, 2000);
      if (ladderResp && ladderResp.crcOk && ladderResp.cmd === 0x75) {
        const lv = new Uint16Array(new Uint8Array(ladderResp.payload).buffer);
        addRow('ladders', lv[0] + ', ' + lv[1]);
        logMsg('ladders: ' + lv[0] + ', ' + lv[1]);
      }
    } catch {}

    try {
      const btnResp = await sendCmd(0x5C, new Uint8Array(), seq++, 2000);
      if (btnResp && btnResp.crcOk && btnResp.cmd === 0x5D) {
        addRow('buttons', Array.from(btnResp.payload).join(' '));
        logMsg('buttons: ' + Array.from(btnResp.payload).join(' '));
      }
    } catch {}
  }

  if (onDone) onDone();
}

// ─── Device info mode ────────────────────────────────────────────────────────

$('btnModeInfo').addEventListener('click', () => setState('info-connect'));
$('btnInfoBack').addEventListener('click', () => setState('home'));

$('btnInfoConnect').addEventListener('click', async () => {
  if (!('serial' in navigator)) {
    logMsg('webserial not supported.', 'err');
    return;
  }
  try {
    const p = await navigator.serial.requestPort();
    await closeSerialPort();
    await sleep(400);
    if (!p.readable) await p.open({ baudRate: 115200 });
    reader = p.readable.getReader();
    writer = p.writable.getWriter();
    port = p;
  } catch (e) {
    logMsg('connection failed: ' + e.message, 'err');
    return;
  }

  setState('info-querying');
  logMsg('connected, querying device...');

  let seq = 1;
  const resp = await sendCmd(0x52, new Uint8Array(), seq++, 3000);
  if (!resp || !resp.crcOk) {
    logMsg('device did not respond.', 'err');
    await closeSerialPort();
    setState('info-connect');
    return;
  }

  const stateStr = resp.payload.join('');
  logMsg('device state: ' + stateStr);

  if (stateStr !== '10510' && stateStr !== '00100') {
    logMsg('device in unknown state: ' + stateStr, 'err');
    await closeSerialPort();
    setState('info-connect');
    return;
  }

  if (stateStr === '00100') {
    const r1 = await sendCmd(0x70, new Uint8Array([1]), seq++, 3000);
    if (!r1 || !r1.crcOk) {
      logMsg('failed to set upload mode.', 'err');
      await closeSerialPort();
      setState('info-connect');
      return;
    }
    await sendNoReply(0x50, new Uint8Array(), seq++);
    logMsg('rebooting to upload mode, reconnect when ready...');
    await closeSerialPort();
    await sleep(3000);
    try {
      port = await navigator.serial.requestPort();
      if (!port.readable) await port.open({ baudRate: 115200 });
      reader = port.readable.getReader();
      writer = port.writable.getWriter();
    } catch (e) {
      logMsg('reconnect failed: ' + e.message, 'err');
      setState('info-connect');
      return;
    }
  }

  await queryDeviceInfo($('infoPanelGrid'), () => {
    setState('info-display');
  });
});

$('btnInfoDisconnect').addEventListener('click', async () => {
  await closeSerialPort();
  logMsg('disconnected.');
  setState('home');
});

// ─── Audio preview ───────────────────────────────────────────────────────────

let previewCtx = null;
let previewSource = null;
let previewFileID = -1;

function stopPreview() {
  if (previewSource) {
    try { previewSource.stop(); } catch {}
    previewSource = null;
  }
  const prev = document.querySelector('.song-play.playing');
  if (prev) prev.classList.remove('playing');
  previewFileID = -1;
  setReelSpinning(false);
}

async function togglePreview(fileID, btn) {
  if (previewFileID === fileID) {
    stopPreview();
    return;
  }
  stopPreview();

  const file = fileMap.get(fileID);
  if (!file) return;

  if (!previewCtx) previewCtx = new AudioContext();

  try {
    const buf = await file.arrayBuffer();
    const audioBuf = await previewCtx.decodeAudioData(buf);
    previewSource = previewCtx.createBufferSource();
    previewSource.buffer = audioBuf;
    previewSource.connect(previewCtx.destination);
    previewSource.onended = () => {
      if (previewFileID === fileID) stopPreview();
    };
    previewSource.start();
    previewFileID = fileID;
    btn.classList.add('playing');
    setReelSpinning(true);
  } catch (e) {
    logMsg('preview failed: ' + e.message, 'err');
  }
}

// ─── Storage: Album editor ──────────────────────────────────────────────────

function formatDuration(totalFrames) {
  const duration = totalFrames / 48000;
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  return minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
}

function addSongToUI(fileID, fileName, songTitle, totalFrames) {
  const numSectors = Math.ceil(totalFrames / FRAMES_PER_SECTOR);
  const songList = $('songList');
  const songCount = songList.querySelectorAll('.song-row').length;

  const row = document.createElement('div');
  row.className = 'song-row';
  row.dataset.fileId = fileID;
  row.dataset.numSectors = numSectors;
  row.dataset.totalFrames = totalFrames;
  row.id = 'song-' + fileID;

  const playSvg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="43.8 38 97.2 97.9"><path d="M136.924,88.8934L47.284,41.9486c-.7382-.387-1.6228-.3598-2.3357.0711s-1.148,1.2046-1.148,2.0368v93.8873c0,.8324.4349,1.606,1.148,2.0368.3765.2278.8052.3431,1.2316.3431.3785,0,.757-.0899,1.1041-.2698l89.64-46.9427c.7842-.412,1.2757-1.2234,1.2757-2.1101s-.4915-1.6981-1.2757-2.1079h0v.0003h0Z" fill="#f33"/></svg>`;

  row.innerHTML = `
    <div class="song-nr" data-nr="${songCount + 1}">${songCount + 1}</div>
    <button class="song-play" title="preview">${playSvg}</button>
    <input type="text" class="song-title-input" placeholder="song title" value="${songTitle}">
    <input type="text" class="song-artist-input" placeholder="artist">
    <div class="song-dur">${formatDuration(totalFrames)}</div>
    <button class="song-delete" title="remove">&times;</button>
    <div class="song-meta-panel">
      <span class="meta-label">file</span>
      <span class="meta-value">${fileName}</span>
      <span class="meta-label">sectors</span>
      <span class="meta-value">0x${numSectors.toString(16)}</span>
      <span class="meta-label">bpm</span>
      <input type="text" class="song-bpm-input" value="80">
    </div>
  `;

  row.querySelector('.song-nr').addEventListener('click', () => {
    row.classList.toggle('expanded');
  });

  const playBtn = row.querySelector('.song-play');
  playBtn.addEventListener('click', () => togglePreview(fileID, playBtn));

  row.querySelector('.song-delete').addEventListener('click', () => {
    if (previewFileID === fileID) stopPreview();
    fileMap.delete(fileID);
    row.remove();
    rebuildAlbumIndex();
  });

  songList.appendChild(row);
}

function rebuildAlbumIndex() {
  const rows = $('songList').querySelectorAll('.song-row');
  let albumOffset = 1;
  let nr = 1;

  rows.forEach(row => {
    const numSectors = Number(row.dataset.numSectors);
    row.querySelector('.song-nr').textContent = nr;
    row.querySelector('.song-nr').dataset.nr = nr;
    albumOffset += numSectors;
    nr++;
  });

  const totalSectors = albumOffset;
  const duration = (340 * (totalSectors - 1)) / 48000;
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  $('albumDuration').textContent = rows.length > 0
    ? minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0')
    : '';
}

function clearAlbum() {
  stopPreview();
  $('albumTitle').value = 'New Album Name';
  const lbl = $('albumLabel');
  if (lbl) lbl.textContent = '';
  $('songList').innerHTML = '';
  $('albumDuration').textContent = '';
  fileMap.clear();
  nextFileID = 0;
}

// ─── Storage: WAV file handling ─────────────────────────────────────────────

async function processWavFiles(files) {
  for (const file of files) {
    const fileID = nextFileID++;
    fileMap.set(fileID, file);
    let songTitle = file.name.replace(/\.[^/.]+$/, '').replaceAll(/[_-]/g, ' ');

    let wavData;
    try {
      const buf = await file.arrayBuffer();
      wavData = wavVerify(buf);
    } catch (err) {
      logMsg('error: ' + file.name + ' — ' + err.message, 'err');
      await tick();
      continue;
    }

    addSongToUI(fileID, file.name, songTitle, wavData.totalFrames);
  }
  rebuildAlbumIndex();
}

$('wavInput').addEventListener('change', (e) => {
  processWavFiles(e.target.files);
  e.target.value = '';
});

// drag & drop
(function setupDropZone() {
  const overlay = $('wavDropOverlay');

  window.addEventListener('dragenter', (e) => {
    e.preventDefault();
    if (currentMode === 'storage') overlay.style.display = 'flex';
  });

  window.addEventListener('dragleave', (e) => {
    if (e.relatedTarget === null) overlay.style.display = 'none';
  });

  window.addEventListener('dragover', (e) => {
    e.preventDefault();
    const fileItems = [...e.dataTransfer.items].filter(i => i.kind === 'file');
    if (fileItems.length > 0) e.dataTransfer.dropEffect = 'move';
  });

  window.addEventListener('drop', (e) => {
    e.preventDefault();
    overlay.style.display = 'none';
    if (currentMode !== 'storage') return;
    const files = [...e.dataTransfer.items].map(i => i.getAsFile()).filter(Boolean);
    if (files.length > 0) processWavFiles(files);
  });
})();

$('btnClearAlbum').addEventListener('click', clearAlbum);

$('albumTitle').addEventListener('input', () => {
  const lbl = $('albumLabel');
  if (lbl) lbl.textContent = $('albumTitle').value;
});

// ─── Storage: Upload to device ──────────────────────────────────────────────

$('btnUpload').addEventListener('click', async () => {
  const songRows = $('songList').querySelectorAll('.song-row');
  if (songRows.length === 0) {
    logMsg('no songs to upload.', 'err');
    return;
  }
  if (!port || !writer) {
    logMsg('not connected to device.', 'err');
    $('storageErrText').textContent = 'not connected to device. please reconnect.';
    setState('storage-error');
    return;
  }

  stopPreview();
  setState('storage-uploading');
  const startTime = Date.now();

  try {
    await uploadAlbum(songRows, startTime);
  } catch (e) {
    logMsg('upload error: ' + e.message, 'err');
    $('storageErrText').textContent = 'upload failed: ' + e.message;
    setState('storage-error');
  }
});

async function uploadAlbum(songRows, startTime) {
  const encoder = new TextEncoder();
  const magic = encoder.encode('ALBUM_PRESENT');

  let albumTitle = $('albumTitle').value.substring(0, STRING_LENGTH - 1) || 'untitled';
  const songs = [];
  let albumOffset = 1;

  for (const row of songRows) {
    const numSectors = Number(row.dataset.numSectors);
    const fileID = Number(row.dataset.fileId);
    const title = row.querySelector('.song-title-input').value.substring(0, STRING_LENGTH - 1) || 'untitled';
    const artist = row.querySelector('.song-artist-input').value.substring(0, STRING_LENGTH - 1) || 'unknown';
    const bpmStr = row.querySelector('.song-bpm-input')?.value || '80';
    const bpm = parseInt(bpmStr) || 80;

    songs.push({ fileID, title, artist, bpm, offset: albumOffset, length: numSectors });
    albumOffset += numSectors;
  }

  const albumLength = albumOffset + 1;

  $('storageUploadText').textContent = 'preparing metadata...';
  storageSetProgress(0, '0%');
  logMsg('uploading album: "' + albumTitle + '" (' + songs.length + ' songs)');

  const metaData = new ArrayBuffer(SECTOR_SIZE);
  const metaBytes = new Uint8Array(metaData);
  metaBytes.fill(0x58);
  const metaView = new DataView(metaData);

  metaBytes.set(magic, OFFSET_MAGIC);
  metaView.setUint32(OFFSET_ALBUM_LEN, albumLength, true);
  metaView.setUint8(OFFSET_NUM_SONGS, songs.length);

  const titleBytes = terminateStr(encoder.encode(albumTitle));
  metaBytes.set(titleBytes, OFFSET_ALBUM_TITLE);

  let songMetaOffset = OFFSET_SONGS;
  for (const song of songs) {
    metaView.setUint32(songMetaOffset, song.offset, true);
    metaView.setUint32(songMetaOffset + 4, song.length, true);
    metaBytes.set(terminateStr(encoder.encode(song.artist)), songMetaOffset + 8);
    metaBytes.set(terminateStr(encoder.encode(song.title)), songMetaOffset + 8 + STRING_LENGTH);
    songMetaOffset += META_SONG_LENGTH;
  }

  await sendCmd(0x37, new Uint8Array(), 1, 3000);

  const WRITE_CMD = 0x39;
  const CHUNK_SIZE = 128;
  const CHUNKS_PER_SECTOR = 64;
  const PAYLOAD_SIZE = 136;

  let emmcOffset = 0;
  let chunkCounter = 0;

  let totalAlbumChunks = CHUNKS_PER_SECTOR;
  for (const song of songs) {
    totalAlbumChunks += song.length * CHUNKS_PER_SECTOR;
  }
  totalAlbumChunks += CHUNKS_PER_SECTOR;

  $('storageUploadText').textContent = 'writing metadata...';

  for (let offset = 0; offset < SECTOR_SIZE; offset += CHUNK_SIZE) {
    const payload = new Uint8Array(PAYLOAD_SIZE);
    const pv = new DataView(payload.buffer);
    pv.setUint32(0, chunkCounter, true);
    pv.setUint32(4, emmcOffset, true);
    payload.set(metaBytes.subarray(offset, offset + CHUNK_SIZE), 8);
    await sendNoReply(WRITE_CMD, payload, chunkCounter);
    emmcOffset += CHUNK_SIZE;
    chunkCounter++;
  }

  storageSetProgress(2, '2%');

  for (let si = 0; si < songs.length; si++) {
    const song = songs[si];
    $('storageUploadText').textContent = `converting "${song.title}"...`;
    logMsg(`converting: ${song.title}`);
    await tick();

    const file = fileMap.get(song.fileID);
    if (!file) throw new Error('file not found for: ' + song.title);

    const buf = await file.arrayBuffer();
    const wavData = parseWAV(buf);
    const sp1Data = encodeToSP1(wavData.channels, wavData.totalFrames, song.bpm);

    $('storageUploadText').textContent = `uploading "${song.title}" (${si + 1}/${songs.length})...`;
    logMsg(`uploading: ${song.title} (${sp1Data.length} bytes)`);

    for (let offset = 0; offset < sp1Data.length; offset += CHUNK_SIZE) {
      const payload = new Uint8Array(PAYLOAD_SIZE);
      const pv = new DataView(payload.buffer);
      pv.setUint32(0, chunkCounter, true);
      pv.setUint32(4, emmcOffset, true);
      payload.set(sp1Data.subarray(offset, offset + CHUNK_SIZE), 8);
      await sendNoReply(WRITE_CMD, payload, chunkCounter);
      emmcOffset += CHUNK_SIZE;
      chunkCounter++;

      if (chunkCounter % 64 === 0) {
        const pct = Math.min(95, (chunkCounter / totalAlbumChunks) * 95);
        const elapsed = (Date.now() - startTime) / 1000;
        const mins = Math.floor(elapsed / 60);
        const secs = Math.ceil(elapsed - mins * 60);
        storageSetProgress(pct, Math.round(pct) + '% — ' + mins.toString().padStart(2,'0') + ':' + secs.toString().padStart(2,'0'));
        await tick();
      }
    }
  }

  $('storageUploadText').textContent = 'writing end marker...';

  const endBuf = new Uint8Array(SECTOR_SIZE);
  endBuf.fill(0x00);
  endBuf.set(magic, SECTOR_SIZE - magic.length);

  for (let offset = 0; offset < SECTOR_SIZE; offset += CHUNK_SIZE) {
    const payload = new Uint8Array(PAYLOAD_SIZE);
    const pv = new DataView(payload.buffer);
    pv.setUint32(0, chunkCounter, true);
    pv.setUint32(4, emmcOffset, true);
    payload.set(endBuf.subarray(offset, offset + CHUNK_SIZE), 8);
    await sendNoReply(WRITE_CMD, payload, chunkCounter);
    emmcOffset += CHUNK_SIZE;
    chunkCounter++;
  }

  storageSetProgress(100, 'complete');

  const elapsed = (Date.now() - startTime) / 1000;
  const mins = Math.floor(elapsed / 60);
  const secs = Math.ceil(elapsed - mins * 60);
  logMsg(`upload complete: ${chunkCounter} chunks in ${mins.toString().padStart(2,'0')}:${secs.toString().padStart(2,'0')}`, 'ok');

  await sendNoReply(0x51, new Uint8Array(), 1);

  $('storageDoneText').innerHTML = `upload complete.<br>${songs.length} song${songs.length > 1 ? 's' : ''} uploaded in ${mins.toString().padStart(2,'0')}:${secs.toString().padStart(2,'0')}.`;
  setState('storage-done');
}

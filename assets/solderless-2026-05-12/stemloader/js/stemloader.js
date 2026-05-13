/***
 * SP-1 stem loader
 * 
 */

var stemLoaderLog;
var album_offset = 1; // current offset in album, songs start at sector 1

// map to store file references
var nextFileID = 0; // counter for unique file IDs
var fileMap = new Map();

// Device states
const DEVICE = {
  DISCONNECTED: -2,
  WAIT_FOR_ENUM: -1, // waiting for restarted device to enumerate again
  CONNECTED: 0,
  BOOT_MODE: 1,
  TRANSFER_MODE: 2
}

var deviceMode = DEVICE.DISCONNECTED;

function stemLoaderInit() {
  stemLoaderLog = document.getElementById('stem-log');

  // prep WAV drop zone
  setupWAVDropzone();

  if (!('serial' in window.navigator)) {
    stemLog('No webserial!');
    hidePanels();
    showWebserialWarning();
    return;
  }
  // setup serial events
  navigator.serial.addEventListener("disconnect", ({ target: port }) => {
    // disconnect fires when the connection is lost unintentionally
    console.log("Port disconnect event:", port);
    setDeviceMode(DEVICE.WAIT_FOR_ENUM);
    closePort(port);
  });

  navigator.serial.addEventListener("connect", ({ target: port }) => {
    console.log("Port connect event:", port);
    openPort(port);
  });

  setDeviceMode(DEVICE.DISCONNECTED);

  // rebuildAlbumIndex();
}

/**
 * setup events for drag and drop of WAV files
 */
function setupWAVDropzone() {
  let dropOverlay = document.getElementById("wav-drop-overlay");

  window.addEventListener('dragenter', e => {
    e.preventDefault();
    // show our drop overlay
    dropOverlay.style.display = 'flex';
  });

  window.addEventListener('dragleave', e => {
    // only fire whe we really leave the window
    if (e.relatedTarget === null) {
      dropOverlay.style.display = 'none';
    }
  });

  window.addEventListener("dragover", (e) => {
    e.preventDefault();
    const fileItems = [...e.dataTransfer.items].filter(
      (item) => item.kind === "file",
    );
    if (fileItems.length > 0) {
      e.dataTransfer.dropEffect = "move";
    }
  });

  window.addEventListener("drop", (e) => {
    e.preventDefault();
    dropOverlay.style.display = 'none';
    if ([...e.dataTransfer.items].some((item) => item.kind === "file")) {
      dropSongs(e);
    }
  });

  const fileInput = document.getElementById("wav-input");
  fileInput.addEventListener("change", (e) => {
    processWavFiles(e.target.files);
  });

}

/**
 * Get files out of drop event and send to processing
 */
function dropSongs(event) {
  event.preventDefault();
  const files = [...event.dataTransfer.items]
    .map((item) => item.getAsFile())
    .filter((file) => file);

  processWavFiles(files);
}

/**
 * process received files through input or drag n drop
 */
async function processWavFiles(files) {
  for (let file of files) {
    // add to map
    let fileID = nextFileID++
    fileMap.set(fileID, file);
    let songTitle = file.name.replace(/\.[^/.]+$/, "");
    songTitle = songTitle.replaceAll(/[_-]/g, " ");
    // console.log("Loading " + songTitle);

    let wavData;
    try {
      const buf = await file.arrayBuffer();
      wavData = wavVerify(buf);
    } catch (err) {
      stemLog('Error for ' + file.name + ': ' + err.message);
      continue;
    }
    const { fmt, totalFrames } = wavData;

    albumAddSong("", songTitle, totalFrames, file.name, fileID);
  }

  rebuildAlbumIndex();
}

/**
 * display the correct UI for the current device state
 */
function setDeviceMode(mode) {
  hidePanels();
  deviceMode = mode;
  switch (mode) {
    case DEVICE.DISCONNECTED:
      stemLog('SP-1 disconnected');
      showConnectPanel();
      break;
    case DEVICE.CONNECTED:
      stemLog('SP-1 connected');
      break;
    case DEVICE.BOOT_MODE:
      showBootPanel();
      stemLog('SP-1 boot mode');
      break;
    case DEVICE.TRANSFER_MODE:
      showTransferPanel();
      stemLog('SP-1 transfer mode');
      break;
    case DEVICE.WAIT_FOR_ENUM:
      showWaitForConnection();
      stemLog('SP-1 waiting for connection..');
      break;
  }
}

/**
 * Hides all UI panels
 */
function hidePanels() {
  let panels = document.getElementsByClassName('panel');
  for (let panel of panels) {
    panel.style.display = 'none';
  };
}


function showConnectPanel() {
  let panel = document.getElementById('connect-panel');
  panel.style.display = "block";
}

function showBootPanel() {
  let panel = document.getElementById('boot-panel');
  panel.style.display = "block";
}

function showTransferPanel() {
  let panel = document.getElementById('transfer-panel');
  panel.style.display = "block";
}

function showWaitForConnection() {
  let panel = document.getElementById('wait-panel');
  panel.style.display = "block";
}

function showWebserialWarning() {
  let panel = document.getElementById('webserial-warning');
  panel.style.display = "block";
}

function showDebugPanel() {
  let panel = document.getElementById('debug-panel');
  panel.style.display = "block";
}

function hideDebugPanel() {
  let panel = document.getElementById('debug-panel');
  panel.style.display = "none";
}

function hideLog() {
  let panel = document.getElementById('stem-log-container');
  panel.classList.remove('visible');
}

function showLog() {
  let panel = document.getElementById('stem-log-container');
  panel.classList.add('visible');
}

/**
 * Start a connection to a serial port, setup the serial events
 */
async function connectToDevice() {
  try {
    port = await navigator.serial.requestPort();
    openPort(port);
  } catch (e) {
    stemLog('Connection failed: ' + e.message);
    return;
  }
}

/**
 * User requested disconnect from serial port
 */
async function disconnectDevice() {
  if (port == null) {
    return;
  }
  try {
    await closePort(port);
  } catch (e) {
    console.log(e.message);
  }
  setDeviceMode(DEVICE.DISCONNECTED);
}


/**
 * Close a disconnected port
 */
async function closePort(port) {
  console.log("Closing port...");
  writer?.releaseLock();
  reader?.releaseLock();
  try {
    await port.close();
  } catch (e) {
    console.log("Failed to close port: " + e.message);
  }
}

/**
 * Open a serial port
 */
async function openPort(port) {
  try {
    await port.open({ baudRate: 115200 });
    reader = port.readable.getReader();
    writer = port.writable.getWriter();
  } catch (e) {
    console.log("port open failed:" + e.message);
    return;
  }

  setDeviceMode(DEVICE.CONNECTED);
  // send a command right away to request device state
  await sendSP1Cmd('R', new Uint8Array());
}

/**
 * Helper for printing byte arrays as hex strings
 */
function toHexString(byteArray) {
  return Array.from(byteArray, function (byte) {
    return ('0' + (byte & 0xFF).toString(16)).slice(-2);
  }).join('')
}

/**
 * Send a command to SP-1 and deal with the response
 */
async function sendSP1Cmd(cmd, payload = new Uint8Array()) {
  let seq = 1;
  let cmdChar = cmd.charCodeAt(0);
  console.log("Send '" + cmd + "' (0x" + cmdChar.toString(16) + "/" + cmdChar + ")");

  let resp;
  try {
    resp = await sendCmd(cmdChar, payload, seq++, 3000);
  } catch (e) {
    stemLog("Failed to send command. Was a connection to the device made?");
    return;
  }
  let respStr; // payload as string
  let respStrNum; // payload as number representation string
  let respStrHex; // payload as hex string
  if (!resp) {
    console.log("Empty response");
    return;
  }
  if (resp.payload) {
    respStr = String.fromCharCode.apply(null, resp.payload);
    respStrNum = resp.payload.join("");
    respStrHex = toHexString(resp.payload);
  }

  console.log("Received '" + String.fromCharCode(resp.cmd) + "' (0x" + resp.cmd.toString(16) + "/" + resp.cmd + ") - " + respStr + ' - ' + respStrNum);

  if (!resp.crcOk) {
    stemLog("Response CRC fail!");
    return;
  }
  if (resp.cmd == 0xff) {
    stemLog("Error: " + respStr);
    return
  }

  // We received some kind of valid response
  switch (String.fromCharCode(resp.cmd)) {
    case 'S':
      // R: get state request
      if (respStrNum == "00100") {
        setDeviceMode(DEVICE.BOOT_MODE);
        // device is in boot mode, auto move to transfer mode
        sendSP1Cmd('p', new Uint8Array([1]));
      } else if (respStrNum == "10510") {
        setDeviceMode(DEVICE.TRANSFER_MODE);
      } else {
        stemLog("Device in unknown mode - " + respStrNum);
        setDeviceMode(DEVICE.CONNECTED);
      }
      break;
    case 'Q':
      // P: Continue boot to app - no response
      break;
    case 'q':
      // p 1: Mode choice
      {
        stemLog("Set mode OK");
        // continue to app after setting mode
        sendSP1Cmd('P');
      }
      break;
    case 'U':
      // T: Device id
      stemLog("Device ID: " + respStrHex);
      break;
    case 'Y':
      // X: Album title
      stemLog('Album title: "' + respStr + '"');
      break;
    case 'D':
      // C: Write counter
      {
        const buffer = new Uint8Array(resp.payload).buffer;
        let resp32LE = new DataView(buffer).getUint32(0, true);
        stemLog('Write counter: ' + resp32LE);
      }
      break;
    case 'e':
      // d: Fader positions
      {
        let t1_vol = resp.payload[0];
        let t2_vol = resp.payload[1];
        let t3_vol = resp.payload[2];
        let t4_vol = resp.payload[3];
        stemLog('Faders: ' + t1_vol + ' ' + t2_vol + ' ' + t3_vol + ' ' + t4_vol);
      }
      break;
    case 'g':
      // f - verify album data
      {
        let album_is_read = resp.payload[0];
        let album_err_num = resp.payload[1];
        stemLog('Album data ' + (album_is_read ? 'OK' : 'FAIL') + (album_err_num == 0 ? '' : ' - error: ' + album_err_num));
        switch (album_err_num) {
          case ALBUM_ERR.MAGIC_NOT_FOUND:
            stemLog('Album not present');
            break;
          case ALBUM_ERR.ALBUM_TOO_SHORT:
            stemLog('Album too short');
            break;
          case ALBUM_ERR.ALBUM_TOO_LONG:
            stemLog('Album too long');
            break;
          case ALBUM_ERR.TOO_MANY_SONGS:
            stemLog('Too many songs');
            break;
          case ALBUM_ERR.ALBUM_TITLE_TOO_LONG:
            stemLog('Album title too long');
            break;
          case ALBUM_ERR.ALBUM_TITLE_ALLOC_FAILED:
            stemLog('Album title alloc fail');
            break;
          case ALBUM_ERR.SONG_INDEX_ALLOC_FAILED:
            stemLog('Song index alloc fail');
            break;
          case ALBUM_ERR.SONG_OFFSET_INVALID:
            stemLog('Invalid song offset');
            break;
          case ALBUM_ERR.SONG_LENGTH_TOO_LARGE:
            stemLog('Song too large');
            break;
          case ALBUM_ERR.SONG_EXCEEDS_ALBUM:
            stemLog('Song exceeds album');
            break;
          case ALBUM_ERR.ARTIST_NAME_TOO_LONG:
            stemLog('Artist name too long');
            break;
          case ALBUM_ERR.ARTIST_NAME_ALLOC_FAILED:
            stemLog('Artist name alloc fail');
            break;
          case ALBUM_ERR.SONG_TITLE_TOO_LONG:
            stemLog('Song title too long');
            break;
          case ALBUM_ERR.SONG_TITLE_ALLOC_FAILED:
            stemLog('Song title alloc fail');
            break;
          case ALBUM_ERR.MAGIC_NOT_FOUND_AT_END:
            stemLog('No magic at end');
            break;
        }
      }
      break;
    case '{':
      // z: battery status
      {
        let val_1 = resp.payload[0];
        let val_2 = resp.payload[1];
        stemLog('Battery status: ' + val_1 + ' ' + val_2);
      }
      break;
    case 'u':
      // t: ladders
      {
        const ladders = new Uint16Array(new Uint8Array(resp.payload).buffer);
        stemLog('Ladders: ' + ladders[0] + ' ' + ladders[1]);
      }
      break;
    case ']':
      // \: button states
      {
        stemLog('Buttons: ' + respStrNum);
      }
      break;
    case '[':
      // Z: LEDs set
      break;
    case '8':
      // 7: reset write counter
      break;
    default:
      stemLog("Unknown response: '" + String.fromCharCode(resp.cmd) + "' - '" + respStr + "' " + respStrNum);
      break;
  }
}

/**
 * Add a message to the log on the page
 */
function stemLog(msg) {
  let el = document.createElement('div');
  el.innerText = msg;
  stemLoaderLog.appendChild(el);
}

function clearLog() {
  stemLoaderLog.innerHTML = "";
}

/*** ALBUM EDITOR */
var selSong = null; // currently selected song


/**
 * Helper to extract BPM from filename
 * @param {*} str 
 * @returns 
 */
function extractBPM(str) {
  const match = str.match(/(\d+(?:\.\d+)?)\s*bpm/i);
  if (match) {
    return parseFloat(match[1]);
  } else {
    // default to 80
    return 80;
  }
}

/**
 * Adds a song to the album
 */
function albumAddSong(songArtist = '', songTitle = '', totalFrames = 0, songFileName = '', fileID = -1) {

  // song length is expressed in frames, convert to time and sectors
  const duration = totalFrames / 48000;
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  let durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');

  // convert to number of sectors 
  const numSectors = Math.ceil(totalFrames / FRAMES_PER_SECTOR);
  let numSectorsHex = "0x" + numSectors.toString(16);

  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');
  let songNR = songs.length + 1;
  let songOffsetHex = "0x" + album_offset.toString(16);
  album_offset += numSectors;

  let song = document.createElement('div');
  song.dataset.fileID = fileID;
  song.dataset.numSectors = numSectors;
  song.classList.add('song');
  song.setAttribute('id', 'song-' + fileID);

  let BPM = extractBPM(songFileName);

  song.innerHTML = `
  <div class="song-top">
    <div class="song-nr">${songNR}</div>
    <div><input type="text" class="song-title" placeholder="" value="${songTitle}"/></div>
    <div><input type="text" class="song-artist" placeholder="" value="${songArtist}"/></div>
    <div><input type="text" class="song-bpm" placeholder="" value="${BPM}"/></div>
    <div class="song-delete"></div>
  </div>
  <div class="song-meta">
   <div class="song-meta-row">
      <div class="label">file</div>
      <div class="song-file">${songFileName}</div>
    </div>
    <div class="song-meta-row">
      <div class="label">duration</div>
      <div class="song-duration"> ${durationStr} </div>
    </div>
    <div class="song-meta-row">
      <div class="label">offset</div>
      <div class="song-offset"> ${songOffsetHex}</div>
    </div>
    <div class="song-meta-row">
      <div class="label">sectors</div>
      <div class="song-length">${numSectorsHex}</div>
    </div>
  </div>
  `;

  let songNr = song.getElementsByClassName('song-nr')[0];
  songNr.onclick = () => { selectSong(fileID) };

  let songClose = song.getElementsByClassName('song-delete')[0];
  songClose.onclick = () => { albumRemoveSong(fileID) };

  songList.appendChild(song);
}

/**
 * update song numbers, offsets and album length
 */
function rebuildAlbumIndex() {
  album_offset = 1;

  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');

  let songNR = 1;
  for (let song of songs) {
    let songLength = Number(song.dataset.numSectors);
    let songOffsetHex = "0x" + album_offset.toString(16);
    song.getElementsByClassName('song-nr')[0].innerText = songNR;
    song.getElementsByClassName('song-offset')[0].innerText = songOffsetHex;
    album_offset += songLength;
    songNR++;
  }

  document.getElementById('album-length').innerText = '0x' + (album_offset + 1).toString(16);

  // album_offset = number of sectors + 1 metadata sector
  const duration = (340 * (album_offset - 1)) / 48000;
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  let durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
  document.getElementById('album-duration').innerText = durationStr;
}

/**
 * Select a song and have it reveal its metadata
 */
function selectSong(songID) {
  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');

  let newSelSong = document.getElementById('song-' + songID);
  if (selSong == newSelSong) {
    // song was already selected, deselect
    for (let song of songs) {
      song.classList.remove('selected');
    }
    selSong = null;
  } else {
    for (let song of songs) {
      song.classList.remove('selected');
    }
    selSong = newSelSong;
    selSong.classList.add('selected');
  }
}

/**
 * Mark a song to show it is being converted
 */
function markSong(songID) {
  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');

  for (let song of songs) {
    song.classList.remove('marked');
  }

  let markedSong = document.getElementById('song-' + songID);
  if (markedSong) {
    markedSong.classList.add('marked');
  }
}

/**
 * Remove a song from the album
 */
function albumRemoveSong(songID) {
  let song = document.getElementById('song-' + songID);
  let fileID = Number(song.dataset.fileID);
  console.log("Removing file " + fileID);
  fileMap.delete(fileID);
  song.remove();
  rebuildAlbumIndex();
}

/**
 * clears the album
 */
function clearAlbum() {
  document.getElementById('album-title').value = "Untitled Album";
  // clear songlist
  let songList = document.getElementById('album-songs');
  songList.innerHTML = '';
  document.getElementById('album-length').innerText = '';
  album_offset = 1;
}

/**
 * Adds the null termination to strings the way SP-1 likes it for album metadata
 */
function terminate(bytes) {
  const term_bytes = new Uint8Array(bytes.length + 1);
  term_bytes.set(bytes);
  return term_bytes;
}


var currProgressTime = 0;

async function printProgress(curr, total, startTime) {
  let pctDone = Math.floor(100 * curr / total);
  let now = Date.now();

  // update every 1/2 second
  if (now < currProgressTime + 500) {
    return;
  }
  currProgressTime = now;
  pctDone = pctDone;

  let duration = (now - startTime) / 1000; // in seconds
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  const durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');

  setProgress(pctDone);
  setProgressText(Math.floor(pctDone) + '% - ' + durationStr);
}

/**
 * Generates the binary data from the WAV files and transfers
 */
async function createBinary() {

  if (deviceMode != DEVICE.TRANSFER_MODE) {
    return;
  }

  let startTime = Date.now();
  currProgress = 0;

  // collect album metadata
  let album = {};
  album.title = document.getElementById('album-title').value.substring(0, STRING_LENGTH - 1);
  if (album.title.length == 0) {
    album.title = "untitled";
  }
  album.length = parseInt(document.getElementById('album-length').innerText);

  album.songs = [];
  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');
  album.numsongs = songs.length;
  for (let el of songs) {
    let song = {};
    song.offset = parseInt(el.getElementsByClassName('song-offset')[0].innerText);
    song.length = parseInt(el.getElementsByClassName('song-length')[0].innerText);
    song.artist = el.getElementsByClassName('song-artist')[0].value.substring(0, STRING_LENGTH - 1);
    song.title = el.getElementsByClassName('song-title')[0].value.substring(0, STRING_LENGTH - 1);
    song.bpm = parseInt(el.getElementsByClassName('song-bpm')[0].value);
    if (song.artist.length == 0) {
      song.artist = "unknown";
    }
    if (song.title.length == 0) {
      song.title = "untitled";
    }
    if (song.bpm == 0) {
      song.bpm = 80;
    }
    song.fileID = Number(el.dataset.fileID);
    album.songs.push(song);
  }

  if (album.songs.length == 0) {
    stemLog("Can't transfer an empty album");
    return;
  }
  if (album.songs.length >= 60) {
    stemLog("Too many songs in album");
    return;
  }

  // create a full sector for metaData
  const metaData = new ArrayBuffer(0x2000);
  const metaDataBytes = new Uint8Array(metaData);
  metaDataBytes.fill(0x58); // fill entire metaData buffer with 'X'
  const view = new DataView(metaData);

  // create magic
  const encoder = new TextEncoder();
  const magic = encoder.encode("ALBUM_PRESENT");
  metaDataBytes.set(magic, OFFSET_MAGIC);
  // album data
  view.setUint32(OFFSET_ALBUM_LEN, album.length, true);
  view.setUint8(OFFSET_NUM_SONGS, album.numsongs, true);


  const albumTitleBytes = terminate(encoder.encode(album.title));
  metaDataBytes.set(albumTitleBytes, OFFSET_ALBUM_TITLE);

  // now start adding songs
  let song_offset = OFFSET_SONGS;
  for (let song of album.songs) {
    view.setUint32(song_offset, song.offset, true);
    view.setUint32(song_offset + 4, song.length, true);
    const artistBytes = terminate(encoder.encode(song.artist));
    metaDataBytes.set(artistBytes, song_offset + 8);
    const titleBytes = terminate(encoder.encode(song.title));
    metaDataBytes.set(titleBytes, song_offset + 8 + STRING_LENGTH);
    song_offset += SONG_LENGTH;
  }

  // reset the write counter
  await sendSP1Cmd('7');

  const writeCMD = 0x39; // '9'
  const CHUNK_SIZE = 128;
  const CHUNKS_PER_SECTOR = 64;
  const PAYLOAD_SIZE = 136;

  let emmcOffset = 0;
  let chunkCounter = 0;
  let albumChunks = album.length * CHUNKS_PER_SECTOR;

  showProgress();


  // send metadata
  for (let offset = 0; offset < SECTOR_SIZE; offset += CHUNK_SIZE) {
    const payload = new Uint8Array(PAYLOAD_SIZE);
    const view = new DataView(payload.buffer);

    view.setUint32(0, chunkCounter, true);  // bytes 0–3, little-endian
    view.setUint32(4, emmcOffset, true);  // bytes 4–7, little-endian
    const dataChunk = metaDataBytes.subarray(offset, offset + CHUNK_SIZE);
    payload.set(dataChunk, 8);

    await sendNoReply(writeCMD, payload, chunkCounter);

    emmcOffset += CHUNK_SIZE;
    chunkCounter++;
  }

  // send song data
  let numSongs = album.songs.length;
  let songNr = 1;
  for (let song of album.songs) {
    markSong(song.fileID); // show this song is being processed
    setProgressTitle("Transfering '" + song.title + "' (" + songNr + "/" + numSongs + ")");
    console.log("Transfering song " + song.title);

    let songBytes = await convertSong(song.fileID, song.bpm);
    if (!songBytes) {
      // conversion failed!
      stemLog("Error converting " + song.title);
      hideProgress();
      return;
    }

    let songLen = songBytes.length;
    for (let offset = 0; offset < songLen; offset += CHUNK_SIZE) {
      const payload = new Uint8Array(PAYLOAD_SIZE);
      const view = new DataView(payload.buffer);

      view.setUint32(0, chunkCounter, true);  // bytes 0–3, little-endian
      view.setUint32(4, emmcOffset, true);  // bytes 4–7, little-endian
      const dataChunk = songBytes.subarray(offset, offset + CHUNK_SIZE);
      payload.set(dataChunk, 8);
      await sendNoReply(writeCMD, payload, chunkCounter);

      emmcOffset += CHUNK_SIZE;
      chunkCounter++;
      await printProgress(offset, songLen, startTime);
    }

    songNr++;
  }
  markSong(-1);
  await printProgress(100, 100, startTime);
  setProgressTitle("Transfering final sector");
  console.log("Done transfering songs, adding final sector");

  // write last sector of album with magic
  {
    const endBuf = new ArrayBuffer(0x2000);
    const endBufBytes = new Uint8Array(endBuf);
    endBufBytes.fill(0x00); // fill last sector with 0 
    endBufBytes.set(magic, 0x2000 - magic.length); // and some magic

    for (let offset = 0; offset < SECTOR_SIZE; offset += CHUNK_SIZE) {
      const payload = new Uint8Array(PAYLOAD_SIZE);
      const view = new DataView(payload.buffer);

      view.setUint32(0, chunkCounter, true);  // bytes 0–3, little-endian
      view.setUint32(4, emmcOffset, true);  // bytes 4–7, little-endian
      const dataChunk = endBufBytes.subarray(offset, offset + CHUNK_SIZE);
      payload.set(dataChunk, 8);
      await sendNoReply(writeCMD, payload, chunkCounter);
      emmcOffset += CHUNK_SIZE;
      chunkCounter++;
    }
  }
  hideProgress();

  let endTime = Date.now();
  let duration = (endTime - startTime) / 1000; // in seconds
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  const durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
  stemLog("Converted album '" + album.title + "' in " + durationStr);
  stemLog(chunkCounter + " chunks");

  // reset to bootloader
  sendSP1Cmd('Q');


  // DOWNLOAD BINARY
  // let binFilename = album.title;
  // binFilename += '.sp1';
  // // export to file
  // try {
  //   const fileHandle = await window.showSaveFilePicker({
  //     suggestedName: binFilename,
  //     types: [{ description: 'Binary', accept: { 'application/octet-stream': ['.sp1'] } }]
  //   });
  //   const writable = await fileHandle.createWritable();
  //   await writable.write(metaData); // can also write in chunks
  //   showProgress();

  //   for (let song of album.songs) {
  //     markSong(song.fileID); // show this song is being processed
  //     setProgressTitle("Converting '" + song.title + "'");
  //     setProgress(0);
  //     let songBytes = await convertSong(song.fileID);
  //     if (!songBytes) {
  //       // conversion failed!
  //       stemLog("Error converting " + song.title);
  //       return;
  //     }
  //     await writable.write(songBytes); // write this song
  //   }
  //   markSong(-1);
  //   // write last sector of album
  //   {
  //     const endBuf = new ArrayBuffer(0x2000);
  //     const endBufBytes = new Uint8Array(endBuf);
  //     endBufBytes.fill(0x00); // fill last sector with 0 
  //     endBufBytes.set(magic, 0x2000 - magic.length); // and some magic
  //     await writable.write(endBuf); // write this song
  //   }
  //   await writable.close();
  // } catch (e) {
  //   hideProgress();
  // }
  // hideProgress();
  // stemLog("Done converting");

}


function showProgress() {
  document.getElementById('progress-bar-container').classList.add('visible');
}

function hideProgress() {
  document.getElementById('progress-bar-container').classList.remove('visible');
}

function setProgress(pct) {
  let pctStr = pct + '%';
  document.getElementById('progress-bar-front').style.width = pctStr;
}

function setProgressText(msg) {
  document.getElementById('progress-text').innerText = msg;
}
function setProgressTitle(msg) {
  document.getElementById('progress-title').innerText = msg;
}

/**
 * Gets a file reference from our fileMap and converts WAV data to SP-1 data
 */
async function convertSong(fileID, bpm) {
  let file = fileMap.get(fileID);
  let wavData;
  let buf;
  try {
    buf = await file.arrayBuffer();
    wavData = parseWAV(buf);
  } catch (err) {
    stemLog(err.message);
    return;
  }
  const { fmt, channels, totalFrames } = wavData;

  let sp1Data;
  try {
    sp1Data = encodeToSP1(channels, totalFrames, bpm);
  } catch (err) {
    stemLog(err.message);
    return false;
  }

  return sp1Data;

}

/** START */
stemLoaderInit();

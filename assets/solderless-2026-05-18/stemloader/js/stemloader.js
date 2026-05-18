/***
 * SP-1 stem loader
 * 
 */

let stemLoaderLog;
let seq = 0;
var numSectorRewrites = 0;

// map to store song data
var nextSongID = 0; // counter for unique song IDs
var songMap = new Map();
var transfer_aborted = false;
var album = {};
album.title = "";
album.length = 1; // current offset in album, songs start at sector 1
album.songs = [];


// Device states
const DEVICE = {
  DISCONNECTED: -2,
  WAIT_FOR_ENUM: -1, // waiting for restarted device to enumerate again
  CONNECTED: 0,
  WAIT_FOR_BOOT: 1,
  BOOT_MODE: 2,
  WAIT_FOR_TRANSFER: 3,
  TRANSFER_MODE: 4,
  TRANSFERRING: 5
}

const CMD = {
  ECHO: 0x01,               // 1 
  CLEAR_FOR_WRITE: 0x37,    // '7' - clears writecounter
  WRITE: 0x39,              // '9'
  GET_WRITE_COUNTER: 0x43,  // 'C'
  EXIT_BOOTLOADER: 0x50,    // 'P'
  RESET: 0x51,              // 'Q'
  GET_DEV_STATE: 0x52,      // 'R'
  GET_ALBUM_TITLE: 0x58,    // 'X'
  GET_DEV_ID: 0x54,         // 'T'
  SET_LEDS: 0x5A,           // 'Z'
  GET_BUTTONS: 0x5C,        // '\' 
  PING: 0x62,               // 'b'
  GET_FADERS: 0x64,         // 'd'
  VERIFY_DATA: 0x66,        // 'f'
  SET_DEV_STATE: 0x70,      // 'p'
  GET_LADDERS: 0x74,        // 't'
  GET_CHARGE_STATE: 0x7a,   // 'z'

  FREQ_SWEEP: 0x56,         // 'V'
  GET_BATT_LEVEL: 0x60,     // '`'

}

const CHUNK_SIZE = 128;
const CHUNKS_PER_SECTOR = 64;
const PAYLOAD_SIZE = 136;

var deviceMode = DEVICE.DISCONNECTED;

function stemLoaderInit() {
  stemLoaderLog = document.getElementById('stem-log');

  setupWAVDropzone();
  loadAlbumFromStorage();
  setupAlbumChangeEvent();

  if (!('serial' in window.navigator)) {
    stemLog('No webserial!');
    hidePanels();
    showWebserialWarning();
    return;
  }

  setDeviceMode(DEVICE.DISCONNECTED);
  setInterval(serialHeartBeat, 5000);
}


/**
 * Reload the album that was already on the page!
 */
function loadAlbumFromStorage() {
  let albumInStorage;
  try {
    albumInStorage = JSON.parse(localStorage.getItem('album'));
  } catch (err) {
    console.log('No valid album in memory');
    return;
  }
  if (!albumInStorage) {
    console.log('No valid album in memory');
    return;
  }
  album = albumInStorage;
  album.length = 1;

  document.getElementById('album-title').value = album.title;

  for (let song of album.songs) {
    albumAddSong(song);
    songMap.set(song.songID, song);
    nextSongID = Math.max(nextSongID, song.songID + 1);
  }
  rebuildAlbumIndex();
}

function setupAlbumChangeEvent() {
  document.getElementById('album-title').addEventListener('change', rebuildAlbumIndex);
}

/**
 * setup events for drag and drop of WAV files
 */
function setupWAVDropzone() {
  let dropOverlay = document.getElementById("wav-drop-overlay");

  window.addEventListener('dragenter', e => {
    e.preventDefault();
    // show our drop overlay
    // only if we have 
    const fileItems = [...e.dataTransfer.items].filter(
      (item) => item.kind === "file",
    );
    if (fileItems.length > 0) {
      dropOverlay.style.display = 'flex';
    }
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
      e.dataTransfer.dropEffect = "copy";
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

    let wavData;
    try {
      const buf = await file.arrayBuffer();
      wavData = wavVerify(buf);
    } catch (err) {
      stemLog('Error for ' + file.name + ': ' + err.message);
      continue;
    }
    const { fmt, totalFrames } = wavData;
    stemLog("+ '" + file.name + "'. Format: " + fmt.bitsPerSample + " bit, " + fmt.sampleRate + " Hz, " + fmt.numChannels + " channels.");

    let songLength = Math.ceil(totalFrames / FRAMES_PER_SECTOR);

    let songMatched = false;

    // check if the current album already contains this song
    for (let song of album.songs) {
      if (!song.file?.name && song.fileName == file.name && song.length == songLength) {
        // found a match!
        if (confirm("Do you want to link the new file to an existing song?\nFile: '" + file.name + "'\nSong: '" + song.title + "'")) {
          // Found match!
          song.file = file;
          let songDiv = document.getElementById('song-' + song.songID);
          songDiv.classList.remove('missing');
          if (song.transferred) {
            // markSongDone(song);
            songDiv.classList.add('done');
          }
          songDiv.getElementsByClassName('song-file')[0].innerText = song.file.name;
          songMatched = true;
        }
      }
    }

    if (!songMatched) {
      // create a new song object and add to songMap
      let newSong = {};
      newSong.offset = 0;
      newSong.transferred = false;
      newSong.fileName = file.name;
      newSong.title = file.name.replace(/\.[^/.]+$/, "");
      newSong.title = newSong.title.replaceAll(/[_-]/g, " ");
      newSong.file = file;
      newSong.artist = "";
      newSong.length = songLength;
      newSong.songID = nextSongID++
      songMap.set(newSong.songID, newSong);
      albumAddSong(newSong);
    }
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
      hideProgress();
      break;
    case DEVICE.CONNECTED:
      stemLog('SP-1 connected');
      break;
    case DEVICE.WAIT_FOR_BOOT:
      showBootWaitPanel();
      stemLog('waiting for boot mode..');
      break;
    case DEVICE.BOOT_MODE:
      showBootPanel();
      stemLog('SP-1 boot mode');
      break;
    case DEVICE.WAIT_FOR_TRANSFER:
      showTransferWaitPanel();
      stemLog('waiting for transfer mode..');
      break;
    case DEVICE.TRANSFER_MODE:
      showTransferPanel();
      break;
    case DEVICE.WAIT_FOR_ENUM:
      showWaitForConnection();
      stemLog('waiting for connection..');
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

function showBootWaitPanel() {
  let panel = document.getElementById('boot-wait-panel');
  panel.style.display = "block";
}

function showTransferWaitPanel() {
  let panel = document.getElementById('transfer-wait-panel');
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
  document.getElementById('album-length').style.display = "inline-block";
}

function hideDebugPanel() {
  let panel = document.getElementById('debug-panel');
  panel.style.display = "none";
}

function toggleDebugPanel() {
  let panel = document.getElementById('debug-panel');
  if (panel.style.display != "block") {
    showDebugPanel();
  } else {
    hideDebugPanel();
  }
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
    await openPort(port);
  } catch (err) {
    stemLog('Connection failed: ' + err.message);
    return;
  }
  // setup serial events
  navigator.serial.addEventListener("disconnect", deviceDisconnectEvent);
  navigator.serial.addEventListener("connect", deviceConnectEvent);
}

/**
 * User requested disconnect from serial port
 */
async function disconnectDevice() {
  if (port == null) {
    return;
  }

  // remove serial event handlers
  navigator.serial.removeEventListener("disconnect", deviceDisconnectEvent);
  navigator.serial.removeEventListener("connect", deviceConnectEvent);

  try {
    await closePort(port);
    await port.forget();
  } catch (err) {
    console.log(err.message);
  }
  setDeviceMode(DEVICE.DISCONNECTED);
}

function deviceDisconnectEvent({ target: port }) {
  setDeviceMode(DEVICE.WAIT_FOR_ENUM);
  closePort(port);
}

function deviceConnectEvent({ target: port }) {
  openPort(port);
}

/**
 * Close a disconnected port
 */
async function closePort(port) {
  writer?.releaseLock();
  await reader?.cancel;
  reader?.releaseLock();
  try {
    await port.close();
  } catch (err) {
    console.log("Failed to close port: " + err.message);
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
  } catch (err) {
    stemLog("port open failed:" + err.message);
    return;
  }

  setDeviceMode(DEVICE.CONNECTED);
  // send a command right away to request device state
  await sendSP1Cmd(CMD.GET_DEV_STATE);
}



async function serialHeartBeat() {
  if (deviceMode == DEVICE.BOOT_MODE || deviceMode == DEVICE.TRANSFER_MODE) {
    await sendEcho();
  }
}

async function sendEcho() {
  let msg = "heartbeat";
  const encoder = new TextEncoder();
  const payload = encoder.encode(msg);

  let resp;
  try {
    resp = await sendCmd(CMD.ECHO, payload, seq++);
  } catch (err) {
    console.log("Failed to send command ECHO " + err.message);
    return;
  }
  let respStr;    // payload as string
  let respStrNum; // payload as number representation string
  let respStrHex; // payload as hex string
  if (!resp) {
    console.log("Echo - Empty response");
    return;
  }
  if (resp.payload) {
    respStr = String.fromCharCode.apply(null, resp.payload);
    respStrNum = resp.payload.join("");
    respStrHex = toHexString(resp.payload);
  }
  if (!resp.crcOk) {
    stemLog("CRC error!");
    return;
  }
  if (resp.cmd == 0xff) {
    stemLog("Error: " + respStr);
    return
  }

  // We received some kind of valid response
  switch (resp.cmd) {
    case 2:
      // 1: echo
      // console.log('Echo: "' + respStr + '"');
      break;
    default:
      stemLog("Echo error: 0x" + resp.cmd.toString(16) + " - '" + respStr + "' " + respStrNum);
      break;
  }
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
 * request and return the write counter
 * This also resets the write counter on device!
 */
async function getWriteCounter() {
  let resp;
  try {
    resp = await sendCmd(CMD.GET_WRITE_COUNTER, new Uint8Array(), seq++);
  } catch (err) {
    console.log("Failed to send command GET_WRITE_COUNTER " + err.message);
    throw (err);
  }
  let respStr; // payload as string
  let respStrNum; // payload as number representation string
  let respStrHex; // payload as hex string
  if (!resp) throw new Error("Empty response - write counter");
  if (!resp.crcOk) throw new Error("CRC error!");
  if (resp.payload) {
    respStr = String.fromCharCode.apply(null, resp.payload);
    respStrNum = resp.payload.join("");
    respStrHex = toHexString(resp.payload);
  }
  if (resp.cmd == 0xff) throw new Error("Error: " + respStr);

  if (resp.cmd == 0x44) { // 'D'
    const buffer = new Uint8Array(resp.payload).buffer;
    let writeCounter = new DataView(buffer).getUint32(0, true);
    return writeCounter;
  }
  return -1;
}

/**
 * Send a command to SP-1 and deal with the response
 */
async function sendSP1Cmd(cmd, payload = new Uint8Array()) {
  // console.log("Send '" + cmd + "' (0x" + cmdChar.toString(16) + "/" + cmdChar + ")");

  let resp;
  try {
    resp = await sendCmd(cmd, payload, seq++);
  } catch (err) {
    stemLog("Failed to send command 0x" + cmd.toString(16));
    throw (err);
  }
  let respStr;    // payload as string
  let respStrNum; // payload as number representation string
  let respStrHex; // payload as hex string
  if (!resp) {
    console.log("Empty response for cmd 0x" + cmd.toString(16));
    return;
  }
  if (resp.payload) {
    respStr = String.fromCharCode.apply(null, resp.payload);
    respStrNum = resp.payload.join("");
    respStrHex = toHexString(resp.payload);
  }

  // console.log("Received '" + String.fromCharCode(resp.cmd) + "' (0x" + resp.cmd.toString(16) + "/" + resp.cmd + ") - " + respStr + ' - ' + respStrNum);

  if (!resp.crcOk) {
    stemLog("CRC error!");
    return;
  }
  if (resp.cmd == 0xff) {
    stemLog("SP-1 error: " + respStr);
    return
  }

  // We received some kind of valid response
  switch (String.fromCharCode(resp.cmd)) {
    case 'S':
      // R: get state request
      if (respStrNum == "00100") {
        // device is in boot mode, auto move to transfer mode
        setDeviceMode(DEVICE.WAIT_FOR_TRANSFER);
        await sendSP1Cmd(CMD.SET_DEV_STATE, new Uint8Array([1]));
        await sendNoReply(CMD.EXIT_BOOTLOADER);

      } else if (respStrNum == "10510") {
        setDeviceMode(DEVICE.TRANSFER_MODE);
        document.getElementById('dev-album').innerText = "";
        // device is in transfer mode, check state and album title
        await sendSP1Cmd(CMD.VERIFY_DATA); // Verify data read
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
      // stemLog("Requested transfer mode OK");
      break;
    case 'U':
      // T: Device id
      stemLog("Device ID: " + respStrHex);
      break;
    case 'Y':
      // X: Album title
      stemLog('Album on device: "' + respStr + '"');
      document.getElementById('dev-album').innerText = "'" + respStr + "'";
      break;
    case 'D':
      // C: Write counter
      {
        const buffer = new Uint8Array(resp.payload).buffer;
        let writeCounter = new DataView(buffer).getUint32(0, true);
        stemLog('Chunks in:  ' + writeCounter);
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

        let devInfo = document.getElementById('dev-info')

        stemLog('Album data ' + (album_is_read ? 'OK' : 'FAIL') + (album_err_num == 0 ? '' : ' - error: ' + album_err_num));
        devInfo.innerText = 'Album data ' + (album_is_read ? 'OK' : 'ERROR') + ' ';
        if (album_is_read) {
          await sendSP1Cmd(CMD.GET_ALBUM_TITLE); // Get album title
        } else {
          document.getElementById('dev-album').innerText = "";
          switch (album_err_num) {
            case ALBUM_ERR.MAGIC_NOT_FOUND:
              stemLog('Album not present');
              devInfo.innerHTML += 'Album not present';
              break;
            case ALBUM_ERR.ALBUM_TOO_SHORT:
              stemLog('Album too short');
              devInfo.innerHTML += 'Album too short';
              break;
            case ALBUM_ERR.ALBUM_TOO_LONG:
              stemLog('Album too long');
              devInfo.innerHTML += 'Album too long';
              break;
            case ALBUM_ERR.TOO_MANY_SONGS:
              stemLog('Too many songs');
              devInfo.innerHTML += 'Too many songs';
              break;
            case ALBUM_ERR.ALBUM_TITLE_TOO_LONG:
              stemLog('Album title too long');
              devInfo.innerHTML += 'Album title too long';
              break;
            case ALBUM_ERR.ALBUM_TITLE_ALLOC_FAILED:
              stemLog('Album title alloc fail');
              devInfo.innerHTML += 'Album title error';
              break;
            case ALBUM_ERR.SONG_INDEX_ALLOC_FAILED:
              stemLog('Song index alloc fail');
              devInfo.innerHTML += 'Song index error';
              break;
            case ALBUM_ERR.SONG_OFFSET_INVALID:
              stemLog('Invalid song offset');
              devInfo.innerHTML += 'Invalid song offset';
              break;
            case ALBUM_ERR.SONG_LENGTH_TOO_LARGE:
              stemLog('Song too large');
              devInfo.innerHTML += 'Song too large';
              break;
            case ALBUM_ERR.SONG_EXCEEDS_ALBUM:
              stemLog('Song exceeds album');
              devInfo.innerHTML += 'Song exceeds album';
              break;
            case ALBUM_ERR.ARTIST_NAME_TOO_LONG:
              stemLog('Artist name too long');
              devInfo.innerHTML += 'Artist name too long';
              break;
            case ALBUM_ERR.ARTIST_NAME_ALLOC_FAILED:
              stemLog('Artist name alloc fail');
              devInfo.innerHTML += 'Artist name error';
              break;
            case ALBUM_ERR.SONG_TITLE_TOO_LONG:
              stemLog('Song title too long');
              devInfo.innerHTML += 'Song title too long';
              break;
            case ALBUM_ERR.SONG_TITLE_ALLOC_FAILED:
              stemLog('Song title alloc fail');
              devInfo.innerHTML += 'Song title error';
              break;
            case ALBUM_ERR.MAGIC_NOT_FOUND_AT_END:
              stemLog('No magic at end');
              devInfo.innerHTML += 'No magic at end';
              break;
            default:
              stemLog('unknown data error');
              devInfo.innerHTML += 'unknown data error';
              break;
          }
        }
      }
      break;
    case '{':
      // z: battery status
      {
        let temperature = resp.payload[0];
        let chg_status = resp.payload[1];
        stemLog('Temperature:     ' + temperature);
        stemLog('Charging status: ' + chg_status);
        stemLog('- Charging: ' + ((chg_status & 0b0001) ? "Enabled" : "Disabled"));
        stemLog('- Measured: ' + ((chg_status & 0b0010) ? "On" : "Off"));
        stemLog('- Power:    ' + ((chg_status & 0b0100) ? "Good" : "No good"));
        stemLog('- ISET:     ' + ((chg_status & 0b1000) ? "On" : "High-Z"));
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
    case 'a':
      // returns battery level
      {
        const buffer = new Uint8Array(resp.payload).buffer;
        let batt_mv = new DataView(buffer).getUint32(0, true);
        stemLog('Battery level:  ' + (batt_mv / 1000).toFixed(1) + " V");
      }
      break;
    case 'W':
      // test tone
      stemLog("Generated tone frequency sweep");
      break;
    default:
      console.log("Unknown response to 0x" + cmd.toString(16) + ": 0x" + resp.cmd.toString(16) + " - '" + respStr + "' " + respStrNum + " " + respStrHex);
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
function albumAddSong(song) {

  song.offset = album.length; // this will need to be updated each time we move around

  // song length is expressed in sectors
  const duration = Math.floor((song.length * FRAMES_PER_SECTOR) / 48000);
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  let durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');


  if (album.length + song.length + 1 >= MAX_SECTORS) {
    stemLog("Can't add song - not enough memory on SP-1");
    return;
  }
  let numSectorsHex = "0x" + song.length.toString(16);

  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');
  song.nr = songs.length + 1;
  let songOffsetHex = "0x" + song.offset.toString(16);
  album.length += song.length;

  let songDiv = document.createElement('div');
  songDiv.dataset.songID = song.songID;
  songDiv.classList.add('song');
  songDiv.setAttribute('id', 'song-' + song.songID);
  if (song.file?.name) {
    song.bpm = extractBPM(song.file.name);
  }

  songDiv.innerHTML = `
  <div class="song-top">
    <div class="song-nr">${song.nr}</div>
    <div><input type="text" class="song-title" placeholder="" value="" maxlength="57"/></div>
    <div><input type="text" class="song-artist" placeholder="" value="" maxlength="57"/></div>
    <div class="song-duration">${durationStr}</div>
    <div><input type="number" min="30" max="300" class="song-bpm" placeholder="" value="" step="0.01"/></div>
    <div class="song-delete"></div>
  </div>
  <div class="song-meta">
   <div class="song-meta-row">
      <div class="label">file</div>
      <div class="song-file"></div>
    </div>
    <div class="song-meta-row">
      <div class="label">offset</div>
      <div class="song-offset">${songOffsetHex}</div>
    </div>
    <div class="song-meta-row">
      <div class="label">sectors</div>
      <div class="song-length">${numSectorsHex}</div>
    </div>
      <div class="song-meta-row transfer-row">
      <div class="label">transfer</div>
      <div class="">
        <input type="button" class="song-transfer dark-button" value="Transfer song">
        <input type="button" class="song-new dark-button" title="Song will not be skipped on next transfer"value="Mark new">
        <input type="button" class="song-done dark-button" title="Song will be skipped on next transfer" value="Mark done">
      </div>
    </div>
  </div>
  `;

  if (song.file?.name) {
    songDiv.getElementsByClassName('song-file')[0].innerText = song.file.name;
  } else {
    songDiv.getElementsByClassName('song-file')[0].innerText = 'file missing after page reload';
    songDiv.classList.add('missing');
  }
  songDiv.getElementsByClassName('song-title')[0].value = song.title;
  songDiv.getElementsByClassName('song-artist')[0].value = song.artist;
  songDiv.getElementsByClassName('song-bpm')[0].value = song.bpm;

  let songNr = songDiv.getElementsByClassName('song-nr')[0];
  songNr.onclick = () => { selectSong(song.songID) };

  let songClose = songDiv.getElementsByClassName('song-delete')[0];
  songClose.onclick = () => { albumRemoveSong(song.songID) };

  let songTransfer = songDiv.getElementsByClassName('song-transfer')[0];
  songTransfer.onclick = () => { transferSong(song, true) };

  let btnNew = songDiv.getElementsByClassName('song-new')[0];
  btnNew.onclick = () => { markSongNew(song) };

  let btnDone = songDiv.getElementsByClassName('song-done')[0];
  btnDone.onclick = () => { markSongDone(song) };

  let songFields = songDiv.getElementsByTagName('input');
  for (let field of songFields) {
    field.addEventListener('change', rebuildAlbumIndex);
  }
  songList.appendChild(songDiv);
}

/**
 * Update album and song data
 */
function rebuildAlbumIndex() {
  album.length = 1;

  const albumField = document.getElementById('album-title');
  album.title = albumField.value.substring(0, 57);
  albumField.value = album.title;
  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');
  if (songs.length == 0) {
    nextSongID = 0;
  }

  let songNR = 1;
  album.songs = [];
  for (let el of songs) {
    let song = songMap.get(Number(el.dataset.songID));
    if (song.offset != album.length && song.transferred == true) {
      // this song was transferred, but changed place in memory! mark it!
      markSongTodo(song);
    }
    song.offset = album.length;
    song.nr = songNR;
    song.artist = el.getElementsByClassName('song-artist')[0].value.substring(0, 57);
    song.title = el.getElementsByClassName('song-title')[0].value.substring(0, 57);
    let bpmField = el.getElementsByClassName('song-bpm')[0];
    song.bpm = Math.min(Math.max(30, parseFloat(bpmField.value)), 300);
    if (!Number.isFinite(song.bpm)) {
      song.bpm = 80;
    }
    bpmField.value = song.bpm;

    let songOffsetHex = "0x" + song.offset.toString(16);
    el.getElementsByClassName('song-nr')[0].innerText = song.nr;
    el.getElementsByClassName('song-offset')[0].innerText = songOffsetHex;
    album.length += song.length;
    album.songs.push(song);
    songNR++;
  }

  document.getElementById('album-length').innerText = '0x' + (album.length + 1).toString(16);

  // album size in bytes, including the metadata and end sector
  let albumSize = (album.length + 1) * SECTOR_SIZE;
  let unit = "B";
  if (albumSize > 1000) {
    albumSize /= 1000;
    unit = "KB";
  }
  if (albumSize > 1000) {
    albumSize /= 1000;
    unit = "MB";
  }
  if (albumSize > 1000) {
    albumSize /= 1000;
    unit = "GB";
  }
  document.getElementById('album-size').innerText = albumSize.toFixed(1) + unit + " / 4GB";

  // album.length = number of sectors + 1 metadata sector (not inculding album end sector!)
  const duration = Math.floor((FRAMES_PER_SECTOR * (album.length - 1)) / 48000);
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  let durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
  document.getElementById('album-duration').innerText = durationStr;

  // show transfer button if we have songs
  let transferButton = document.getElementById('transfer-button');
  if (songs.length > 0) {
    transferButton.classList.remove('inactive');
  } else {
    transferButton.classList.add('inactive');
  }

  // store the album
  localStorage.setItem('album', JSON.stringify(album));
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
 * Mark a song to show it failed transfer
 */
function markSongTodo(song) {
  song.transferred = false;
  let songDiv = document.getElementById('song-' + song.songID);
  if (songDiv) {
    songDiv.classList.remove('done');
    songDiv.classList.add('todo');
  }
  rebuildAlbumIndex();
}

/**
 * Mark a song as completely transferred
 */
function markSongDone(song) {
  song.transferred = true;
  let songDiv = document.getElementById('song-' + song.songID);
  if (songDiv) {
    songDiv.classList.remove('todo');
    songDiv.classList.add('done');
  }
  rebuildAlbumIndex();
}

/**
 * Remove done and todo mark, like a new song
 */
function markSongNew(song) {
  song.transferred = false;
  let songDiv = document.getElementById('song-' + song.songID);
  if (songDiv) {
    songDiv.classList.remove('todo');
    songDiv.classList.remove('done');
  }
  rebuildAlbumIndex();
}

/**
 * Mark a song as missing
 */
function markSongMissing(song) {
  song.transferred = false;
  let songDiv = document.getElementById('song-' + song.songID);
  if (songDiv) {
    songDiv.classList.remove('todo');
    songDiv.classList.remove('done');
    songDiv.getElementsByClassName('song-file')[0].innerText = 'file missing.. was the file moved or renamed?';
    songDiv.classList.add('missing');
  }
}


/**
 * Remove a song from the album
 */
function albumRemoveSong(songID) {
  let song = document.getElementById('song-' + songID);
  songMap.delete(songID);
  song.remove();
  rebuildAlbumIndex();
}

/**
 * clears the album
 */
function clearAlbum() {
  document.getElementById('album-title').value = "Untitled Album";
  // clear song list
  let songList = document.getElementById('album-songs');
  songList.innerHTML = '';
  document.getElementById('album-length').innerText = '';
  album.length = 1;
  songMap.clear();
  nextSongID = 0;
}




var currProgressTime = 0;

async function printProgress(curr, total, startTime) {
  let pctDone = Math.floor(100 * curr / total);
  let now = Date.now();

  // update max every 1 second
  if (now < currProgressTime + 1000) {
    return;
  }
  currProgressTime = now;

  let duration = Math.floor((now - startTime) / 1000); // in seconds
  const minutes = Math.floor(duration / 60);
  const seconds = Math.ceil(duration - minutes * 60);
  const durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
  const speedStr = (duration > 0) ? " - " + (curr / (1000 * duration)).toFixed() + " KBps" : "";
  setProgress(pctDone);

  let timeLeftStr = "";
  if (curr > SECTOR_SIZE) {
    // make an estimate of time left after a full sector?
    let timeLeft = Math.floor((total - curr) * duration / curr);
    const minutesLeft = Math.floor(timeLeft / 60);
    const secondsLeft = Math.ceil(timeLeft - minutesLeft * 60);
    if (minutesLeft) {
      timeLeftStr = " (~" + minutesLeft.toString() + "m left)";
    } else if (secondsLeft) {
      timeLeftStr = " (~" + secondsLeft.toString() + "s left)";
    }
  }
  setProgressText(Math.floor(pctDone) + '% - ' + durationStr + speedStr + timeLeftStr);
}

/**
 * Generates the binary data from the WAV files and transfers
 */
async function transferAlbum(downloadBinary = false) {

  if (deviceMode != DEVICE.TRANSFER_MODE && !downloadBinary) {
    return;
  }

  let startTime = Date.now();
  numSectorRewrites = 0;
  rebuildAlbumIndex();

  if (!downloadBinary) {
    await transferMetaData();

    // TRANSFER  SONGS
    for (let song of album.songs) {
      if (song.transferred) {
        stemLog("Skipping '" + song.title + "'. (already transferred)")
        continue;
      }
      try {
        await transferSong(song);
      } catch (err) {
        stemLog('Failed transfer: ' + err.message);
        break;
      }
    }

    let endTime = Date.now();
    let duration = Math.floor((endTime - startTime) / 1000); // in seconds
    const hours = Math.floor(duration / 3600);
    const minutes = Math.floor((duration % 3600) / 60);
    const seconds = Math.floor(duration % 60);
    let durationStr = (hours > 0) ? (hours.toString().padStart(2, '0') + 'h ') : '';
    durationStr += minutes.toString().padStart(2, '0') + 'm ';
    durationStr += seconds.toString().padStart(2, '0') + 's';
    stemLog("Transferred album '" + album.title + "' in " + durationStr);
    if (album.length) {
      console.log("Total sector rewrites: " + numSectorRewrites + " (" + (100 * numSectorRewrites / album.length).toFixed(2) + "%)");
    }
    hideProgress();
    setDeviceMode(DEVICE.WAIT_FOR_BOOT);
    await sendNoReply(CMD.RESET); // reset to bootloader

  } else {
    // DOWNLOAD BINARY
    let binFilename = album.title + '.sp1';

    // export to file
    try {
      const fileHandle = await window.showSaveFilePicker({
        suggestedName: binFilename,
        types: [{ description: 'Binary', accept: { 'application/octet-stream': ['.sp1'] } }]
      });
      const writable = await fileHandle.createWritable();
      // await writable.write(metaData); // for testing, we don't need metadata
      showProgress();

      for (let song of album.songs) {
        markSongTodo(song); // show this song is being processed
        setProgressTitle("Converting '" + song.title + "'");
        setProgress(0);
        let songBytes = await convertSong(song);
        if (!songBytes) {
          stemLog("Error converting " + song.title);
          await writable.abort();
          hideProgress();
          return;
        }
        await writable.write(songBytes); // write this song
      }
      // write last sector of album
      {
        const albumEndBytes = createAlbumEnd();
        await writable.write(albumEndBytes);
      }
      await writable.close();
    } catch (err) {
      hideProgress();
    }
  }

  hideProgress();

}

async function transferMetaData(resetAtEnd = false) {
  setProgressTitle("Transferring metadata");
  stemLog("Transferring metadata");

  let metaData = createMetaData();
  if (!metaData) {
    console.log('Failed to create metadata');
    return;
  }

  try {
    await transferData(metaData, 0);
  } catch (err) {
    stemLog("Error transferring metadata: " + err.message);
    setDeviceMode(DEVICE.WAIT_FOR_BOOT);
    hideProgress();
    await sendNoReply(CMD.RESET);
    return;
  }

  // write last sector of album with magic
  let albumEndSector = album.length;
  let albumEndBytes = createAlbumEnd();
  if (!albumEndBytes) {
    console.log('Failed to create end of album');
    return;
  }
  stemLog("Transferring album end");
  try {
    await transferData(albumEndBytes, albumEndSector);
  } catch (err) {
    stemLog("Error transferring last sector: " + err.message);
    setDeviceMode(DEVICE.WAIT_FOR_BOOT);
    await sendNoReply(CMD.RESET);
    return;
  }

  if (resetAtEnd) {
    hideProgress();
    setDeviceMode(DEVICE.WAIT_FOR_BOOT);
    // reset to bootloader
    await sendNoReply(CMD.RESET);
  }
}

/**
 * Sleep, but don't allow throttling if the tab goes in background!
 * !! Don't do long busySleeps - blocks UI !!
 * @param {*} millis 
 */
function busySleep(millis) {
  const end = performance.now() + millis;
  while (performance.now() < end) {
    // sleeping!
  }
}

/**
 * Generic transfer of data to device
 * @param {*} data : Uint8Array
 * @param {*} sectorOffset : offset in SP-1 memory, number of sectors
 * @returns 
 */
async function transferData(data, sectorOffset) {

  if (deviceMode != DEVICE.TRANSFER_MODE && deviceMode != DEVICE.TRANSFERRING) {
    return;
  }

  // Write to serial port
  // reset the write counter
  try {
    await sendSP1Cmd(CMD.CLEAR_FOR_WRITE);
  } catch (err) {
    stemLog('Failed to reset write counter: ' + err.message);
    throw (err);
  }

  let songOffset = sectorOffset * SECTOR_SIZE; // position to start writing to in eMMC
  let chunkCounter = 0;
  let numRetries = 0;
  let dataLen = data.length;

  let startTime = Date.now();
  setDeviceMode(DEVICE.TRANSFERRING);
  showProgress();
  await printProgress(0, dataLen, startTime);


  // send data
  for (let offset = 0; offset < dataLen; offset += CHUNK_SIZE) {
    const payload = new Uint8Array(PAYLOAD_SIZE);
    const view = new DataView(payload.buffer);

    // test every sector and retry if needed
    if (chunkCounter == CHUNKS_PER_SECTOR) {
      await printProgress(offset, dataLen, startTime);
      // requesting the write counter from device also resets it!
      let writeCounter = await getWriteCounter();

      if (writeCounter != chunkCounter) {
        let currSector = Math.floor(offset / SECTOR_SIZE);
        let seconds = Math.floor(currSector * FRAMES_PER_SECTOR / 48000);
        let minutes = Math.floor(seconds / 60);
        seconds = seconds - (minutes * 60);
        const timeStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
        let pct = (100 * offset / dataLen).toFixed();
        console.log(" " + timeStr + " Dropped packet at sector 0x" + currSector.toString(16) + " - " + writeCounter + "/" + chunkCounter);
        if (numRetries < 3) {
          offset -= SECTOR_SIZE;
          numRetries++;
          numSectorRewrites++;
          console.log(' 0x' + currSector.toString(16) + ' - retry #' + numRetries);
          await sendSP1Cmd(CMD.CLEAR_FOR_WRITE);
        } else {
          throw new Error('Failed to transfer sector after 3 attempts!');
        }
      } else {
        // transmission OK, reset retry counter
        numRetries = 0;
      }
      chunkCounter = 0; // reset chunkCounter
    }

    chunkCounter++;
    view.setUint32(0, chunkCounter, true);  // bytes 0–3, little-endian
    view.setUint32(4, songOffset + offset, true);  // bytes 4–7, little-endian
    const dataChunk = data.subarray(offset, offset + CHUNK_SIZE);
    payload.set(dataChunk, 8);

    await sendNoReply(CMD.WRITE, payload, seq++);
    if (chunkCounter % 4 == 0) {
      // wait a little after transferring 4 chunks to avoid dropped packets?
      busySleep(3);
    }

    if (transfer_aborted) {
      transfer_aborted = false;
      hideProgress();
      setDeviceMode(DEVICE.TRANSFER_MODE);
      throw new Error('Transfer was aborted!');
    }
  }

  let endTime = Date.now();
  let duration = Math.floor((endTime - startTime) / 1000); // in seconds
  if (duration > 0) {
    const minutes = Math.floor(duration / 60);
    const seconds = Math.ceil(duration - minutes * 60);
    const durationStr = minutes.toString().padStart(2, '0') + ':' + seconds.toString().padStart(2, '0');
    stemLog("Transferred " + (dataLen / SECTOR_SIZE).toFixed() + " sectors in " + durationStr + " (" + (dataLen / (1000 * duration)).toFixed(1) + " KBps)");
  }
}

/**
 * Adds the null termination to strings the way SP-1 likes it for album metadata
 */
function terminate(bytes) {
  const term_bytes = new Uint8Array(bytes.length + 1);
  term_bytes.set(bytes);
  return term_bytes;
}

function createMetaData() {

  // make sure all data is up to date
  rebuildAlbumIndex();

  // collect album metadata
  if (album.title.length == 0) {
    album.title = "untitled";
  }

  // SP-1 serial comms fails with album titles longer than 57!
  album.title = album.title.substring(0, 57);

  album.songs = [];
  let songList = document.getElementById('album-songs');
  let songs = songList.getElementsByClassName('song');
  for (let el of songs) {
    let song = songMap.get(Number(el.dataset.songID));
    if (song.title.length == 0) {
      song.title = "untitled";
    }
    if (!Number.isFinite(song.bpm)) {
      song.bpm = 80;
    }
    album.songs.push(song);
  }

  if (album.songs.length == 0) {
    stemLog("Can't transfer an empty album");
    return null;
  }
  if (album.songs.length > 60) {
    stemLog("Too many songs in album");
    return null;
  }
  if (album.length >= MAX_SECTORS) {
    stemLog("Album too big for memory");
    return null;
  }

  // collected all metadata.

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
  view.setUint32(OFFSET_ALBUM_LEN, album.length + 1, true);
  view.setUint8(OFFSET_NUM_SONGS, album.songs.length);

  const albumTitleBytes = terminate(encoder.encode(album.title));
  metaDataBytes.set(albumTitleBytes, OFFSET_ALBUM_TITLE);

  // now start adding songs
  let song_offset = OFFSET_SONGS;
  for (let song of album.songs) {
    view.setUint32(song_offset, song.offset, true);
    view.setUint32(song_offset + 4, song.length, true);
    let songArtist = "" + song.artist;
    if (songArtist.length == 0) {
      songArtist = "unknown";
    }
    const artistBytes = terminate(encoder.encode(songArtist));
    metaDataBytes.set(artistBytes, song_offset + 8);
    const titleBytes = terminate(encoder.encode(song.title));
    metaDataBytes.set(titleBytes, song_offset + 8 + STRING_LENGTH);
    song_offset += SONG_INDEX_LENGTH;
  }

  return metaDataBytes;
}

function createAlbumEnd() {
  // create magic
  const encoder = new TextEncoder();
  const magic = encoder.encode("ALBUM_PRESENT");
  const endBuf = new ArrayBuffer(SECTOR_SIZE);
  const albumEndBytes = new Uint8Array(endBuf);
  albumEndBytes.fill(0x00); // fill last sector with 0 
  albumEndBytes.set(magic, SECTOR_SIZE - magic.length); // and some magic
  return albumEndBytes;
}

/**
 * transfer a song to devie
 * @param {*} song song object
 * @returns 
 */
async function transferSong(song, resetAtEnd = false) {

  if (!song) {
    console.log("Song not found *_*");
    return;
  }
  if (!song.file.name) {
    stemLog("Skipping '" + song.title + "'. (file missing)");
    return;
  }
  // Send song data
  markSongTodo(song); // show this song is being processed
  setProgressTitle("Transferring '" + song.title + "' (" + song.nr + "/" + album.songs.length + ")");
  stemLog("Transferring " + song.nr + ". '" + song.title + "'");
  console.log("Transferring " + song.nr + ". '" + song.title + "' - offset: 0x" + song.offset.toString(16) + ' length: 0x' + song.length.toString(16));


  let songBytes = await convertSong(song);
  if (!songBytes) {
    stemLog("Error converting " + song.title);
    hideProgress();
    setDeviceMode(DEVICE.TRANSFER_MODE);
    return;
  }

  try {
    await transferData(songBytes, song.offset);
  } catch (err) {
    stemLog("Error transferring song: " + err.message);
    hideProgress();
    setDeviceMode(DEVICE.TRANSFER_MODE);
    throw err;
  }
  stemLog("Done transferring '" + song.title + "'");
  markSongDone(song);

  if (resetAtEnd) {
    await transferMetaData(true);
  }
}

function abortTransfer() {
  if (confirm("Are you sure you want to abort the transfer?")) {
    transfer_aborted = true;
  }
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
 * Gets a song songMap and converts WAV data to SP-1 data
 */
async function convertSong(song) {

  if (!song) {
    console.log("Song not found *_*");
    return null;
  }
  if (!song.file) {
    console.log("Song file not found");
    return null;
  }
  let file = song.file;
  let buf;
  try {
    buf = await file.arrayBuffer();
  } catch (err) {
    stemLog("File read failed:" + err.message);
    markSongMissing(song);
    return null;
  }

  let wavData;
  try {
    wavData = parseWAV(buf);
  } catch (err) {
    stemLog("WAV parsing failed:" + err.message);
    return null;
  }

  const { fmt, channels, totalFrames } = wavData;

  let sp1Data;
  try {
    sp1Data = encodeToSP1(channels, totalFrames, song.bpm);
  } catch (err) {
    stemLog(err.message);
    return null;
  }

  return sp1Data;

}

/** START */
stemLoaderInit();

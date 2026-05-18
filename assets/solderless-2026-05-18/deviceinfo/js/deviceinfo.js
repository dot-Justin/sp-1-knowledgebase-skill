/**
 * SP-1 device info — queries all readable device state.
 */

const $ = id => document.getElementById(id);
const statusEl = $('status');
const btnRefresh = $('btnRefresh');
const btnTransfer = $('btnTransfer');
const btnReboot = $('btnReboot');
let seq = 1;

function setStatus(msg, cls) {
  statusEl.textContent = msg;
  statusEl.className = 'status' + (cls ? ' ' + cls : '');
}

function setVal(id, text) {
  const el = $(id);
  el.textContent = text;
  el.classList.toggle('dim', text === '—');
}

function resetAll() {
  ['valDeviceId','valMode','valAlbum','valAlbumData','valWriteCounter',
   'valTemp','valCharging','valPower','valFaders','valButtons','valLadders'
  ].forEach(id => setVal(id, '—'));
}

function toHex(byteArray) {
  return Array.from(byteArray, b => ('0' + (b & 0xFF).toString(16)).slice(-2)).join('');
}

async function cmd(charCode, payload = new Uint8Array()) {
  return await sendCmd(charCode, payload, seq++, 3000);
}

let queryRetries = 0;
const MAX_RETRIES = 5;

async function queryAll() {
  setStatus('querying device...', '');
  resetAll();

  try {
    // Get device state (R → S)
    const rState = await cmd(0x52);
    if (!rState || !rState.crcOk) {
      if (++queryRetries < MAX_RETRIES) {
        setStatus('retrying (' + queryRetries + '/' + MAX_RETRIES + ')...', '');
        await sleep(1000);
        return queryAll();
      }
      setStatus('device not responding', 'err');
      return;
    }
    queryRetries = 0;

    const stateStr = String.fromCharCode(...rState.payload);
    const stateNum = rState.payload.join('');
    let mode = 'unknown (' + stateNum + ')';
    let inTransferMode = false;
    if (stateNum === '00100') { mode = 'boot mode'; btnTransfer.disabled = false; }
    else if (stateNum === '10510') { mode = 'transfer mode'; inTransferMode = true; btnTransfer.disabled = true; }
    setVal('valMode', mode);
    btnReboot.disabled = false;

    // Device ID (T → U)
    const rId = await cmd('T'.charCodeAt(0));
    if (rId && rId.crcOk && rId.cmd !== 0xff) {
      setVal('valDeviceId', toHex(rId.payload));
    }

    if (inTransferMode) {
      // Album data check (f → g)
      const rVerify = await cmd('f'.charCodeAt(0));
      if (rVerify && rVerify.crcOk && rVerify.payload.length >= 2) {
        const ok = rVerify.payload[0];
        const errNum = rVerify.payload[1];
        setVal('valAlbumData', ok ? 'ok' : 'error ' + errNum);
      }

      // Album title (X → Y)
      const rAlbum = await cmd('X'.charCodeAt(0));
      if (rAlbum && rAlbum.crcOk && rAlbum.cmd !== 0xff) {
        setVal('valAlbum', String.fromCharCode(...rAlbum.payload));
      }

      // Write counter (C → D)
      const rCounter = await cmd('C'.charCodeAt(0));
      if (rCounter && rCounter.crcOk && rCounter.payload.length >= 4) {
        setVal('valWriteCounter', readLE32(rCounter.payload, 0).toString());
      }

      // Battery (z → {)
      const rBatt = await cmd('z'.charCodeAt(0));
      if (rBatt && rBatt.crcOk && rBatt.payload.length >= 2) {
        const temp = rBatt.payload[0];
        const chg = rBatt.payload[1];
        setVal('valTemp', temp + ' °C');
        setVal('valCharging', (chg & 0x01) ? 'yes' : 'no');
        setVal('valPower', (chg & 0x04) ? 'good' : 'no power');
      }

      // Faders (d → e)
      const rFaders = await cmd('d'.charCodeAt(0));
      if (rFaders && rFaders.crcOk && rFaders.payload.length >= 4) {
        const p = rFaders.payload;
        setVal('valFaders', 'T1:' + p[0] + '  T2:' + p[1] + '  T3:' + p[2] + '  T4:' + p[3]);
      }

      // Buttons (\ → ])
      const rButtons = await cmd('\\'.charCodeAt(0));
      if (rButtons && rButtons.crcOk) {
        setVal('valButtons', rButtons.payload.join(' '));
      }

      // Ladders (t → u)
      const rLadders = await cmd('t'.charCodeAt(0));
      if (rLadders && rLadders.crcOk && rLadders.payload.length >= 4) {
        const ladders = new Uint16Array(new Uint8Array(rLadders.payload).buffer);
        setVal('valLadders', ladders[0] + '  ' + ladders[1]);
      }
    }

    setStatus('last queried: ' + new Date().toLocaleTimeString(), 'ok');
  } catch (e) {
    if (++queryRetries < MAX_RETRIES) {
      setStatus('retrying (' + queryRetries + '/' + MAX_RETRIES + ')...', '');
      await sleep(1000);
      return queryAll();
    }
    setStatus('query failed: ' + e.message, 'err');
  }
}

async function enterTransferMode() {
  setStatus('entering transfer mode...', '');
  btnTransfer.disabled = true;
  try {
    await cmd('p'.charCodeAt(0), new Uint8Array([1]));
    await sleep(200);
    await cmd('P'.charCodeAt(0));
    await sleep(2000);
    queryRetries = 0;
    queryAll();
  } catch (e) {
    setStatus('failed: ' + e.message, 'err');
  }
}

async function rebootDevice() {
  setStatus('rebooting...', '');
  btnReboot.disabled = true;
  btnTransfer.disabled = true;
  try {
    await cmd('Q'.charCodeAt(0));
  } catch (e) {}
}

// Auto-query when parent signals connection
async function onConnected() {
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });
    reader = port.readable.getReader();
    writer = port.writable.getWriter();
    btnRefresh.disabled = false;
    btnTransfer.disabled = false;
    btnReboot.disabled = false;
    queryRetries = 0;
    queryAll();
    startLive();
  } catch (e) {
    setStatus('connection failed: ' + e.message, 'err');
  }
}

// ─── Live monitor ─────────────────────────────────────────────────────────────

const livePanel = $('livePanel');
let liveInterval = null;
let liveRunning = false;
let liveBusy = false;

// Fader knob Y range within the track (track y=1174.355, h=371.674)
const FADER_Y_TOP = 1174.355 + 29.36;
const FADER_Y_BOT = 1174.355 + 371.674 - 29.36;

function setKnobPos(idx, value) {
  const knob = document.getElementById('knob' + idx);
  if (!knob) return;
  const cy = FADER_Y_TOP + (value / 255) * (FADER_Y_BOT - FADER_Y_TOP);
  knob.setAttribute('cy', cy.toFixed(2));
}

const BTN_MAP = {
  0: 'btnFwd',
  1: 'btnVolUp',
  2: 'btnRwd',
  3: 'btnVolDown',
  4: 'btnT4',
  5: 'btnT3',
  6: 'btnT2',
  7: 'btnT1',
  8: 'btnPlay',
  9: 'btnFunc'
};

// Track button array index → LED index (T1=led0, T2=led1, T3=led2, T4=led3)
const BTN_TO_LED = { 7: 0, 6: 1, 5: 2, 4: 3 };

function setAllButtons(payload) {
  // Reset all
  Object.values(BTN_MAP).forEach(id => {
    const el = document.getElementById(id);
    if (!el) return;
    const tag = el.tagName.toLowerCase();
    if (tag === 'rect' || tag === 'path') {
      el.setAttribute('fill', 'none');
      el.setAttribute('stroke', '#ccc');
    } else if (tag === 'polygon' || tag === 'circle') {
      el.setAttribute('fill', '#ddd');
    }
  });
  for (let i = 0; i < 4; i++) {
    const led = document.getElementById('led' + i);
    if (led) { led.setAttribute('fill', 'none'); led.setAttribute('stroke', '#ddd'); }
  }

  // Set pressed
  for (let i = 0; i < payload.length; i++) {
    if (!payload[i]) continue;
    const elId = BTN_MAP[i];
    if (!elId) continue;
    const el = document.getElementById(elId);
    if (!el) continue;
    const tag = el.tagName.toLowerCase();
    if (tag === 'rect' || tag === 'path') {
      el.setAttribute('fill', '#1a1a1a');
      el.setAttribute('stroke', '#1a1a1a');
    } else if (tag === 'polygon' || tag === 'circle') {
      el.setAttribute('fill', '#1a1a1a');
    }
    // Light corresponding LED for track buttons
    if (i in BTN_TO_LED) {
      const led = document.getElementById('led' + BTN_TO_LED[i]);
      if (led) { led.setAttribute('fill', '#5ec45e'); led.setAttribute('stroke', '#5ec45e'); }
    }
  }
}

async function pollLive() {
  if (!writer || !reader || liveBusy) return;
  liveBusy = true;

  try {
    const rFaders = await cmd('d'.charCodeAt(0));
    if (rFaders && rFaders.crcOk && rFaders.payload.length >= 4) {
      for (let i = 0; i < 4; i++) setKnobPos(i, rFaders.payload[3 - i]);
    }

    const rButtons = await cmd('\\'.charCodeAt(0));
    if (rButtons && rButtons.crcOk) {
      const p = rButtons.payload;
      const parts = [];
      for (let i = 0; i < p.length; i++) parts.push(i + ':' + p[i]);
      $('btnRaw').textContent = parts.join(' ');
      setAllButtons(p);
    }

    const rLadders = await cmd('t'.charCodeAt(0));
    if (rLadders && rLadders.crcOk && rLadders.payload.length >= 4) {
      const ladders = new Uint16Array(new Uint8Array(rLadders.payload).buffer);
      $('ladder0').textContent = 'L1: ' + ladders[0];
      $('ladder1').textContent = 'L2: ' + ladders[1];
    }
  } catch (e) {
    stopLive();
  }

  liveBusy = false;
}

function startLive() {
  liveRunning = true;
  livePanel.classList.add('active');
  pollLive();
  liveInterval = setInterval(pollLive, 100);
}

function stopLive() {
  liveRunning = false;
  liveBusy = false;
  livePanel.classList.remove('active');
  if (liveInterval) { clearInterval(liveInterval); liveInterval = null; }
}

// ─── Init ─────────────────────────────────────────────────────────────────────

btnRefresh.addEventListener('click', queryAll);
btnTransfer.addEventListener('click', enterTransferMode);
btnReboot.addEventListener('click', rebootDevice);

if ('serial' in navigator) {
  setStatus('waiting for connection...', '');
  window.addEventListener('serial-proxy-connect', onConnected);
  window.addEventListener('serial-proxy-disconnect', function() {
    stopLive();
    btnRefresh.disabled = true;
    btnTransfer.disabled = true;
    btnReboot.disabled = true;
  });
  window.parent.postMessage({ type: 'serial-query-state' }, '*');
  window.addEventListener('message', function initCheck(e) {
    if (e.data && e.data.type === 'serial-state' && e.data.state === 'connected') {
      window.removeEventListener('message', initCheck);
      onConnected();
    }
  });
} else {
  setStatus('web serial not supported — use chrome or edge', 'err');
}

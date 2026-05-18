(function() {
/**
 * SP-1 Doom — runs Doom via wasm-doom WASM binary, polls SP-1 for input.
 * wasm-doom DOOM class inlined to avoid ES module / CORS issues on file://.
 */

var statusEl = document.getElementById('status');
var canvas = document.getElementById('doomCanvas');
var ctx = canvas.getContext('2d');
var faderDisplay = document.getElementById('faderDisplay');
var btnDisplay = document.getElementById('btnDisplay');

var doomInstance = null;
var _seq = 1;
var polling = false;
var pollGen = 0;

function setStatus(msg, cls) {
  statusEl.textContent = msg;
  statusEl.className = 'status' + (cls ? ' ' + cls : '');
}

/* ── Doom internal key codes ── */
var DKEY = {
  UP: 173, DOWN: 175, LEFT: 172, RIGHT: 174,
  FIRE: 157, USE: 32, RUN: 182, STRAFE: 184,
  MAP: 9, ENTER: 13, ESC: 27,
  COMMA: 44, PERIOD: 46,
  NUM1: 49, NUM2: 50, NUM3: 51, NUM4: 52,
  NUM5: 53, NUM6: 54, NUM7: 55,
};

/* ── Browser keyCode → Doom keyCode mapping (from wasm-doom) ── */
function mapBrowserKeyToDoom(k) {
  switch (k) {
    case 8:  return 127;
    case 17: return 157;
    case 18: return 184;
    case 37: return 172;
    case 38: return 173;
    case 39: return 174;
    case 40: return 175;
    default:
      if (k >= 65 && k <= 90) return k + 32;
      if (k >= 112 && k <= 123) return k + 75;
      return k;
  }
}

/* ── Fader → key mapping ── */
var DEAD_LO = 10;
var DEAD_HI = 235;
var keyState = {};

function sendKey(code, pressed) {
  if (doomInstance) {
    doomInstance.instance.exports.add_browser_event(pressed ? 0 : 1, code);
  }
}

function setKey(code, pressed) {
  if (!!keyState[code] === pressed) return;
  keyState[code] = pressed;
  sendKey(code, pressed);
}

function mapFaders(t1, t2, t3, t4) {
  // T1 (F1): forward / backward
  setKey(DKEY.UP,   t1 < DEAD_LO);
  setKey(DKEY.DOWN, t1 > DEAD_HI);

  // T2 (F2): weapon select (7 slots across 0-255)
  if (t2 < 37) setWeapon(1);
  else if (t2 < 73)  setWeapon(2);
  else if (t2 < 110) setWeapon(3);
  else if (t2 < 146) setWeapon(4);
  else if (t2 < 183) setWeapon(5);
  else if (t2 < 219) setWeapon(6);
  else setWeapon(7);

  // T3 (F3): strafe left / right
  setKey(DKEY.COMMA,  t3 > DEAD_HI);
  setKey(DKEY.PERIOD, t3 < DEAD_LO);

  // T4 (F4): turn left / turn right
  setKey(DKEY.LEFT,  t4 > DEAD_HI);
  setKey(DKEY.RIGHT, t4 < DEAD_LO);
}

var currentWeapon = 0;
function setWeapon(n) {
  if (n === currentWeapon) return;
  currentWeapon = n;
  var code = DKEY.NUM1 + (n - 1);
  sendKey(code, true);
  setTimeout(function() { sendKey(code, false); }, 50);
}

function mapButtons(payload) {
  setKey(DKEY.FIRE,  payload.length > 2 && payload[2] !== 0);  // btn 2: fire trigger
  setKey(DKEY.USE,   payload.length > 5 && payload[5] !== 0);  // btn 6: use/open
  setKey(DKEY.RUN,   payload.length > 6 && payload[6] !== 0);  // btn 7: run
  setKey(DKEY.ESC,   payload.length > 7 && payload[7] !== 0);  // btn 8: menu
  setKey(DKEY.ENTER, payload.length > 8 && payload[8] !== 0);  // btn 9: confirm
}

/* ── SP-1 mode transition (boot → transfer) ── */
async function ensureTransferMode() {
  for (var attempt = 0; attempt < 5; attempt++) {
    var r = await sendCmd('R'.charCodeAt(0), new Uint8Array(), _seq++, 500);
    if (!r || !r.crcOk) { await sleep(200); continue; }
    var state = Array.from(r.payload).join('');
    console.log('SP-1 state: ' + state);
    if (state === '10510') return true;
    if (state === '00100') {
      await sendCmd('p'.charCodeAt(0), new Uint8Array([1]), _seq++, 500);
      await sleep(300);
      await sendCmd('P'.charCodeAt(0), new Uint8Array(), _seq++, 500);
      await sleep(500);
      continue;
    }
    await sleep(300);
  }
  return false;
}

/* ── SP-1 polling loop ── */
async function pollLoop() {
  polling = true;
  var myGen = ++pollGen;

  if (!modeChecked) {
    var ready = await ensureTransferMode();
    if (myGen !== pollGen) return;
    if (!ready) console.warn('could not reach transfer mode, polling anyway');
    modeChecked = true;
  }

  while (myGen === pollGen) {
    try {
      var rFaders = await sendCmd('d'.charCodeAt(0), new Uint8Array(), _seq++, 300);
      if (myGen !== pollGen) return;
      if (rFaders && rFaders.crcOk && rFaders.payload.length >= 4) {
        var p = rFaders.payload;
        mapFaders(p[0], p[1], p[2], p[3]);
        faderDisplay.innerHTML =
          '<span>T1:' + p[0] + '</span><span>T2:' + p[1] +
          '</span><span>T3:' + p[2] + '</span><span>T4:' + p[3] + '</span>';
      }

      var rButtons = await sendCmd('\\'.charCodeAt(0), new Uint8Array(), _seq++, 300);
      if (myGen !== pollGen) return;
      if (rButtons && rButtons.crcOk) {
        mapButtons(rButtons.payload);
        btnDisplay.textContent = 'buttons: ' + Array.from(rButtons.payload).join(' ');
      }
    } catch (e) {
      console.warn('poll error:', e);
    }

    await sleep(60);
  }
}

function stopPolling() {
  polling = false;
  pollGen++;
  modeChecked = false;
}

var modeChecked = false;

/* ── WASM Doom loader (inlined from wasm-doom, no ES module needed) ── */
var DOOM_W = 640;
var DOOM_H = 400;

async function loadDoomWasm(wasmUrl) {
  var memory = new WebAssembly.Memory({ initial: 108 });
  var noop = function() {};

  function render(ptr) {
    var buf = new Uint8ClampedArray(memory.buffer, ptr, DOOM_W * DOOM_H * 4);
    var img = new ImageData(buf, DOOM_W, DOOM_H);
    ctx.putImageData(img, 0, 0);
  }

  var importObj = {
    js: {
      js_console_log: noop,
      js_stdout: noop,
      js_stderr: noop,
      js_draw_screen: render,
      js_milliseconds_since_start: function() { return performance.now(); },
    },
    env: { memory: memory },
  };

  var response = await fetch(wasmUrl);
  var bytes = await response.arrayBuffer();
  return await WebAssembly.instantiate(bytes, importObj);
}

/* ── Doom initialization ── */
var doomReady = false;

async function initDoom() {
  if (doomReady) return;
  setStatus('loading spoom1...', '');

  try {
    doomInstance = await loadDoomWasm('./wasm/doom.wasm');
    doomReady = true;

    document.addEventListener('keydown', function(e) {
      doomInstance.instance.exports.add_browser_event(0, mapBrowserKeyToDoom(e.keyCode));
      e.preventDefault();
    });
    document.addEventListener('keyup', function(e) {
      doomInstance.instance.exports.add_browser_event(1, mapBrowserKeyToDoom(e.keyCode));
      e.preventDefault();
    });

    doomInstance.instance.exports.main();

    function gameLoop() {
      doomInstance.instance.exports.doom_loop_step();
      window.requestAnimationFrame(gameLoop);
    }
    window.requestAnimationFrame(gameLoop);

    setStatus('spoom1 loaded — use SP-1 to play', 'ok');
  } catch (e) {
    setStatus('failed to load spoom1: ' + e.message, 'err');
    console.error(e);
  }
}

/* ── Serial connection handling ── */
var serialConnected = false;
var serialSetup = false;
var tabActive = false;

async function setupSerial() {
  if (serialSetup) return true;
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });
    reader = port.readable.getReader();
    writer = port.writable.getWriter();
    serialSetup = true;
    return true;
  } catch (e) {
    setStatus('serial setup failed: ' + e.message, 'err');
    return false;
  }
}

async function activateDoom() {
  if (!serialConnected) return;
  if (!serialSetup) {
    if (!await setupSerial()) return;
  }
  if (!doomReady) await initDoom();
  else setStatus('spoom1 loaded — use SP-1 to play', 'ok');
  pollLoop();
}

function onConnected() {
  serialConnected = true;
  setStatus('connected — click spoom1 tab to play', 'ok');
  if (tabActive) activateDoom();
}

function onDisconnected() {
  serialConnected = false;
  serialSetup = false;
  stopPolling();
  Object.keys(keyState).forEach(function(k) {
    if (keyState[k]) { keyState[k] = false; sendKey(parseInt(k), false); }
  });
  setStatus('disconnected', 'err');
}

function onTabActivated() {
  tabActive = true;
  if (serialConnected) activateDoom();
}

function onTabDeactivated() {
  tabActive = false;
  stopPolling();
}

if ('serial' in navigator) {
  setStatus('waiting for connection...', '');

  window.addEventListener('message', function(e) {
    if (!e.data || !e.data.type) return;
    if (e.data.type === 'tab-activated') onTabActivated();
    if (e.data.type === 'tab-deactivated') onTabDeactivated();
    if (e.data.type === 'serial-state' && e.data.state === 'connected' && !serialConnected) {
      onConnected();
    }
  });

  window.addEventListener('serial-proxy-connect', function() {
    if (!serialConnected) onConnected();
  });
  window.addEventListener('serial-proxy-disconnect', onDisconnected);

  window.parent.postMessage({ type: 'serial-query-state' }, '*');
} else {
  setStatus('web serial not supported — use chrome or edge', 'err');
}

})();

/**
 * Firmware flash logic and UI events.
 * Depends on: protocol.js (loaded first)
 */

let fwData = null;

function fwSetProgress(pct, text) {
  $('progFill').style.width = pct + '%';
  $('progPct').textContent = text || (pct.toFixed(0) + '%');
}

// ─── FW: Events ─────────────────────────────────────────────────────────────

$('btnModeFW').addEventListener('click', () => setState('fw-step1'));
$('btnFwBack1').addEventListener('click', () => { fwData = null; $('fileInput').value = ''; setState('home'); });

$('btnFile').addEventListener('click', () => $('fileInput').click());
$('fileInput').addEventListener('change', async (e) => {
  const file = e.target.files[0];
  if (!file) return;
  fwData = new Uint8Array(await file.arrayBuffer());
  const maxSize = FLASH_END - FLASH_START;
  if (fwData.length === 0 || fwData.length > maxSize) {
    $('fileLabel').textContent = fwData.length === 0 ? 'file is empty.' : 'file too large.';
    fwData = null;
    return;
  }
  const numPages = Math.ceil(fwData.length / FW_PAGE_SIZE);
  $('fileLabel').textContent = `${file.name} — ${(fwData.length / 1024).toFixed(1)} kb, ${numPages} pages`;
  logMsg(`selected: ${file.name} (${fwData.length} bytes)`);
  setState('fw-step2');
});

$('btnChangeFile').addEventListener('click', () => {
  fwData = null;
  $('fileInput').value = '';
  setState('fw-step1');
});

$('btnConnectFlash').addEventListener('click', async () => {
  if (!fwData) return;
  if (!('serial' in navigator)) {
    logMsg('webserial not supported. use chrome or edge.', 'err');
    $('errText').textContent = 'webserial is not supported in this browser. use chrome or edge.';
    setState('fw-error');
    return;
  }
  try {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });
    reader = port.readable.getReader();
    writer = port.writable.getWriter();
    logMsg('connected.');
  } catch (e) {
    logMsg('connection failed: ' + e.message, 'err');
    $('errText').textContent = 'connection failed. make sure device is in bootloader mode.';
    setState('fw-error');
    return;
  }
  setState('fw-flashing');
  const ok = await flashFirmware(fwData);
  await closeSerialPort();
  if (ok) {
    $('doneText').innerHTML = 'update complete.<br>press the function button to restart your device.';
    setState('fw-done');
  } else {
    setState('fw-error');
  }
});

$('btnFwDone').addEventListener('click', () => {
  fwData = null;
  $('fileInput').value = '';
  setState('home');
});

$('btnFwRetry').addEventListener('click', () => {
  if (fwData) setState('fw-step2');
  else setState('fw-step1');
});

// ─── FW: Flash logic ────────────────────────────────────────────────────────

async function flashFirmware(fw) {
  const fwSize = fw.length;
  const numPages = Math.ceil(fwSize / FW_PAGE_SIZE);
  let seq = 1;

  $('flashText').textContent = 'checking device...';
  fwSetProgress(0, 'connecting');
  logMsg('phase 0: status (R)');

  const r0 = await sendCmd(0x52, new Uint8Array(), seq++, 3000);
  if (!r0 || !r0.crcOk || r0.cmd !== 0x53) {
    logMsg('device did not respond to R.', 'err');
    $('errText').textContent = 'device not responding. check connection.';
    return false;
  }
  if (r0.payload.length >= 4) logMsg(`  pages: ${readLE32(r0.payload, 0)}`);

  $('flashText').textContent = 'preparing device...';
  fwSetProgress(2, 'erasing');
  logMsg('phase 1: format (F)');

  const r1 = await sendCmd(0x46, new Uint8Array(), seq++, 5000);
  if (!r1 || !r1.crcOk || r1.cmd !== 0x47) {
    logMsg('format failed.', 'err');
    $('errText').textContent = 'format command failed.';
    return false;
  }
  logMsg('  format ok.');

  $('flashText').textContent = 'writing firmware...';
  logMsg(`phase 2: writing ${fwSize} bytes, ${numPages} pages`);

  let eSeq = 0, totalChunks = 0;
  for (let p = 0; p < numPages; p++)
    totalChunks += Math.ceil(Math.min(FW_PAGE_SIZE, fwSize - p * FW_PAGE_SIZE) / FW_CHUNK_SIZE);

  let chunksSent = 0;

  for (let pgIdx = 0; pgIdx < numPages; pgIdx++) {
    const pageNum = FIRST_PAGE + pgIdx;
    const pageBase = pgIdx * FW_PAGE_SIZE;
    const pageDataLen = Math.min(FW_PAGE_SIZE, fwSize - pageBase);

    for (let offset = 0; offset < pageDataLen; offset += FW_CHUNK_SIZE) {
      eSeq++;
      const chunkLen = Math.min(FW_CHUNK_SIZE, pageDataLen - offset);
      const addr = FLASH_START + pageBase + offset;
      const data = fw.slice(pageBase + offset, pageBase + offset + chunkLen);
      await sendNoReply(0x45, new Uint8Array([...le32(eSeq), ...le32(addr), ...data]), seq++);
      chunksSent++;

      const pct = 5 + (chunksSent / totalChunks) * 85;
      fwSetProgress(pct, Math.round(pct) + '%');
      $('flashText').textContent = `writing page ${pgIdx + 1} of ${numPages}...`;

      if (offset === 0 && pgIdx > 0) await sleep(100);
      else await sleep(5);
    }
  }
  logMsg(`  sent ${eSeq} chunks.`);

  $('flashText').textContent = 'committing...';
  fwSetProgress(92, '92%');
  logMsg('phase 3: commit (H)');

  await sleep(150);
  const lastPage = FIRST_PAGE + numPages - 1;
  const r3 = await sendCmd(0x48, le32(lastPage), seq++, 5000);
  if (!r3 || !r3.crcOk || r3.cmd !== 0x49) {
    logMsg('commit failed.', 'err');
    $('errText').textContent = 'commit failed. try again.';
    return false;
  }

  let counterVal = eSeq;
  if (r3.payload.length >= 4) {
    counterVal = readLE32(r3.payload, 0);
    logMsg(`  counter: ${counterVal}`);
  }

  $('flashText').textContent = 'finalizing...';
  fwSetProgress(96, '96%');
  logMsg('phase 4: finalize (H)');

  const r4 = await sendCmd(0x48, le32(counterVal), seq++, 5000);
  if (!r4 || !r4.crcOk) {
    logMsg('finalize failed.', 'err');
    $('errText').textContent = `finalize failed. retry with H(${counterVal}).`;
    return false;
  }

  if (r4.cmd === 0x49) {
    if (r4.payload.length >= 4) logMsg(`  finalize counter: ${readLE32(r4.payload, 0)}`);
    logMsg('firmware update complete.', 'ok');
    fwSetProgress(100, 'complete');
    return true;
  } else {
    logMsg(`unexpected response: ${hex(r4.cmd)}`, 'err');
    $('errText').textContent = 'unexpected response during finalize.';
    return false;
  }
}

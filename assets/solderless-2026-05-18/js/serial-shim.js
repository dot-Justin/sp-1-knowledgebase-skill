/**
 * Serial shim for iframe-embedded apps.
 *
 * When loaded inside an iframe whose parent runs the serial proxy,
 * this replaces navigator.serial with a postMessage-based bridge.
 * When loaded standalone (top-level window), it's a no-op — the
 * real Web Serial API is used directly.
 *
 * Usage: load this script BEFORE the app's protocol script
 * (protocol.js or sp-1_protocol.js).
 */
(function() {
  var inIframe = (window !== window.top);
  if (!inIframe) return;

  var rxQueue = [];
  var rxResolve = null;
  var portOpen = false;
  var connected = false;
  var txId = 0;
  var txPending = {};

  window.addEventListener('message', function(e) {
    var msg = e.data;
    if (!msg || !msg.type) return;

    if (msg.type === 'serial-rx') {
      var chunk = new Uint8Array(msg.data);
      if (rxResolve) {
        var r = rxResolve;
        rxResolve = null;
        r({ value: chunk, done: false });
      } else {
        rxQueue.push(chunk);
      }
    }

    if (msg.type === 'serial-tx-ack') {
      var resolve = txPending[msg.txId];
      if (resolve) {
        delete txPending[msg.txId];
        resolve();
      }
    }

    if (msg.type === 'serial-state') {
      var wasConnected = connected;
      connected = (msg.state === 'connected');
      if (wasConnected && !connected) {
        rxQueue = [];
        if (rxResolve) {
          var r = rxResolve;
          rxResolve = null;
          r({ value: undefined, done: true });
        }
        Object.keys(txPending).forEach(function(id) {
          txPending[id]();
          delete txPending[id];
        });
        window.dispatchEvent(new CustomEvent('serial-proxy-disconnect'));
      }
      if (!wasConnected && connected) {
        window.dispatchEvent(new CustomEvent('serial-proxy-connect'));
      }
    }
  });

  var fakeReader = {
    read: function() {
      if (rxQueue.length > 0) {
        return Promise.resolve({ value: rxQueue.shift(), done: false });
      }
      return new Promise(function(resolve) { rxResolve = resolve; });
    },
    cancel: function() {
      if (rxResolve) {
        var r = rxResolve;
        rxResolve = null;
        r({ value: undefined, done: true });
      }
      return Promise.resolve();
    },
    releaseLock: function() {}
  };

  var fakeWriter = {
    write: function(data) {
      var arr = Array.from(new Uint8Array(data));
      var id = txId++;
      return new Promise(function(resolve) {
        txPending[id] = resolve;
        window.parent.postMessage({ type: 'serial-tx', data: arr, txId: id }, '*');
      });
    },
    releaseLock: function() {}
  };

  var fakePort = {
    open: function() {
      portOpen = true;
      return Promise.resolve();
    },
    close: function() {
      portOpen = false;
      return Promise.resolve();
    },
    forget: function() { return Promise.resolve(); },
    readable: { getReader: function() { return fakeReader; } },
    writable: { getWriter: function() { return fakeWriter; } }
  };

  var disconnectListeners = [];
  var connectListeners = [];

  window.addEventListener('serial-proxy-disconnect', function() {
    disconnectListeners.forEach(function(fn) {
      try { fn({ target: fakePort }); } catch(e) { console.error(e); }
    });
  });

  window.addEventListener('serial-proxy-connect', function() {
    connectListeners.forEach(function(fn) {
      try { fn({ target: fakePort }); } catch(e) { console.error(e); }
    });
  });

  Object.defineProperty(navigator, 'serial', {
    configurable: true,
    get: function() {
      return {
        requestPort: function() {
          window.parent.postMessage({ type: 'serial-request-port' }, '*');
          return new Promise(function(resolve, reject) {
            function onState(e) {
              if (e.data && e.data.type === 'serial-state') {
                if (e.data.state === 'connected') {
                  window.removeEventListener('message', onState);
                  connected = true;
                  resolve(fakePort);
                }
              }
            }
            if (connected) {
              resolve(fakePort);
            } else {
              window.addEventListener('message', onState);
              setTimeout(function() {
                window.removeEventListener('message', onState);
                reject(new Error('Connection timed out'));
              }, 60000);
            }
          });
        },
        addEventListener: function(type, fn) {
          if (type === 'disconnect') disconnectListeners.push(fn);
          if (type === 'connect') connectListeners.push(fn);
        },
        removeEventListener: function(type, fn) {
          if (type === 'disconnect') {
            disconnectListeners = disconnectListeners.filter(function(f) { return f !== fn; });
          }
          if (type === 'connect') {
            connectListeners = connectListeners.filter(function(f) { return f !== fn; });
          }
        }
      };
    }
  });

  window.parent.postMessage({ type: 'serial-query-state' }, '*');
})();

#pragma once
#include <pgmspace.h>
// Generated from web-preview.html by embed_web.py.
static const char WEB_PAGE[] PROGMEM = R"WEBUI(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>PandaFlasher preview</title>
  <style>
    body{font:16px system-ui;margin:0;min-height:100vh;display:flex;flex-direction:column;background:#121821;color:#eee}
    header,footer{padding:1rem max(1rem,calc((100% - 48rem)/2));background:#202b39}
    header{display:flex;align-items:center;justify-content:space-between;gap:1rem;flex-wrap:wrap;position:sticky;top:0;z-index:1}
    header h1{display:flex;align-items:center;gap:.65rem;margin:0}
    header h1 svg{width:2rem;height:2rem;flex:none}
    nav{display:flex;gap:.25rem;flex-wrap:wrap}
    nav a{padding:.5rem;color:#eee;text-decoration:none;border-radius:.4rem}
    nav a:hover,nav a:focus-visible{background:#344458}
    .toolbar{display:flex;align-items:center;gap:.75rem;flex-wrap:wrap;padding:.75rem max(1rem,calc((100% - 48rem)/2));background:#1b2532}
    [hidden]{display:none!important}
    main{width:min(48rem,calc(100% - 2rem));margin:1rem auto;flex:1}
    main article{display:none}
    main article:target,main:not(:has(article:target)) #home{display:block}
    article{scroll-margin-top:5rem}
    footer a{display:inline-flex;align-items:center;gap:.5rem;color:#eee;text-decoration:none}
    footer a:hover{text-decoration:underline}
    footer svg{width:1.1rem;height:1.1rem}
    footer span{margin-left:1.5rem}
    a{color:#8cf}
    section{background:#202b39;padding:1rem;margin:1rem 0;border-radius:.7rem}
    section h3{margin-top:0}
    .device-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(13rem,1fr));gap:1rem}
    .device-card{position:relative;margin:0;padding-right:3.5rem}
    .device-card.selected{outline:2px solid #68b6ff;background:#283a50}
    .device-select{position:absolute;top:1rem;right:1rem;width:1.8rem;height:1.8rem;padding:0;margin:0;border:2px solid #b8c6d6;border-radius:50%;background:transparent;color:#102030}
    .device-select svg{display:none;width:1.2rem;height:1.2rem;margin:auto}
    .device-card.selected .device-select{background:#68b6ff;border-color:#68b6ff}
    .device-card.selected .device-select svg{display:block}
    .device-card p{margin:.4rem 0}
    #selection-actions{position:sticky;bottom:0;z-index:1}
    table{width:100%;border-collapse:collapse}
    th,td{padding:.5rem 0;vertical-align:top}
    th{width:40%;text-align:left;font-weight:500;color:#b8c6d6}
    .files th:last-child,.files td:last-child{text-align:right;width:1%}
    .files button{display:inline-flex;align-items:center;justify-content:center}
    .files button svg{width:1.1rem;height:1.1rem}
    dialog{width:min(24rem,calc(100% - 2rem));border:1px solid #43536b;border-radius:.7rem;background:#202b39;color:#eee;padding:1.25rem}
    dialog::backdrop{background:#0009}
    dialog h3{margin-top:0}
    dialog form{display:flex;justify-content:flex-end;gap:.5rem}
    dialog button[value="delete"],dialog button[value="flash"]{background:#b93030;color:#fff;border:0;border-radius:.3rem}
    #flash-progress{max-height:12rem;overflow:auto}
    .flash-target{margin:.75rem 0}
    .flash-target progress{display:block;width:100%}
    #flash-log{box-sizing:border-box;width:100%;resize:vertical;background:#121821;color:#eee;border:1px solid #43536b;border-radius:.3rem;padding:.5rem;font:12px ui-monospace,monospace}
    input,select,button{font:inherit;padding:.45rem;margin:.25rem}
    button{cursor:pointer}
    pre{white-space:pre-wrap}
  </style>
</head>
<body>
  <header>
    <h1>
      <svg viewBox="0 0 94.22 84.59" fill="currentColor" aria-hidden="true" focusable="false">
        <path d="M49.8,82.35c-13.89-0.61-24.22-2.52-32.84-9.2C5.21,64.03,4.23,51.28,6.19,37.81c0.2-1.35,0.45-2.75,0.99-3.99c1.38-3.2,0.63-5.87-1.15-8.77C1.22,17.21,3,9.01,10.12,4.87c4.09-2.38,8.43-2.67,12.55-0.51c4.5,2.35,8.85,2.55,13.73,1.35c7.24-1.79,14.53-1.39,21.91,0.04c3.17,0.61,6.92-0.14,10.03-1.29c4.16-1.53,8.12-2.55,12.32-0.79c8.36,3.5,11.18,13.01,5.96,20.95c-1.67,2.54-1.77,4.58-0.74,7.26c2.96,7.67,3.21,15.63,1.7,23.62c-2.11,11.2-9.13,18.63-19.45,22.42C61.31,80.43,53.93,81.39,49.8,82.35z M47.12,9.2C47.12,9.2,47.21,9.53,47.12,9.2c-6.09,0.6-12.11,1.3-17.34,3.86C14.59,20.46,7.05,38.09,10.96,54.87c2.31,9.9,8.41,16.07,17.86,19.39c12.57,4.41,25.05,4.01,37.43-0.46c6.18-2.23,11.08-6.21,14.08-12.2C90.09,42.09,78.86,8.93,47.12,9.2z"/>
        <path d="M63.1,33.44c9.15-0.03,18.66,10.3,17.91,19.45c-0.36,4.43-2.13,7.94-6.5,9.62c-4.37,1.68-8.29,0.36-11.34-2.7c-3.23-3.25-6.11-6.91-8.76-10.66c-2.28-3.24-2.81-7.11-0.72-10.62C55.71,35.13,59.01,33.36,63.1,33.44z M62.12,47.3c2.61-0.33,4.29-1.64,4.07-4.35c-0.21-2.64-2.21-4.32-4.53-3.73c-1.54,0.39-3.37,2.5-3.7,4.12C57.42,46.04,59.64,47.13,62.12,47.3z"/>
        <path d="M30.04,34.25c8.08,0.08,12.5,7.38,8.28,14.31c-4.52,6.21-10.38,17.04-19.6,12.9C5.11,54.41,17.89,33.64,30.04,34.25z M35.21,43.4c1.23-5.76-5.19-6.37-7.86-2.27C25.08,46.46,34.09,47.81,35.21,43.4z"/>
        <path d="M46.61,61.29c-3.72,0-6.59-2.22-6.69-5.17c-0.11-3.1,2.87-5.55,6.73-5.54c3.74,0.01,6.57,2.22,6.69,5.2C53.45,58.93,50.57,61.29,46.61,61.29z"/>
        <path d="M46.75,71.28c-3.33-1.65-6.19-2.94-8.89-4.51c-0.55-0.32-0.52-1.66-0.75-2.53c0.87,0.05,2-0.22,2.57,0.22c4.73,3.6,9.31,3.58,14-0.07c0.55-0.43,1.69-0.09,2.55-0.11c-0.18,0.77-0.11,1.97-0.58,2.25C52.87,68.18,49.95,69.61,46.75,71.28z"/>
      </svg>
      PandaFlasher
    </h1>
    <nav aria-label="Main navigation">
      <a href="#home">Home</a>
      <a href="#filemanager">Filemanager</a>
      <a href="#info">Info</a>
      <a href="#settings">Settings</a>
    </nav>
  </header>
  <div id="home-toolbar" class="toolbar">
    <button id="identify" type="button">Identify</button>
  </div>
  <main>
  <p id="notice" role="status" hidden></p>
  <article id="home">
    <p id="scan-message">Press Identify to find connected devices.</p>
    <div id="device-grid" class="device-grid" aria-live="polite"></div>
  </article>

  <article id="filemanager">
    <h2>Filemanager</h2>
    <section>
      <h3>Upload to SD card</h3>
      <p>Upload a firmware .bin file from your computer to the SD card. It will then be available for flashing.</p>
      <form id="upload-form">
        <input id="upload-file" type="file" accept=".bin" aria-label="Firmware file to upload" required>
        <button>Upload</button>
      </form>
    </section>
    <section>
      <h3>Files on SD card</h3>
      <table class="files">
        <thead><tr><th scope="col">Firmware file</th><th scope="col">Action</th></tr></thead>
        <tbody id="file-list">
          <tr><td>target-full-flash.bin</td><td><button type="button" class="delete-file" aria-label="Delete target-full-flash.bin" title="Delete target-full-flash.bin"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M3 6h18M8 6V4h8v2m3 0-1 14H6L5 6m5 4v7m4-7v7"/></svg></button></td></tr>
          <tr><td>panda-ota.bin</td><td><button type="button" class="delete-file" aria-label="Delete panda-ota.bin" title="Delete panda-ota.bin"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M3 6h18M8 6V4h8v2m3 0-1 14H6L5 6m5 4v7m4-7v7"/></svg></button></td></tr>
          <tr><td>extension-ota.bin</td><td><button type="button" class="delete-file" aria-label="Delete extension-ota.bin" title="Delete extension-ota.bin"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M3 6h18M8 6V4h8v2m3 0-1 14H6L5 6m5 4v7m4-7v7"/></svg></button></td></tr>
        </tbody>
      </table>
    </section>
    <dialog id="delete-dialog" aria-labelledby="delete-title">
      <h3 id="delete-title">Delete firmware file?</h3>
      <p>Delete <strong id="delete-filename"></strong> from the SD card?</p>
      <form method="dialog">
        <button value="cancel" autofocus>Cancel</button>
        <button value="delete">Delete file</button>
      </form>
    </dialog>
  </article>

  <article id="settings">
    <h2>Settings</h2>
    <section>
      <h3>Wi-Fi</h3>
      <form id="ap-name-form">
        <label for="ap-ssid">Setup access point name</label>
        <input id="ap-ssid" name="apSsid" required minlength="1" maxlength="32">
        <button>Save name</button>
      </form>
      <form>
        <button type="button" data-op="wifi">Change Wi-Fi network</button>
        <button type="button" data-op="ap">Restart in AP mode</button>
      </form>
    </section>
    <section>
      <h3>Power</h3>
      <form>
        <button type="button" data-op="reboot">Restart PandaFlasher</button>
        <button type="button" data-op="extension-restart">Restart PandaExtension</button>
      </form>
    </section>
    <h2>Firmware update</h2>
    <section>
      <h3>PandaFlasher</h3>
      <p>Select a PandaFlasher ESP32-S3 OTA update file (.bin) from the SD card. Use an application image, not a merged image.</p>
      <form>
        <select id="self-file" name="file" aria-label="PandaFlasher firmware file" required>
          <option value="panda-ota.bin">panda-ota.bin</option>
        </select>
        <button type="button" data-op="self">Update PandaFlasher</button>
      </form>
    </section>
    <section id="extension-update">
      <h3>PandaExtension</h3>
      <p>Select a PandaExtension ESP32-C3 OTA update file (.bin) from the SD card. Use an application image.</p>
      <form>
        <select id="extension-file" name="file" aria-label="PandaExtension firmware file" required>
          <option value="extension-ota.bin">extension-ota.bin</option>
        </select>
        <button type="button" data-op="extension">Update PandaExtension</button>
      </form>
    </section>
  </article>

  <article id="info">
    <h2>Info</h2>
    <section>
      <h3>Device</h3>
      <table>
        <tbody>
          <tr><th scope="row">Device name</th><td id="info-name">panda-1a2b3c.local</td></tr>
          <tr><th scope="row">Firmware version</th><td id="info-version">v2.2.0-dev</td></tr>
          <tr><th scope="row">Uptime</th><td id="info-uptime">2 h 14 min</td></tr>
        </tbody>
      </table>
    </section>
    <section id="extension-info">
      <h3>PandaExtension</h3>
      <table>
        <tbody>
          <tr><th scope="row">Status</th><td>Connected</td></tr>
          <tr><th scope="row">Firmware version</th><td id="info-extension-version">v1.0.0</td></tr>
          <tr><th scope="row">Uptime</th><td id="info-extension-uptime">37 min</td></tr>
          <tr><th scope="row">MAC address</th><td id="info-extension-mac">AA:BB:CC:4D:5E:6F</td></tr>
        </tbody>
      </table>
    </section>
    <section>
      <h3>Network</h3>
      <table>
        <tbody>
          <tr><th scope="row">Wi-Fi network</th><td id="info-ssid">Workshop</td></tr>
          <tr><th scope="row">IP address</th><td id="info-ip">192.168.1.42</td></tr>
          <tr><th scope="row">MAC address</th><td id="info-mac">AA:BB:CC:1A:2B:3C</td></tr>
          <tr><th scope="row">Wi-Fi signal</th><td id="info-signal">-58 dBm (good)</td></tr>
        </tbody>
      </table>
    </section>
  </article>
  </main>
  <div id="selection-actions" class="toolbar" hidden>
    <strong id="selected-target"></strong>
    <button id="reset-button" type="button">Reset selected</button>
    <select id="flash-file" aria-label="Full firmware image to flash">
      <option value="target-full-flash.bin">target-full-flash.bin</option>
    </select>
    <button id="flash-button" type="button">Flash</button>
  </div>
  <dialog id="flash-dialog" aria-labelledby="flash-title">
    <h3 id="flash-title">Flash selected devices?</h3>
    <p id="flash-warning">Flashing will erase everything on each selected ESP. This cannot be undone.</p>
    <p id="flash-targets"></p>
    <div id="flash-progress" hidden aria-live="polite"></div>
    <label id="flash-log-label" for="flash-log" hidden>Flash log</label>
    <textarea id="flash-log" rows="6" readonly hidden></textarea>
    <form id="flash-form" method="dialog">
      <button value="cancel" autofocus>Cancel</button>
      <button value="flash">Flash</button>
    </form>
  </dialog>
  <footer>
    <a href="https://github.com/derDeno/PandaFlasher" target="_blank" rel="noopener noreferrer">
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
        <path d="M9 19c-5 1.5-5-2.5-7-3m14 6v-3.87a3.37 3.37 0 0 0-.94-2.61c3.14-.35 6.44-1.54 6.44-7A5.44 5.44 0 0 0 20 4.77 5.07 5.07 0 0 0 19.91 1S18.73.65 16 2.48a13.38 13.38 0 0 0-7 0C6.27.65 5.09 1 5.09 1A5.07 5.07 0 0 0 5 4.77a5.44 5.44 0 0 0-1.5 3.78c0 5.42 3.3 6.61 6.44 7A3.37 3.37 0 0 0 9 18.13V22"/>
      </svg>
      GitHub
    </a>
    <span>&copy; derDeno</span>
  </footer>
  <script>
    const live = location.protocol !== 'file:';
    if (live) document.title = 'PandaFlasher';
    const grid = document.querySelector('#device-grid');
    const actionBar = document.querySelector('#selection-actions');
    const selectedCards = new Map();
    let token = '';

    function showNotice(message) {
      const notice = document.querySelector('#notice');
      notice.textContent = message;
      notice.hidden = !message;
    }

    async function api(url, options) {
      const response = await fetch(url, options);
      if (!response.ok) throw new Error(await response.text());
      return response;
    }

    async function operate(op, values = {}) {
      showNotice('Working…');
      try {
        const body = new URLSearchParams({token, op, ...values});
        const response = await api('/api/action', {method: 'POST', body});
        const message = await response.text();
        showNotice(message);
        if (op === 'extension-restart' || op === 'extension') await loadInfo();
        return message;
      } catch (error) { showNotice(error.message); return ''; }
    }

    function selectedMask() {
      return [...selectedCards.values()].reduce((mask, device) => mask | (1 << device.port), 0);
    }

    function updateHomeBars() {
      const onHome = !location.hash || location.hash === '#home';
      document.querySelector('#home-toolbar').hidden = !onHome;
      actionBar.hidden = !onHome || !selectedCards.size;
      document.querySelector('#selected-target').textContent = selectedCards.size
        ? 'Selected: ' + [...selectedCards.values()].map(device => device.name).join(', ') : '';
    }

    function renderDevices(devices) {
      grid.replaceChildren();
      selectedCards.clear();
      const message = document.querySelector('#scan-message');
      message.hidden = !!devices.length;
      message.textContent = 'No connected devices responded. Press Identify to scan again.';
      for (const device of devices) {
        const card = document.createElement('section');
        card.className = 'device-card';
        const title = document.createElement('h3');
        title.textContent = device.name;
        const mac = document.createElement('p');
        mac.textContent = 'MAC address: ' + device.mac;
        const chip = document.createElement('p');
        chip.textContent = 'Chip: ' + device.chip;
        const button = document.createElement('button');
        button.type = 'button';
        button.className = 'device-select';
        button.setAttribute('aria-label', 'Select ' + device.name);
        button.setAttribute('aria-pressed', 'false');
        button.innerHTML = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="m5 12 5 5L19 7"/></svg>';
        button.addEventListener('click', () => {
          const selecting = !selectedCards.has(card);
          if (selecting) selectedCards.set(card, device);
          else selectedCards.delete(card);
          card.classList.toggle('selected', selecting);
          button.setAttribute('aria-pressed', String(selecting));
          button.setAttribute('aria-label', (selecting ? 'Deselect ' : 'Select ') + device.name);
          updateHomeBars();
        });
        card.append(title, mac, chip, button);
        grid.append(card);
      }
      updateHomeBars();
    }

    document.querySelector('#identify').addEventListener('click', async () => {
      if (!live) {
        renderDevices([
          {port: 1, name: 'Port 1', mac: 'AA:BB:CC:10:20:31', chip: 'ESP32-S3'},
          {port: 3, name: 'Port 3', mac: 'AA:BB:CC:10:20:33', chip: 'ESP32-C3'},
          {port: 8, name: 'Port 8', mac: 'AA:BB:CC:10:20:38', chip: 'ESP32-S3'}
        ]);
        return;
      }
      grid.replaceChildren();
      selectedCards.clear();
      document.querySelector('#scan-message').hidden = true;
      updateHomeBars();
      showNotice('Scanning connected devices…');
      try {
        renderDevices(await (await api('/api/identify')).json());
        showNotice('');
      } catch (error) { showNotice(error.message); }
    });

    addEventListener('hashchange', () => {
      updateHomeBars();
      if (!live) return;
      const refresh = location.hash === '#info' ? loadInfo
        : location.hash === '#filemanager' || location.hash === '#settings' ? loadFiles : null;
      if (refresh) refresh().catch(error => showNotice(error.message));
    });
    updateHomeBars();

    document.querySelector('#flash-button').addEventListener('click', () => {
      if (!selectedCards.size) return;
      document.querySelector('#flash-targets').textContent = 'Targets: ' + [...selectedCards.values()].map(device => device.name).join(', ');
      document.querySelector('#flash-dialog').showModal();
    });
    const flashDialog = document.querySelector('#flash-dialog');
    const flashForm = document.querySelector('#flash-form');
    let flashing = false;
    flashDialog.addEventListener('cancel', event => { if (flashing) event.preventDefault(); });
    flashDialog.addEventListener('close', async () => {
      if (!live || flashDialog.returnValue !== 'flash') return;
      flashing = true;
      document.querySelector('#flash-title').textContent = 'Flashing devices';
      document.querySelector('#flash-warning').hidden = true;
      document.querySelector('#flash-targets').hidden = true;
      const progress = document.querySelector('#flash-progress');
      progress.replaceChildren();
      progress.hidden = false;
      const log = document.querySelector('#flash-log');
      log.value = '';
      log.hidden = false;
      document.querySelector('#flash-log-label').hidden = false;
      const rows = new Map();
      for (const device of selectedCards.values()) {
        const row = document.createElement('div');
        row.className = 'flash-target';
        const label = document.createElement('span');
        label.textContent = device.name + ': 0%';
        const bar = document.createElement('progress');
        bar.max = 100;
        bar.value = 0;
        row.append(label, bar);
        progress.append(row);
        rows.set(device.port, {label, bar});
      }
      flashForm.replaceChildren(Object.assign(document.createElement('button'), {value: 'close', textContent: 'Flashing…', disabled: true}));
      flashDialog.showModal();
      try {
        const response = await api('/api/action', {method: 'POST', body: new URLSearchParams({token, op: 'flash', ports: selectedMask(), file: document.querySelector('#flash-file').value})});
        const reader = response.body.getReader();
        const decoder = new TextDecoder();
        let pending = '';
        while (true) {
          const {value, done} = await reader.read();
          pending += decoder.decode(value || new Uint8Array(), {stream: !done});
          const lines = pending.split('\n');
          pending = lines.pop();
          for (const line of lines) {
            if (!line) continue;
            const event = JSON.parse(line);
            if (event.type === 'progress') {
              const row = rows.get(event.port);
              if (row) { row.bar.value = event.percent; row.label.textContent = row.label.textContent.replace(/: .*$/, ': ' + event.percent + '%'); }
            } else if (event.type === 'result') {
              const row = rows.get(event.port);
              if (row) row.label.textContent = row.label.textContent.replace(/: .*$/, ': ' + event.text.replace(/\n/g, ' '));
            } else if (event.type === 'log') {
              log.value += event.text + '\n';
              log.scrollTop = log.scrollHeight;
            }
          }
          if (done) break;
        }
      } catch (error) { log.value += error.message + '\n'; }
      flashForm.replaceChildren(Object.assign(document.createElement('button'), {value: 'close', textContent: 'Close'}));
      flashForm.querySelector('button').focus();
      document.querySelector('#flash-title').textContent = 'Flash complete';
      flashing = false;
    });
    document.querySelector('#reset-button').addEventListener('click', () => {
      if (live && selectedCards.size) operate('reset', {ports: selectedMask()});
    });

    async function loadFiles() {
      if (!live) return;
      const files = await (await api('/api/files')).json();
      const list = document.querySelector('#file-list');
      list.replaceChildren();
      for (const name of files) {
        const row = list.insertRow();
        row.insertCell().textContent = name;
        const button = document.createElement('button');
        button.type = 'button';
        button.className = 'delete-file';
        button.setAttribute('aria-label', 'Delete ' + name);
        button.title = 'Delete ' + name;
        button.innerHTML = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M3 6h18M8 6V4h8v2m3 0-1 14H6L5 6m5 4v7m4-7v7"/></svg>';
        row.insertCell().append(button);
      }
      for (const id of ['flash-file', 'self-file', 'extension-file']) {
        const select = document.getElementById(id);
        const previous = select.value;
        select.replaceChildren(...files.map(name => new Option(name, name)));
        if (files.includes(previous)) select.value = previous;
      }
    }

    async function loadInfo() {
      if (!live) return;
      const info = await (await api('/api/info')).json();
      token = info.token;
      for (const [id, value] of Object.entries({
        name: info.name, version: info.version, uptime: info.uptime,
        ssid: info.ssid, ip: info.ip, mac: info.mac, signal: info.signal
      })) document.getElementById('info-' + id).textContent = value;
      document.querySelector('#ap-ssid').value = info.apSsid;
      document.querySelector('#extension-info').hidden = !info.extension;
      document.querySelector('#extension-update').hidden = !info.extension;
      document.querySelector('[data-op="extension-restart"]').hidden = !info.extension;
      if (info.extension) for (const [id, value] of Object.entries(info.extension))
        document.getElementById('info-extension-' + id).textContent = value;
    }

    document.querySelector('#upload-form').addEventListener('submit', async event => {
      event.preventDefault();
      if (!live) return;
      const file = document.querySelector('#upload-file').files[0];
      if (!file) return;
      showNotice('Uploading ' + file.name + '…');
      try {
        const body = new FormData();
        body.append('file', file);
        const response = await api('/api/upload', {method: 'POST', headers: {'X-Panda-Token': token}, body});
        showNotice(await response.text());
        event.target.reset();
        await loadFiles();
      } catch (error) { showNotice(error.message); }
    });

    document.querySelector('#settings').addEventListener('click', event => {
      const button = event.target.closest('[data-op]');
      if (!button || !live) return;
      const op = button.dataset.op;
      const file = op === 'self' ? document.querySelector('#self-file').value
        : op === 'extension' ? document.querySelector('#extension-file').value : '';
      operate(op, file ? {file} : {});
    });

    document.querySelector('#ap-name-form').addEventListener('submit', async event => {
      event.preventDefault();
      if (!live) return;
      try {
        const body = new URLSearchParams({token, apSsid: document.querySelector('#ap-ssid').value});
        showNotice(await (await api('/api/ap-name', {method: 'POST', body})).text());
      } catch (error) { showNotice(error.message); }
    });

    const deleteDialog = document.querySelector('#delete-dialog');
    let pendingRow;
    document.querySelector('.files').addEventListener('click', event => {
      const button = event.target.closest('.delete-file');
      if (!button) return;
      pendingRow = button.closest('tr');
      document.querySelector('#delete-filename').textContent = pendingRow.cells[0].textContent;
      deleteDialog.returnValue = '';
      deleteDialog.showModal();
    });
    deleteDialog.addEventListener('close', () => {
      if (deleteDialog.returnValue === 'delete' && pendingRow) {
        const name = pendingRow.cells[0].textContent;
        if (live) {
          operate('delete', {file: name}).then(result => {
            if (result) loadFiles().catch(error => showNotice(error.message));
          });
        } else pendingRow.remove();
      }
      pendingRow = null;
    });
    if (live) Promise.all([loadInfo(), loadFiles()]).catch(error => showNotice(error.message));
  </script>
</body>
</html>
)WEBUI";

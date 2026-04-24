#include "control_server.h"

namespace
{
const char kControlPage[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Tank Drive RC Car</title>
  <style>
    :root{
      --bg:#0f172a;
      --card:#1e293b;
      --accent:#22c55e;
      --text:#e2e8f0;
      --muted:#94a3b8;
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      font-family: "Trebuchet MS", "Segoe UI", sans-serif;
      background: radial-gradient(circle at top,#1e293b 0%,#0f172a 55%);
      color:var(--text);
      min-height:100vh;
      display:flex;
      align-items:center;
      justify-content:center;
      padding:16px;
    }
    .panel{
      width:min(760px,100%);
      background:linear-gradient(160deg,rgba(30,41,59,0.95),rgba(15,23,42,0.95));
      border:1px solid #334155;
      border-radius:16px;
      padding:20px;
      box-shadow:0 24px 56px rgba(2,6,23,0.45);
    }
    h1{margin:0 0 16px;font-size:1.4rem}
    .controls{
      display:grid;
      grid-template-columns:1fr 1fr;
      gap:16px;
    }
    .card{
      background:var(--card);
      border:1px solid #334155;
      border-radius:12px;
      padding:12px;
    }
    label{
      display:flex;
      justify-content:space-between;
      margin-bottom:8px;
      color:var(--muted);
      font-size:0.95rem;
    }
    input[type=range]{
      width:100%;
    }
    .drive{
      display:grid;
      grid-template-columns:1fr 1fr;
      gap:12px;
    }
    button{
      margin-top:14px;
      width:100%;
      background:var(--accent);
      color:#06290f;
      border:none;
      border-radius:10px;
      padding:12px;
      font-weight:700;
      font-size:1rem;
      cursor:pointer;
    }
    .quick-grid{
      margin-top:10px;
      display:grid;
      grid-template-columns:1fr 1fr;
      gap:8px;
    }
    .quick-grid button{
      margin-top:0;
      padding:10px;
      font-size:0.9rem;
      background:#16a34a;
    }
    .hint{margin-top:10px;color:var(--muted);font-size:0.9rem}
    @media (max-width: 640px){
      .controls{grid-template-columns:1fr}
    }
  </style>
</head>
<body>
  <main class="panel">
    <h1>ESP32 Tank-Drive Controller</h1>
    <section class="controls">
      <article class="card">
        <div class="drive">
          <div>
            <label for="left">Left track <span id="leftValue">0</span></label>
            <input id="left" type="range" min="-100" max="100" value="0">
          </div>
          <div>
            <label for="right">Right track <span id="rightValue">0</span></label>
            <input id="right" type="range" min="-100" max="100" value="0">
          </div>
        </div>
      </article>
      <article class="card">
        <label for="speedMultiplier">Speed multiplier <span id="speedValue">1.00</span></label>
        <input id="speedMultiplier" type="range" min="0" max="100" value="100">
        <label for="ratio" style="margin-top:12px;">Left-to-right ratio <span id="ratioValue">1.00</span></label>
        <input id="ratio" type="range" min="20" max="200" value="100">
        <button id="stopBtn">Stop</button>
        <div class="quick-grid">
          <button id="forwardBtn" type="button">Forward</button>
          <button id="reverseBtn" type="button">Reverse</button>
          <button id="pivotLeftBtn" type="button">Pivot Left</button>
          <button id="pivotRightBtn" type="button">Pivot Right</button>
        </div>
        <p class="hint">Tank drive: use left/right sliders independently for turning, pivoting, and straight driving. Joystick UI: <a href="/drive" style="color:#86efac;">/drive</a></p>
      </article>
    </section>
  </main>
  <script>
    const left = document.getElementById('left');
    const right = document.getElementById('right');
    const speedMultiplier = document.getElementById('speedMultiplier');
    const ratio = document.getElementById('ratio');
    const leftValue = document.getElementById('leftValue');
    const rightValue = document.getElementById('rightValue');
    const speedValue = document.getElementById('speedValue');
    const ratioValue = document.getElementById('ratioValue');
    const stopBtn = document.getElementById('stopBtn');
    const forwardBtn = document.getElementById('forwardBtn');
    const reverseBtn = document.getElementById('reverseBtn');
    const pivotLeftBtn = document.getElementById('pivotLeftBtn');
    const pivotRightBtn = document.getElementById('pivotRightBtn');

    let inFlight = false;
    let needsResend = false;

    function displayValues(){
      leftValue.textContent = left.value;
      rightValue.textContent = right.value;
      speedValue.textContent = (speedMultiplier.value / 100).toFixed(2);
      ratioValue.textContent = (ratio.value / 100).toFixed(2);
    }

    async function sendDrive(){
      if (inFlight){
        needsResend = true;
        return;
      }
      inFlight = true;
      const params = new URLSearchParams({
        left: left.value,
        right: right.value,
        speedMultiplier: (speedMultiplier.value / 100).toFixed(2),
        ratio: (ratio.value / 100).toFixed(2)
      });
      try{
        await fetch('/api/drive?' + params.toString(), { method: 'GET' });
      } catch (e){
        console.error('drive send failed', e);
      } finally {
        inFlight = false;
        if (needsResend){
          needsResend = false;
          sendDrive();
        }
      }
    }

    function onInput(){
      displayValues();
      sendDrive();
    }

    [left, right, speedMultiplier, ratio].forEach((el) => el.addEventListener('input', onInput));

    stopBtn.addEventListener('click', async () => {
      left.value = '0';
      right.value = '0';
      displayValues();
      try{
        await fetch('/api/stop', { method: 'POST' });
      } catch (e){
        console.error('stop send failed', e);
      }
    });

    function setTracks(leftTrack, rightTrack){
      left.value = String(leftTrack);
      right.value = String(rightTrack);
      onInput();
    }

    forwardBtn.addEventListener('click', () => setTracks(80, 80));
    reverseBtn.addEventListener('click', () => setTracks(-80, -80));
    pivotLeftBtn.addEventListener('click', () => setTracks(-70, 70));
    pivotRightBtn.addEventListener('click', () => setTracks(70, -70));

    displayValues();
    sendDrive();
  </script>
</body>
</html>
)HTML";

const char kJoystickPage[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Tank Drive Joystick</title>
  <style>
    :root{
      --bg:#07131e;
      --panel:#0f2233;
      --ring:#5eead4;
      --dot:#22d3ee;
      --stick:#f59e0b;
      --text:#e2e8f0;
      --muted:#94a3b8;
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      min-height:100vh;
      display:flex;
      align-items:center;
      justify-content:center;
      font-family:"Trebuchet MS","Segoe UI",sans-serif;
      background:
        radial-gradient(circle at 20% 20%,rgba(34,211,238,0.22),transparent 45%),
        radial-gradient(circle at 80% 20%,rgba(16,185,129,0.20),transparent 35%),
        linear-gradient(160deg,#020617,#07131e 55%,#0f2233);
      color:var(--text);
      padding:16px;
    }
    .panel{
      width:min(700px,100%);
      background:linear-gradient(150deg,rgba(15,34,51,0.95),rgba(2,6,23,0.95));
      border:1px solid #164e63;
      border-radius:18px;
      padding:18px;
      box-shadow:0 22px 50px rgba(2,6,23,0.55);
    }
    h1{margin:0 0 6px;font-size:1.25rem}
    .subtitle{margin:0 0 14px;color:var(--muted);font-size:0.95rem}
    .stage{
      position:relative;
      width:min(420px,90vw);
      height:min(420px,90vw);
      margin:0 auto;
    }
    .ring{
      position:absolute;
      inset:0;
      border-radius:50%;
      border:3px solid var(--ring);
      background:radial-gradient(circle,rgba(20,184,166,0.14),rgba(8,47,73,0.10) 55%,transparent 72%);
    }
    .dot{
      position:absolute;
      width:16px;
      height:16px;
      border-radius:50%;
      background:var(--dot);
      box-shadow:0 0 14px rgba(34,211,238,0.8);
      transform:translate(-50%,-50%);
    }
    .dot.top{left:50%;top:6%}
    .dot.bottom{left:50%;top:94%}
    .dot.left{left:6%;top:50%}
    .dot.right{left:94%;top:50%}
    .stick{
      position:absolute;
      width:76px;
      height:76px;
      border-radius:50%;
      border:2px solid #fcd34d;
      background:radial-gradient(circle at 30% 30%,#fde68a,#d97706);
      box-shadow:0 10px 24px rgba(245,158,11,0.35);
      transform:translate(-50%,-50%);
      touch-action:none;
      cursor:grab;
    }
    .metrics{
      margin-top:16px;
      display:grid;
      grid-template-columns:1fr 1fr;
      gap:14px;
    }
    .card{
      background:var(--panel);
      border:1px solid #164e63;
      border-radius:12px;
      padding:12px;
    }
    label{
      display:flex;
      justify-content:space-between;
      margin-bottom:8px;
      color:var(--muted);
      font-size:0.95rem;
    }
    input[type=range]{width:100%}
    .values{margin-top:8px;color:var(--text);font-size:0.95rem}
    @media (max-width: 640px){
      .metrics{grid-template-columns:1fr}
    }
  </style>
</head>
<body>
  <main class="panel">
    <h1>Joystick Drive</h1>
    <p class="subtitle">Touch and drag the stick. Releasing touch auto-resets to center.</p>
    <section class="stage" id="stage">
      <div class="ring"></div>
      <div class="dot top"></div>
      <div class="dot bottom"></div>
      <div class="dot left"></div>
      <div class="dot right"></div>
      <div class="stick" id="stick"></div>
    </section>
    <section class="metrics">
      <article class="card">
        <label for="speedMultiplier">Speed multiplier <span id="speedValue">1.00</span></label>
        <input id="speedMultiplier" type="range" min="0" max="100" value="100">
        <label for="ratio" style="margin-top:12px;">Left-to-right ratio <span id="ratioValue">1.00</span></label>
        <input id="ratio" type="range" min="20" max="200" value="100">
      </article>
      <article class="card">
        <div class="values">X (steer): <span id="xValue">0</span></div>
        <div class="values">Y (throttle): <span id="yValue">0</span></div>
        <div class="values">Tip: Y+ forward, Y- reverse, X steers left/right.</div>
      </article>
    </section>
  </main>
  <script>
    const stage = document.getElementById('stage');
    const stick = document.getElementById('stick');
    const speedMultiplier = document.getElementById('speedMultiplier');
    const ratio = document.getElementById('ratio');
    const speedValue = document.getElementById('speedValue');
    const ratioValue = document.getElementById('ratioValue');
    const xValue = document.getElementById('xValue');
    const yValue = document.getElementById('yValue');

    let state = { x: 0, y: 0 };
    let dragging = false;
    let activePointerId = null;
    let inFlight = false;
    let needsResend = false;

    function updateDisplays(){
      speedValue.textContent = (speedMultiplier.value / 100).toFixed(2);
      ratioValue.textContent = (ratio.value / 100).toFixed(2);
      xValue.textContent = String(state.x);
      yValue.textContent = String(state.y);
    }

    function clamp(v, lo, hi){
      return Math.max(lo, Math.min(hi, v));
    }

    function updateStickVisual(){
      const rect = stage.getBoundingClientRect();
      const center = rect.width / 2;
      const radius = rect.width * 0.38;
      const px = center + (state.x / 100) * radius;
      const py = center - (state.y / 100) * radius;
      stick.style.left = px + 'px';
      stick.style.top = py + 'px';
    }

    async function sendJoystick(){
      if (inFlight){
        needsResend = true;
        return;
      }
      inFlight = true;
      const params = new URLSearchParams({
        x: String(state.x),
        y: String(state.y),
        speedMultiplier: (speedMultiplier.value / 100).toFixed(2),
        ratio: (ratio.value / 100).toFixed(2)
      });
      try{
        await fetch('/api/joystick?' + params.toString(), { method: 'GET' });
      } catch (e){
        console.error('joystick send failed', e);
      } finally {
        inFlight = false;
        if (needsResend){
          needsResend = false;
          sendJoystick();
        }
      }
    }

    function setState(x, y){
      state.x = clamp(Math.round(x), -100, 100);
      state.y = clamp(Math.round(y), -100, 100);
      updateDisplays();
      updateStickVisual();
      sendJoystick();
    }

    function pointerToState(clientX, clientY){
      const rect = stage.getBoundingClientRect();
      const centerX = rect.left + rect.width / 2;
      const centerY = rect.top + rect.height / 2;
      const dx = clientX - centerX;
      const dy = clientY - centerY;
      const maxRadius = rect.width * 0.38;
      const dist = Math.sqrt(dx * dx + dy * dy);
      const scale = dist > maxRadius ? maxRadius / dist : 1;
      const nx = dx * scale;
      const ny = dy * scale;
      const x = (nx / maxRadius) * 100;
      const y = (-ny / maxRadius) * 100;
      setState(x, y);
    }

    stage.addEventListener('pointerdown', (ev) => {
      dragging = true;
      activePointerId = ev.pointerId;
      stage.setPointerCapture(ev.pointerId);
      pointerToState(ev.clientX, ev.clientY);
    });

    stage.addEventListener('pointermove', (ev) => {
      if (!dragging || ev.pointerId !== activePointerId){
        return;
      }
      pointerToState(ev.clientX, ev.clientY);
    });

    function resetStick(){
      dragging = false;
      activePointerId = null;
      setState(0, 0);
    }

    stage.addEventListener('pointerup', resetStick);
    stage.addEventListener('pointercancel', resetStick);
    stage.addEventListener('pointerleave', () => {
      if (dragging){
        resetStick();
      }
    });

    [speedMultiplier, ratio].forEach((el) => el.addEventListener('input', () => {
      updateDisplays();
      sendJoystick();
    }));

    window.addEventListener('resize', updateStickVisual);

    updateDisplays();
    updateStickVisual();
    sendJoystick();
  </script>
</body>
</html>
)HTML";
} // namespace

ControlServer::ControlServer(uint16_t port) : server_(port) {}

void ControlServer::begin(const DriveHandler &driveHandler, const JoystickHandler &joystickHandler, const StopHandler &stopHandler)
{
  driveHandler_ = driveHandler;
  joystickHandler_ = joystickHandler;
  stopHandler_ = stopHandler;
  registerRoutes();
  server_.begin();
  Serial.println("[http] Web control server started.");
}

void ControlServer::loop()
{
  server_.handleClient();
}

void ControlServer::registerRoutes()
{
  server_.on("/", HTTP_GET, [this]()
             { handleIndex(); });

  server_.on("/drive", HTTP_GET, [this]()
             { handleDrivePage(); });

  server_.on("/api/drive", HTTP_GET, [this]()
             { handleDrive(); });

  server_.on("/api/joystick", HTTP_GET, [this]()
             { handleJoystick(); });

  server_.on("/api/stop", HTTP_POST, [this]()
             { handleStop(); });

  server_.onNotFound([this]()
                     { server_.send(404, "text/plain", "Not found"); });
}

void ControlServer::handleIndex()
{
  Serial.println("[http] GET /");
  server_.send_P(200, "text/html", kControlPage);
}

void ControlServer::handleDrivePage()
{
  Serial.println("[http] GET /drive");
  server_.send_P(200, "text/html", kJoystickPage);
}

void ControlServer::handleDrive()
{
  int left = getIntArg("left", 0);
  int right = getIntArg("right", 0);
  float speedMultiplier = getFloatArg("speedMultiplier", 1.0f);
  float ratio = getFloatArg("ratio", 1.0f);

  Serial.printf("[http] /api/drive left=%d right=%d speedMultiplier=%.2f ratio=%.2f\n", left, right, speedMultiplier, ratio);
  if (driveHandler_)
  {
    driveHandler_(left, right, speedMultiplier, ratio);
  }

  server_.send(200, "application/json", "{\"ok\":true}");
}

void ControlServer::handleJoystick()
{
  int x = getIntArg("x", 0);
  int y = getIntArg("y", 0);
  float speedMultiplier = getFloatArg("speedMultiplier", 1.0f);
  float ratio = getFloatArg("ratio", 1.0f);

  Serial.printf("[http] /api/joystick x=%d y=%d speedMultiplier=%.2f ratio=%.2f\n", x, y, speedMultiplier, ratio);
  if (joystickHandler_)
  {
    joystickHandler_(x, y, speedMultiplier, ratio);
  }

  server_.send(200, "application/json", "{\"ok\":true}");
}

void ControlServer::handleStop()
{
  Serial.println("[http] /api/stop");
  if (stopHandler_)
  {
    stopHandler_();
  }
  server_.send(200, "application/json", "{\"ok\":true}");
}

int ControlServer::getIntArg(const char *name, int defaultValue)
{
  if (!server_.hasArg(name))
  {
    return defaultValue;
  }
  return server_.arg(name).toInt();
}

float ControlServer::getFloatArg(const char *name, float defaultValue)
{
  if (!server_.hasArg(name))
  {
    return defaultValue;
  }
  return server_.arg(name).toFloat();
}

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
        <p class="hint">Tank drive: use left/right sliders independently for turning, pivoting, and straight driving.</p>
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
} // namespace

ControlServer::ControlServer(uint16_t port) : server_(port) {}

void ControlServer::begin(const DriveHandler &driveHandler, const StopHandler &stopHandler)
{
  driveHandler_ = driveHandler;
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

  server_.on("/api/drive", HTTP_GET, [this]()
             { handleDrive(); });

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

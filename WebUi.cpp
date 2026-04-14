#include "WebUi.h"

namespace {
String jsonEscape(const String& input) {
  String out;
  out.reserve(input.length() + 8);

  for (size_t i = 0; i < input.length(); ++i) {
    const char c = input.charAt(i);
    if (c == '\\') {
      out += "\\\\";
    } else if (c == '"') {
      out += "\\\"";
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else {
      out += c;
    }
  }

  return out;
}
}

String WebUi::renderRoot(const AppState& state) {
  String html = R"====(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Parabole 3D Control</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body{margin:0;font-family:Arial,sans-serif;background:#05060a;color:#e5e9f0;}
.topbar{padding:10px 20px;background:#111827;display:flex;justify-content:space-between;align-items:center;box-shadow:0 2px 8px rgba(0,0,0,.6);}
.statustopbar{padding:10px 20px;background:#111805;display:flex;justify-content:space-between;align-items:center;box-shadow:0 2px 8px rgba(0,0,0,.6);flex-direction: column;}
.statustopbaritem{display:flex;justify-content:space-between;align-items:center;gap: 2vw;width: 100%;}
.title{font-size:20px;font-weight:bold;letter-spacing:1px;}
.ip{font-size:12px;opacity:.7;}
.container{display:flex;flex-wrap:wrap;padding:15px;gap:15px;justify-content: center;}
.container-3d{display:flex;flex-wrap:wrap;padding:15px;gap:15px;width: 66vw;}
.container-panel{display:flex;flex-wrap:wrap;padding:15px;gap:15px;align-items: flex-start;width: 100%;}
.card{background:#111827;border-radius:10px;padding:15px;box-shadow:0 2px 10px rgba(0,0,0,.7);flex:1 1 260px;min-width:260px;display: flex;flex-direction: column;align-items: stretch;}
.card h2{margin-top:0;font-size:16px;border-bottom:1px solid #1f2937;padding-bottom:5px;margin-bottom:10px;}
.label{font-size:12px;text-transform:uppercase;color:#9ca3af;}
.value{font-size:18px;margin-bottom:5px;}
.ok{color:#10b981;}.bad{color:#ef4444;}
input{background:#020617;border:1px solid #374151;border-radius:6px;padding:4px 6px;color:#e5e9f0;margin-bottom:5px;}
input:focus{outline:none;border-color:#3b82f6;}
button{background:#3b82f6;border:none;border-radius:6px;padding:6px 10px;color:#fff;cursor:pointer;font-size:12px;margin-top:5px;}
button:hover{background:#2563eb;}
#oForm{display:flex;flex-direction:column;align-items:stretch;}
#c3d{width:100%;height:450px;display:block;border-radius:6px;cursor:grab;}
#c3d:active{cursor:grabbing;}
.legend{font-size:11px;margin-top:6px;color:#9ca3af;display:flex;gap:12px;}
.ld{display:flex;align-items:center;gap:4px;}
.lc{width:10px;height:10px;border-radius:50%;display:inline-block;}
</style>
</head>
<body>
<div class="topbar">
    <div class="title">Parabole Control 3D</div>
    <div class="statustopbar">
        <div class="statustopbaritem">
            <div class="label">Client</div>
            <div class="value" id="clientState">--</div>
        </div>
        <div class="statustopbaritem">
            <div class="label">Derniere commande</div>
            <div class="value" id="lastCmd">--</div>
        </div>
    </div>
</div>
<div class="container">
    <div class="container-3d">
        <div class="card" >
            <h2>Vue 3D</h2>
            <canvas id="c3d"></canvas>
            <div class="legend">
            <div class="ld"><span class="lc" style="background:#90a4ae"></span>Courant</div>
            <div class="ld"><span class="lc" style="background:#fbbf24"></span>Cible</div>
            <div class="ld"><span class="lc" style="background:#10b981"></span>Direction</div>
            </div>
        </div>
    </div>
    <div class="container-panel">
        <div class="card">
            <h2>IMU &amp; Cible</h2>
            <div class="label">Current AZ</div><div class="value" id="curAz">--</div>
            <div class="label">Current EL</div><div class="value" id="curEl">--</div>
            <div class="label">Target AZ</div><div class="value" id="tarAz">--</div>
            <div class="label">Target EL</div><div class="value" id="tarEl">--</div>
        </div>
        <div class="card">
            <h2>Offsets</h2>
            <form id="oForm">
            <div class="label">AZ offset</div>
            <input type="text" id="offAz" name="az">
            <div class="label">EL offset</div>
            <input type="text" id="offEl" name="el"><br>
            <button type="submit">Enregistrer</button>
            </form>
        </div>
        <div class="card">
            <h2>Motor Drive</h2>
            <div class="label">Manual step</div>
            <select id="manualStep" style="background:#020617;border:1px solid #374151;border-radius:6px;padding:4px 6px;color:#e5e9f0;margin-bottom:8px;">
            <option value="0.5">0.5&deg;</option>
            <option value="1.0">1&deg;</option>
            <option value="2.0" selected>2&deg;</option>
            <option value="5.0">5&deg;</option>
            </select>
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;">
            <button type="button" id="upBtn" style="grid-column:1 / span 2;">Up</button>
            <button type="button" id="leftBtn">Left</button>
            <button type="button" id="rightBtn">Right</button>
            <button type="button" id="downBtn" style="grid-column:1 / span 2;">Down</button>
            </div>
        </div>
        <div class="card">
            <h2>Park</h2>
            <div class="label" style="margin-top:10px;">Park AZ</div>
            <input type="text" id="parkAz" name="parkAz">
            <div class="label">Park EL</div>
            <input type="text" id="parkEl" name="parkEl"><br>
            <button type="button" id="saveParkBtn">Save Park</button>
            <button type="button" id="goParkBtn">Park Now</button>
        </div>
    </div>
</div>

<script type="importmap">
{"imports":{"three":"https://unpkg.com/three@0.158.0/build/three.module.js","three/addons/":"https://unpkg.com/three@0.158.0/examples/jsm/"}}
</script>

<script type="module">
import * as THREE from 'three';
import {OrbitControls} from 'three/addons/controls/OrbitControls.js';

let D = {
  currentAz:%CUR_AZ%, currentEl:%CUR_EL%,
  targetAz:%TAR_AZ%,  targetEl:%TAR_EL%,
  offsetAz:%OFF_AZ%,  offsetEl:%OFF_EL%,
  parkAz:%PARK_AZ%,  parkEl:%PARK_EL%,
  client:"%CLIENT%",  lastCmd:"%LASTCMD%"
};

const canvas = document.getElementById('c3d');
const renderer = new THREE.WebGLRenderer({canvas, antialias:true});
renderer.setPixelRatio(devicePixelRatio);
renderer.setClearColor(0x0d1117);

const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(45, 1, 0.1, 200);
camera.position.set(7, 6, 9);

const controls = new OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;
controls.dampingFactor = 0.1;
controls.target.set(0, 2, 0);
controls.minDistance = 2;
controls.maxDistance = 30;

function resize() {
  const w = canvas.clientWidth, h = canvas.clientHeight;
  renderer.setSize(w, h, false);
  camera.aspect = w / h;
  camera.updateProjectionMatrix();
}
window.addEventListener('resize', resize);
resize();

scene.add(new THREE.AmbientLight(0x607080, 1.2));
const sun = new THREE.DirectionalLight(0xffffff, 1.8);
sun.position.set(8, 12, 6);
scene.add(sun);
const fill = new THREE.DirectionalLight(0x3b82f6, 0.4);
fill.position.set(-6, 4, -4);
scene.add(fill);
scene.add(new THREE.GridHelper(20, 20, 0x1f2937, 0x1f2937));

function makeCompass() {
  const g = new THREE.Group();
  const ring = new THREE.Mesh(
    new THREE.TorusGeometry(3.2, 0.05, 8, 72),
    new THREE.MeshBasicMaterial({color:0x374151})
  );
  ring.rotation.x = Math.PI / 2;
  g.add(ring);
  const dirs = [
    {t:'N', a:0,   c:'#ef4444'},
    {t:'E', a:90,  c:'#94a3b8'},
    {t:'S', a:180, c:'#94a3b8'},
    {t:'W', a:270, c:'#94a3b8'}
  ];
  dirs.forEach(d => {
    const r = d.a * Math.PI / 180;
    const sx = Math.sin(r), sz = Math.cos(r);
    const cv = document.createElement('canvas');
    cv.width = 64; cv.height = 64;
    const ctx = cv.getContext('2d');
    ctx.fillStyle = d.c;
    ctx.font = 'bold 42px Arial';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(d.t, 32, 32);
    const sp = new THREE.Sprite(new THREE.SpriteMaterial({map:new THREE.CanvasTexture(cv), depthTest:false}));
    sp.scale.set(0.55, 0.55, 0.55);
    sp.position.set(sx * 3.9, 0.05, sz * 3.9);
    g.add(sp);
    const tk = new THREE.Mesh(
      new THREE.BoxGeometry(0.06, 0.01, 0.28),
      new THREE.MeshBasicMaterial({color: d.t === 'N' ? 0xef4444 : 0x4b5563})
    );
    tk.position.set(sx * 3.2, 0.01, sz * 3.2);
    tk.rotation.y = -r;
    g.add(tk);
  });
  for (let a = 0; a < 360; a += 10) {
    if (a % 90 === 0) continue;
    const r = a * Math.PI / 180;
    const tk = new THREE.Mesh(
      new THREE.BoxGeometry(0.03, 0.01, a % 45 === 0 ? 0.2 : 0.12),
      new THREE.MeshBasicMaterial({color:0x374151})
    );
    tk.position.set(Math.sin(r) * 3.2, 0.01, Math.cos(r) * 3.2);
    tk.rotation.y = -r;
    g.add(tk);
  }
  return g;
}
scene.add(makeCompass());

function makeDish(color, wire) {
  const pts = [];
  for (let i = 0; i <= 24; i++) {
    const x = (i / 24) * 1.2;
    pts.push(new THREE.Vector2(x, x * x / 1.3));
  }
  const geo = new THREE.LatheGeometry(pts, 48);
  geo.rotateX(Math.PI / 2);
  const mat = wire
    ? new THREE.MeshBasicMaterial({color, transparent:true, opacity:0.2})
    : new THREE.MeshPhongMaterial({color, shininess:80, side:THREE.DoubleSide});
  return new THREE.Mesh(geo, mat);
}

const base = new THREE.Mesh(
  new THREE.CylinderGeometry(0.55, 0.65, 0.3, 16),
  new THREE.MeshPhongMaterial({color:0x1f2937}));
base.position.y = 0.15;
scene.add(base);

const mast = new THREE.Mesh(
  new THREE.CylinderGeometry(0.08, 0.12, 2.5, 12),
  new THREE.MeshPhongMaterial({color:0x374151}));
mast.position.y = 1.25;
scene.add(mast);

const joint = new THREE.Mesh(
  new THREE.SphereGeometry(0.14, 12, 12),
  new THREE.MeshPhongMaterial({color:0x4b5563}));
joint.position.y = 2.52;
scene.add(joint);

const curGroup = new THREE.Group();
curGroup.position.set(0, 2.5, 0);
curGroup.add(makeDish(0x90a4ae, false));

const feed = new THREE.Mesh(
  new THREE.SphereGeometry(0.07, 12, 12),
  new THREE.MeshPhongMaterial({color:0xf97316, emissive:0xf97316, emissiveIntensity:0.5}));
feed.position.set(0, 0, 0.52);
curGroup.add(feed);

const armGeo = new THREE.CylinderGeometry(0.015, 0.015, 0.55, 6);
armGeo.rotateX(Math.PI / 2);
const arm = new THREE.Mesh(armGeo, new THREE.MeshPhongMaterial({color:0x6b7280}));
arm.position.set(0, 0, 0.27);
curGroup.add(arm);
scene.add(curGroup);

const tarGroup = new THREE.Group();
tarGroup.position.copy(curGroup.position);
tarGroup.add(makeDish(0xfbbf24, true));
scene.add(tarGroup);

const arrow = new THREE.ArrowHelper(
  new THREE.Vector3(0, 0, 1),
  new THREE.Vector3(0, 2.5, 0),
  2.5, 0x10b981, 0.35, 0.18);
scene.add(arrow);

function setDishRotation(group, az, el) {
  group.rotation.order = 'YXZ';
  group.rotation.y = THREE.MathUtils.degToRad(az);
  group.rotation.x = -THREE.MathUtils.degToRad(el);
}

function getDishDir(az, el) {
  const a = THREE.MathUtils.degToRad(az);
  const e = THREE.MathUtils.degToRad(el);
  return new THREE.Vector3(
    Math.cos(e) * Math.sin(a),
    Math.sin(e),
    Math.cos(e) * Math.cos(a)
  ).normalize();
}

function animate() {
  requestAnimationFrame(animate);
  controls.update();
  setDishRotation(curGroup, D.currentAz, D.currentEl);
  setDishRotation(tarGroup, D.targetAz, D.targetEl);
  arrow.setDirection(getDishDir(D.currentAz, D.currentEl));
  renderer.render(scene, camera);
}
animate();

function updateUI() {
  const conn = Number(D.client) === 1;
  const cs = document.getElementById('clientState');
  cs.textContent = conn ? "Connecte" : "Aucun";
  cs.className = conn ? "value ok" : "value bad";
  document.getElementById('lastCmd').textContent = D.lastCmd.trim();
  document.getElementById('curAz').textContent = D.currentAz.toFixed(1) + "\xb0";
  document.getElementById('curEl').textContent = D.currentEl.toFixed(1) + "\xb0";
  document.getElementById('tarAz').textContent = D.targetAz.toFixed(1) + "\xb0";
  document.getElementById('tarEl').textContent = D.targetEl.toFixed(1) + "\xb0";

  const offAzInput = document.getElementById('offAz');
  const offElInput = document.getElementById('offEl');
  const parkAzInput = document.getElementById('parkAz');
  const parkElInput = document.getElementById('parkEl');

  if (document.activeElement !== offAzInput) offAzInput.value = D.offsetAz;
  if (document.activeElement !== offElInput) offElInput.value = D.offsetEl;
  if (document.activeElement !== parkAzInput) parkAzInput.value = D.parkAz;
  if (document.activeElement !== parkElInput) parkElInput.value = D.parkEl;
}

function fetchData() {
  fetch('/status').then(r => r.json()).then(j => { D = j; updateUI(); }).catch(() => {});
}

document.getElementById('oForm').addEventListener('submit', e => {
  e.preventDefault();
  fetch('/setoffset?az=' + encodeURIComponent(document.getElementById('offAz').value)
    + '&el=' + encodeURIComponent(document.getElementById('offEl').value))
    .then(() => setTimeout(fetchData, 300));
});

function sendManual(dir) {
  const step = document.getElementById('manualStep').value || '2.0';
  fetch('/manual?d=' + encodeURIComponent(dir) + '&s=' + encodeURIComponent(step))
    .then(() => setTimeout(fetchData, 120));
}

document.getElementById('upBtn').addEventListener('click', () => sendManual('up'));
document.getElementById('downBtn').addEventListener('click', () => sendManual('down'));
document.getElementById('leftBtn').addEventListener('click', () => sendManual('right'));
document.getElementById('rightBtn').addEventListener('click', () => sendManual('left'));

document.getElementById('goParkBtn').addEventListener('click', () => {
  fetch('/park').then(() => setTimeout(fetchData, 120));
});

document.getElementById('saveParkBtn').addEventListener('click', () => {
  fetch('/setpark?az=' + encodeURIComponent(document.getElementById('parkAz').value)
    + '&el=' + encodeURIComponent(document.getElementById('parkEl').value))
    .then(() => setTimeout(fetchData, 120));
});

updateUI();
setInterval(fetchData, 600);
</script>
</body>
</html>
)====";

  html.replace("%CUR_AZ%", String(state.currentAz, 1));
  html.replace("%CUR_EL%", String(state.currentEl, 1));
  html.replace("%TAR_AZ%", String(state.targetAz, 1));
  html.replace("%TAR_EL%", String(state.targetEl, 1));
  html.replace("%OFF_AZ%", String(state.offsetAz, 1));
  html.replace("%OFF_EL%", String(state.offsetEl, 1));
  html.replace("%PARK_AZ%", String(state.parkAz, 1));
  html.replace("%PARK_EL%", String(state.parkEl, 1));
  html.replace("%CLIENT%", state.clientConnected ? "1" : "0");
  html.replace("%LASTCMD%", state.lastCommand);

  return html;
}

String WebUi::statusJson(const AppState& state) {
  String json = "{";
  json += "\"currentAz\":" + String(state.currentAz,1) + ",";
  json += "\"currentEl\":" + String(state.currentEl,1) + ",";
  json += "\"targetAz\":" + String(state.targetAz,1) + ",";
  json += "\"targetEl\":" + String(state.targetEl,1) + ",";
  json += "\"offsetAz\":" + String(state.offsetAz,1) + ",";
  json += "\"offsetEl\":" + String(state.offsetEl,1) + ",";
  json += "\"parkAz\":" + String(state.parkAz,1) + ",";
  json += "\"parkEl\":" + String(state.parkEl,1) + ",";
  json += "\"client\":\"" + String(state.clientConnected ? "1" : "0") + "\",";
  json += "\"lastCmd\":\"" + jsonEscape(state.lastCommand) + "\"";
  json += "}";
  return json;
}

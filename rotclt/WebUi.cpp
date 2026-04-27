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
<html lang="en">
<head>
<meta charset="utf-8">
<title>Rotator Control</title>
<meta name="viewport" content="width=device-width,initial-scale=1.0,maximum-scale=1.0,user-scalable=no">
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0;}
:root{
  --bg:#080b14;--surface:#0f1629;--surface2:#161d33;--border:#1e2a45;
  --text:#e2e8f0;--text2:#94a3b8;--text3:#64748b;
  --accent:#3b82f6;--accent-hover:#2563eb;--accent-glow:rgba(59,130,246,.15);
  --green:#10b981;--red:#ef4444;--amber:#f59e0b;--orange:#f97316;
  --radius:12px;--radius-sm:8px;
}
html{font-size:14px;}
body{font-family:'Inter','Segoe UI',system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);min-height:100vh;overflow-x:hidden;}

/* ── Top Bar ─────────────────────────── */
.topbar{
  position:sticky;top:0;z-index:100;
  background:rgba(15,22,41,.92);backdrop-filter:blur(12px);
  border-bottom:1px solid var(--border);
  padding:10px 16px;
  display:flex;align-items:center;gap:12px;flex-wrap:wrap;
}
.topbar-brand{display:flex;align-items:center;gap:8px;margin-right:auto;}
.topbar-brand svg{width:26px;height:26px;color:var(--accent);}
.topbar-title{font-size:1.15rem;font-weight:700;letter-spacing:.5px;white-space:nowrap;}
.status-chips{display:flex;gap:8px;flex-wrap:wrap;}
.chip{
  display:flex;align-items:center;gap:6px;
  background:var(--surface2);border:1px solid var(--border);border-radius:20px;
  padding:4px 12px 4px 8px;font-size:.75rem;white-space:nowrap;
}
.chip-dot{width:8px;height:8px;border-radius:50%;flex-shrink:0;}
.chip-dot.on{background:var(--green);box-shadow:0 0 6px var(--green);}
.chip-dot.off{background:var(--red);opacity:.7;}
.chip-label{color:var(--text2);}
.chip-value{color:var(--text);font-weight:600;}

/* ── Layout ──────────────────────────── */
.main{display:grid;gap:12px;padding:12px;grid-template-columns:1fr;max-width:1600px;margin:0 auto;}
@media(min-width:768px){.main{grid-template-columns:1fr 1fr;}}
@media(min-width:1100px){.main{grid-template-columns:1.6fr 1fr;}}

.col-3d{grid-column:1 / -1;}
@media(min-width:768px){.col-3d{grid-column:1 / -1;}}
@media(min-width:1100px){.col-3d{grid-column:1;grid-row:1 / 4;}}

.panels{
  display:grid;gap:12px;
  grid-template-columns:1fr;
  grid-column:1 / -1;
}
@media(min-width:480px){.panels{grid-template-columns:1fr 1fr;}}
@media(min-width:1100px){.panels{grid-column:2;grid-row:1 / 4;grid-template-columns:1fr;}}

/* ── Cards ───────────────────────────── */
.card{
  background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);
  padding:16px;
  transition:border-color .2s;
}
.card:hover{border-color:rgba(59,130,246,.25);}
.card-header{
  display:flex;align-items:center;gap:8px;
  margin-bottom:12px;padding-bottom:8px;
  border-bottom:1px solid var(--border);
}
.card-icon{font-size:1.1rem;}
.card-title{font-size:.9rem;font-weight:700;text-transform:uppercase;letter-spacing:.6px;color:var(--text2);}

/* ── Data display ────────────────────── */
.data-grid{display:grid;grid-template-columns:1fr 1fr;gap:4px 16px;}
.data-item{display:flex;flex-direction:column;padding:4px 0;}
.data-label{font-size:.65rem;text-transform:uppercase;letter-spacing:.8px;color:var(--text3);margin-bottom:2px;}
.data-value{font-size:1.3rem;font-weight:700;font-variant-numeric:tabular-nums;color:var(--text);}
.data-value.accent{color:var(--accent);}
.data-value.amber{color:var(--amber);}

/* ── Form elements ───────────────────── */
.form-group{display:flex;flex-direction:column;gap:3px;margin-bottom:8px;}
.form-label{font-size:.65rem;text-transform:uppercase;letter-spacing:.8px;color:var(--text3);}
.form-input{
  background:var(--bg);border:1px solid var(--border);border-radius:var(--radius-sm);
  padding:8px 10px;color:var(--text);font-size:.9rem;width:100%;
  transition:border-color .2s;
}
.form-input:focus{outline:none;border-color:var(--accent);box-shadow:0 0 0 3px var(--accent-glow);}
select.form-input{cursor:pointer;appearance:none;
  background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12' fill='%2394a3b8'%3E%3Cpath d='M6 8L1 3h10z'/%3E%3C/svg%3E");
  background-repeat:no-repeat;background-position:right 10px center;padding-right:28px;
}

/* ── Buttons ─────────────────────────── */
.btn{
  display:inline-flex;align-items:center;justify-content:center;gap:6px;
  border:none;border-radius:var(--radius-sm);padding:9px 14px;
  font-size:.8rem;font-weight:600;cursor:pointer;
  transition:all .15s;white-space:nowrap;
}
.btn-primary{background:var(--accent);color:#fff;}
.btn-primary:hover{background:var(--accent-hover);transform:translateY(-1px);box-shadow:0 4px 12px rgba(59,130,246,.3);}
.btn-primary:active{transform:translateY(0);}
.btn-success{background:var(--green);color:#fff;}
.btn-success:hover{background:#059669;}
.btn-danger{background:var(--red);color:#fff;}
.btn-danger:hover{background:#dc2626;}
.btn-amber{background:var(--amber);color:#000;}
.btn-amber:hover{background:#d97706;}
.btn-outline{background:transparent;border:1px solid var(--border);color:var(--text2);}
.btn-outline:hover{border-color:var(--accent);color:var(--accent);background:var(--accent-glow);}
.btn-sm{padding:6px 10px;font-size:.75rem;}
.btn-block{width:100%;}
.btn-icon{width:40px;height:40px;padding:0;font-size:1.1rem;}

/* ── Direction pad ───────────────────── */
.dpad{display:grid;grid-template-columns:repeat(3,1fr);gap:6px;max-width:180px;margin:0 auto;}
.dpad .btn{height:44px;}
.dpad-up{grid-column:2;}
.dpad-left{grid-column:1;grid-row:2;}
.dpad-right{grid-column:3;grid-row:2;}
.dpad-down{grid-column:2;grid-row:3;}

/* ── Enable toggle ───────────────────── */
.toggle-row{display:flex;gap:8px;align-items:center;}
.btn-enable{flex:1;}
.btn-enable.active{background:var(--green);color:#fff;}
.btn-enable.active:hover{background:#059669;}

/* ── Button groups ───────────────────── */
.btn-group{display:flex;gap:6px;margin-top:4px;}
.btn-group .btn{flex:1;}

/* ── 3D Canvas ───────────────────────── */
#c3d{width:100%;height:350px;display:block;border-radius:var(--radius-sm);cursor:grab;}
#c3d:active{cursor:grabbing;}
@media(min-width:768px){#c3d{height:450px;}}
@media(min-width:1100px){#c3d{height:calc(100vh - 100px);min-height:500px;max-height:900px;}}

.legend{display:flex;gap:14px;margin-top:8px;flex-wrap:wrap;}
.ld{display:flex;align-items:center;gap:5px;font-size:.7rem;color:var(--text3);}
.lc{width:10px;height:10px;border-radius:50%;display:inline-block;}

/* ── Mobile tab bar ──────────────────── */
.mob-tabbar{
  display:none;position:fixed;bottom:0;left:0;right:0;z-index:200;
  background:rgba(15,22,41,.96);backdrop-filter:blur(12px);
  border-top:1px solid var(--border);
  padding:6px 0 max(6px,env(safe-area-inset-bottom));
}
@media(max-width:767px){.mob-tabbar{display:flex;}}
.mob-tab{
  flex:1;display:flex;flex-direction:column;align-items:center;gap:2px;
  background:none;border:none;color:var(--text3);font-size:.6rem;font-weight:600;
  padding:4px 0;cursor:pointer;letter-spacing:.3px;text-transform:uppercase;
  transition:color .15s;
}
.mob-tab svg{width:22px;height:22px;}
.mob-tab.active{color:var(--accent);}

/* ── Mobile: show/hide sections ──────── */
@media(max-width:767px){
  body{padding-bottom:62px;}
  .col-3d.mob-hidden,.panels.mob-hidden{display:none !important;}
}

/* ── Mobile quick controls overlay ───── */
.mob-quick{
  display:none;position:fixed;bottom:62px;left:0;right:0;z-index:150;
  background:rgba(15,22,41,.94);backdrop-filter:blur(10px);
  border-top:1px solid var(--border);
  padding:8px 12px;
}
@media(max-width:767px){.mob-quick{display:flex;gap:10px;align-items:center;}}
.mob-quick.mob-hidden{display:none !important;}
.mob-quick-pos{display:grid;grid-template-columns:1fr 1fr;gap:2px 12px;flex:1;min-width:0;}
.mob-quick-pos .data-label{font-size:.55rem;margin-bottom:0;}
.mob-quick-pos .data-value{font-size:1rem;}
.mob-quick-dpad{display:grid;grid-template-columns:repeat(3,32px);gap:3px;}
.mob-quick-dpad .btn{width:32px;height:32px;padding:0;font-size:.85rem;min-width:0;}
.mob-dpad-up{grid-column:2;}
.mob-dpad-left{grid-column:1;grid-row:2;}
.mob-dpad-right{grid-column:3;grid-row:2;}
.mob-dpad-down{grid-column:2;grid-row:3;}
</style>
</head>
<body>

<!-- ── Top Bar ──────────────────────────────── -->
<header class="topbar">
  <div class="topbar-brand">
    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10A15.3 15.3 0 0 1 12 2z"/><line x1="2" y1="12" x2="22" y2="12"/></svg>
    <span class="topbar-title">Rotator Control</span>
  </div>
  <div class="status-chips">
    <div class="chip">
      <span class="chip-dot off" id="chipDot"></span>
      <span class="chip-label">Client:</span>
      <span class="chip-value" id="clientState">--</span>
    </div>
    <div class="chip">
      <span class="chip-label">Cmd:</span>
      <span class="chip-value" id="lastCmd">--</span>
    </div>
  </div>
</header>

<!-- ── Main Layout ─────────────────────────── -->
<div class="main">

  <!-- 3D View -->
  <div class="col-3d">
    <div class="card">
      <div class="card-header">
        <span class="card-icon">&#127758;</span>
        <span class="card-title">3D View</span>
      </div>
      <canvas id="c3d"></canvas>
      <div class="legend">
        <div class="ld"><span class="lc" style="background:#90a4ae"></span>Current</div>
        <div class="ld"><span class="lc" style="background:#fbbf24"></span>Target</div>
        <div class="ld"><span class="lc" style="background:#10b981"></span>Direction</div>
      </div>
    </div>
  </div>

  <!-- Control Panels -->
  <div class="panels">

    <!-- Position Card -->
    <div class="card" data-panel="ctrl">
      <div class="card-header">
        <span class="card-icon">&#128225;</span>
        <span class="card-title">Position</span>
      </div>
      <div class="data-grid">
        <div class="data-item">
          <span class="data-label">Current AZ</span>
          <span class="data-value" id="curAz">--</span>
        </div>
        <div class="data-item">
          <span class="data-label">Current EL</span>
          <span class="data-value" id="curEl">--</span>
        </div>
        <div class="data-item">
          <span class="data-label">Target AZ</span>
          <span class="data-value amber" id="tarAz">--</span>
        </div>
        <div class="data-item">
          <span class="data-label">Target EL</span>
          <span class="data-value amber" id="tarEl">--</span>
        </div>
      </div>
    </div>

    <!-- Motor Drive -->
    <div class="card" data-panel="ctrl">
      <div class="card-header">
        <span class="card-icon">&#9881;</span>
        <span class="card-title">Motor Drive</span>
      </div>
      <div class="toggle-row" style="margin-bottom:10px;">
        <button type="button" class="btn btn-outline btn-enable btn-block" id="enableBtn">Enable Motors</button>
      </div>
      <div class="form-group">
        <span class="form-label">Step size</span>
        <select id="manualStep" class="form-input">
          <option value="0.5">0.5&deg;</option>
          <option value="1.0">1&deg;</option>
          <option value="2.0" selected>2&deg;</option>
          <option value="5.0">5&deg;</option>
          <option value="10.0">10&deg;</option>
        </select>
      </div>
      <div class="dpad">
        <button type="button" class="btn btn-outline btn-icon dpad-up" id="upBtn">&#9650;</button>
        <button type="button" class="btn btn-outline btn-icon dpad-left" id="leftBtn">&#9664;</button>
        <button type="button" class="btn btn-outline btn-icon dpad-right" id="rightBtn">&#9654;</button>
        <button type="button" class="btn btn-outline btn-icon dpad-down" id="downBtn">&#9660;</button>
      </div>
    </div>

    <!-- Offsets -->
    <div class="card" data-panel="ctrl">
      <div class="card-header">
        <span class="card-icon">&#128295;</span>
        <span class="card-title">Offsets</span>
      </div>
      <form id="oForm">
        <div class="data-grid">
          <div class="form-group">
            <span class="form-label">AZ Offset</span>
            <input type="text" id="offAz" name="az" class="form-input" inputmode="decimal">
          </div>
          <div class="form-group">
            <span class="form-label">EL Offset</span>
            <input type="text" id="offEl" name="el" class="form-input" inputmode="decimal">
          </div>
        </div>
        <button type="submit" class="btn btn-primary btn-block">Save Offsets</button>
      </form>
      <div style="margin-top:8px;">
        <button type="button" class="btn btn-outline btn-sm btn-block" id="setParkBtn">Calibrate to Park</button>
      </div>
    </div>

    <!-- Park -->
    <div class="card" data-panel="park">
      <div class="card-header">
        <span class="card-icon">&#127968;</span>
        <span class="card-title">Park</span>
      </div>
      <div class="data-grid">
        <div class="form-group">
          <span class="form-label">Park AZ</span>
          <input type="text" id="parkAz" name="parkAz" class="form-input" inputmode="decimal">
        </div>
        <div class="form-group">
          <span class="form-label">Park EL</span>
          <input type="text" id="parkEl" name="parkEl" class="form-input" inputmode="decimal">
        </div>
      </div>
      <div class="btn-group">
        <button type="button" class="btn btn-outline" id="saveParkBtn">Save</button>
        <button type="button" class="btn btn-amber" id="goParkBtn">Park Now</button>
      </div>
    </div>

  </div>
</div>

<!-- ── Mobile quick controls (overlay on 3D tab) ── -->
<div class="mob-quick" id="mobQuick">
  <div class="mob-quick-pos">
    <div><span class="data-label">AZ</span><span class="data-value" id="mqAz">--</span></div>
    <div><span class="data-label">EL</span><span class="data-value" id="mqEl">--</span></div>
    <div><span class="data-label">T.AZ</span><span class="data-value amber" id="mqTAz">--</span></div>
    <div><span class="data-label">T.EL</span><span class="data-value amber" id="mqTEl">--</span></div>
  </div>
  <div class="mob-quick-dpad">
    <button type="button" class="btn btn-outline mob-dpad-up" id="mqUp">&#9650;</button>
    <button type="button" class="btn btn-outline mob-dpad-left" id="mqLeft">&#9664;</button>
    <button type="button" class="btn btn-outline mob-dpad-right" id="mqRight">&#9654;</button>
    <button type="button" class="btn btn-outline mob-dpad-down" id="mqDown">&#9660;</button>
  </div>
</div>

<!-- ── Mobile tab bar ──────────────────────── -->
<nav class="mob-tabbar" id="mobTabbar">
  <button class="mob-tab active" data-tab="3d">
    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/></svg>
    3D
  </button>
  <button class="mob-tab" data-tab="ctrl">
    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M9 3v18M3 9h18"/></svg>
    Controls
  </button>
  <button class="mob-tab" data-tab="park">
    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/></svg>
    Park
  </button>
</nav>

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
  parkAz:%PARK_AZ%,   parkEl:%PARK_EL%,
  enable:%ENABLE%,
  client:"%CLIENT%",   lastCmd:"%LASTCMD%"
};

const dirty = { offAz:false, offEl:false, parkAz:false, parkEl:false };

const canvas = document.getElementById('c3d');
const renderer = new THREE.WebGLRenderer({canvas, antialias:true});
renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
renderer.setClearColor(0x080b14);

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
    ? new THREE.MeshBasicMaterial({color, transparent:true, opacity:0.2, side:THREE.DoubleSide})
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
  group.rotation.y = -THREE.MathUtils.degToRad(az);
  group.rotation.x = -THREE.MathUtils.degToRad(el);
}

function getDishDir(az, el) {
  const a = -THREE.MathUtils.degToRad(az);
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
  const dot = document.getElementById('chipDot');
  cs.textContent = conn ? 'Connected' : 'None';
  dot.className = conn ? 'chip-dot on' : 'chip-dot off';
  document.getElementById('lastCmd').textContent = D.lastCmd ? D.lastCmd.trim() : '--';

  document.getElementById('curAz').textContent = D.currentAz.toFixed(1) + '\xb0';
  document.getElementById('curEl').textContent = D.currentEl.toFixed(1) + '\xb0';
  document.getElementById('tarAz').textContent = D.targetAz.toFixed(1) + '\xb0';
  document.getElementById('tarEl').textContent = D.targetEl.toFixed(1) + '\xb0';

  const offAzInput = document.getElementById('offAz');
  const offElInput = document.getElementById('offEl');
  const parkAzInput = document.getElementById('parkAz');
  const parkElInput = document.getElementById('parkEl');

  if (document.activeElement !== offAzInput && !dirty.offAz) offAzInput.value = D.offsetAz;
  if (document.activeElement !== offElInput && !dirty.offEl) offElInput.value = D.offsetEl;
  if (document.activeElement !== parkAzInput && !dirty.parkAz) parkAzInput.value = D.parkAz;
  if (document.activeElement !== parkElInput && !dirty.parkEl) parkElInput.value = D.parkEl;

  const eb = document.getElementById('enableBtn');
  if (D.enable) {
    eb.textContent = 'Disable Motors';
    eb.className = 'btn btn-enable btn-block active';
  } else {
    eb.textContent = 'Enable Motors';
    eb.className = 'btn btn-outline btn-enable btn-block';
  }
}

document.getElementById('offAz').addEventListener('input', () => { dirty.offAz = true; });
document.getElementById('offEl').addEventListener('input', () => { dirty.offEl = true; });
document.getElementById('parkAz').addEventListener('input', () => { dirty.parkAz = true; });
document.getElementById('parkEl').addEventListener('input', () => { dirty.parkEl = true; });

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
  const azVal = document.getElementById('parkAz').value;
  const elVal = document.getElementById('parkEl').value;
  const az = parseFloat(azVal);
  const el = parseFloat(elVal);
  if (isNaN(az) || isNaN(el)) {
    alert('Invalid park values');
    return;
  }
  fetch('/setpark?az=' + encodeURIComponent(azVal)
    + '&el=' + encodeURIComponent(elVal))
    .then(() => {
      dirty.parkAz = false;
      dirty.parkEl = false;
      setTimeout(fetchData, 120);
    });
});

document.getElementById('enableBtn').addEventListener('click', () => {
  const newState = D.enable ? 0 : 1;
  fetch('/enable?state=' + newState).then(() => setTimeout(fetchData, 300));
});

document.getElementById('setParkBtn').addEventListener('click', () => {
  fetch('/calibrate').then(() => setTimeout(fetchData, 120));
});

updateUI();
setInterval(fetchData, 100);

/* ── Mobile tab switching ─────────────── */
const col3d = document.querySelector('.col-3d');
const panels = document.querySelector('.panels');
const mobQuick = document.getElementById('mobQuick');
const tabs = document.querySelectorAll('.mob-tab');

function setMobTab(tab) {
  tabs.forEach(t => t.classList.toggle('active', t.dataset.tab === tab));
  const isMobile = window.matchMedia('(max-width:767px)').matches;
  if (!isMobile) return;
  if (tab === '3d') {
    col3d.classList.remove('mob-hidden');
    panels.classList.add('mob-hidden');
    mobQuick.classList.remove('mob-hidden');
  } else {
    col3d.classList.add('mob-hidden');
    panels.classList.remove('mob-hidden');
    mobQuick.classList.add('mob-hidden');
    /* Show only cards matching the active tab */
    panels.querySelectorAll('.card[data-panel]').forEach(c => {
      c.style.display = c.dataset.panel === tab ? '' : 'none';
    });
  }
  resize();
}
tabs.forEach(t => t.addEventListener('click', () => setMobTab(t.dataset.tab)));
setMobTab('3d');

/* ── Mobile quick dpad ───────────────── */
document.getElementById('mqUp').addEventListener('click', () => sendManual('up'));
document.getElementById('mqDown').addEventListener('click', () => sendManual('down'));
document.getElementById('mqLeft').addEventListener('click', () => sendManual('right'));
document.getElementById('mqRight').addEventListener('click', () => sendManual('left'));

/* Update mobile overlay values */
const origUpdateUI = updateUI;
updateUI = function() {
  origUpdateUI();
  const mqAz = document.getElementById('mqAz');
  const mqEl = document.getElementById('mqEl');
  const mqTAz = document.getElementById('mqTAz');
  const mqTEl = document.getElementById('mqTEl');
  if (mqAz) mqAz.textContent = D.currentAz.toFixed(1) + '\xb0';
  if (mqEl) mqEl.textContent = D.currentEl.toFixed(1) + '\xb0';
  if (mqTAz) mqTAz.textContent = D.targetAz.toFixed(1) + '\xb0';
  if (mqTEl) mqTEl.textContent = D.targetEl.toFixed(1) + '\xb0';
};
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
  html.replace("%ENABLE%", state.enable ? "1" : "0");
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
  json += "\"enable\":" + String(state.enable ? "1" : "0") + ",";
  json += "\"client\":\"" + String(state.clientConnected ? "1" : "0") + "\",";
  json += "\"lastCmd\":\"" + jsonEscape(state.lastCommand) + "\"";
  json += "}";
  return json;
}

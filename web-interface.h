#pragma once

#include <Arduino.h>

// =====================================================
// WEB INTERFACE (Original Calibrator UI + New Features)
// =====================================================

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Robot Servo Calibrator & Controller</title>
<style>
:root {
    --bg-main: #0e1117;
    --bg-card: #181d27;
    --bg-subtle: #242c3d;
    --primary: #0084ff;
    --shoulder-accent: #ff8800;
    --pose-accent: #8b5cf6;
    --success: #00c853;
    --danger: #d50000;
    --text-main: #f0f4f8;
    --text-muted: #8c9ba5;
    --border-color: #2b3547;
    --border-radius: 12px;
}
* {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}
body {
    background: var(--bg-main);
    color: var(--text-main);
    font-family: Arial, Helvetica, sans-serif;
    padding: 16px;
    display: flex;
    justify-content: center;
}
.app-container {
    width: 100%;
    max-width: 680px;
    display: flex;
    flex-direction: column;
    gap: 16px;
}
header {
    text-align: center;
    padding: 12px 0 6px;
}
h1 {
    font-size: 1.55rem;
    font-weight: 700;
    color: white;
}
.ip-badge {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    background: var(--bg-subtle);
    border: 1px solid var(--border-color);
    padding: 4px 12px;
    border-radius: 20px;
    font-size: 0.85rem;
    color: #4dc3ff;
    margin-top: 8px;
}
.status-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--success);
    box-shadow: 0 0 6px var(--success);
}
.tab-nav {
    display: flex;
    flex-wrap: wrap;
    background: var(--bg-card);
    border: 1px solid var(--border-color);
    border-radius: var(--border-radius);
    padding: 4px;
    gap: 4px;
}
.tab-btn {
    flex: 1;
    min-width: 100px;
    background: transparent;
    border: none;
    padding: 10px 8px;
    color: var(--text-muted);
    font-weight: 600;
    font-size: 0.85rem;
    border-radius: 8px;
    cursor: pointer;
}
.tab-btn.active {
    background: var(--primary);
    color: white;
}
.tab-btn.active.shoulder-tab-btn {
    background: var(--shoulder-accent);
}
.tab-btn.active.pose-tab-btn {
    background: var(--pose-accent);
    color: white;
}
.tab-btn.active.spin-tab-btn {
    background: #8b5cf6;
    color: white;
}
.tab-btn.active.ota-tab-btn {
    background: #0284c7;
}
.panel {
    display: none;
    flex-direction: column;
    gap: 14px;
}
.panel.active {
    display: flex;
}
.card {
    background: var(--bg-card);
    border: 1px solid var(--border-color);
    border-radius: var(--border-radius);
    padding: 16px;
}
.card.shoulder-card {
    border-left: 4px solid var(--shoulder-accent);
}
.card.elbow-card {
    border-left: 4px solid var(--primary);
}
.card.pose-card {
    border-left: 4px solid var(--pose-accent);
}
.card.default-card {
    border-left: 4px solid var(--success);
}
.card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    gap: 10px;
    margin-bottom: 12px;
}
.servo-info {
    display: flex;
    flex-direction: column;
    gap: 4px;
}
.servo-title {
    font-weight: 700;
    font-size: 1.02rem;
}
.servo-pin {
    font-size: 0.78rem;
    color: var(--text-muted);
    line-height: 1.5;
}
.angle-display {
    font-size: 1.35rem;
    font-weight: 800;
    color: #4dc3ff;
    background: var(--bg-subtle);
    padding: 6px 10px;
    border-radius: 8px;
    min-width: 65px;
    text-align: center;
}
.shoulder-card .angle-display {
    color: #ffaa44;
}
.slider-row {
    margin: 10px 0 14px;
}
input[type=range] {
    width: 100%;
    height: 8px;
    cursor: pointer;
}
.shoulder-card input[type=range] {
    accent-color: var(--shoulder-accent);
}
.elbow-card input[type=range] {
    accent-color: var(--primary);
}
.btn-row {
    display: flex;
    gap: 6px;
    margin-top: 6px;
    flex-wrap: wrap;
}
.btn-chip {
    flex: 1;
    min-width: 46px;
    padding: 9px 5px;
    background: var(--bg-subtle);
    border: 1px solid var(--border-color);
    color: var(--text-main);
    border-radius: 6px;
    font-size: 0.82rem;
    font-weight: 600;
    cursor: pointer;
    text-align: center;
}
.btn-chip:hover {
    background: #323d54;
}
.btn-chip.highlight {
    background: rgba(255, 136, 0, 0.15);
    border-color: var(--shoulder-accent);
    color: #ffaa44;
}
.btn-chip.highlight-green {
    background: rgba(0, 200, 83, 0.12);
    border-color: var(--success);
    color: #00e664;
}
.btn-chip.highlight-blue {
    background: rgba(0, 132, 255, 0.15);
    border-color: var(--primary);
    color: #4dc3ff;
}
.btn-chip.highlight-purple {
    background: rgba(139, 92, 246, 0.15);
    border-color: var(--pose-accent);
    color: #a78bfa;
}
.action-bar {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(125px, 1fr));
    gap: 8px;
}
.action-btn {
    padding: 12px;
    border: none;
    border-radius: 9px;
    font-size: 0.9rem;
    font-weight: 700;
    cursor: pointer;
    text-align: center;
    transition: opacity 0.15s, transform 0.1s;
}
.action-btn:active {
    transform: scale(0.97);
}
.action-btn:disabled {
    opacity: 0.55;
    cursor: not-allowed;
}
.btn-accent {
    background: var(--shoulder-accent);
    color: #111;
}
.btn-blue {
    background: var(--primary);
    color: white;
}
.btn-green {
    background: var(--success);
    color: white;
}
.btn-red {
    background: var(--danger);
    color: white;
}
.btn-purple {
    background: var(--pose-accent);
    color: white;
}
.btn-dark {
    background: var(--bg-subtle);
    color: var(--text-main);
    border: 1px solid var(--border-color);
}
.default-button {
    width: 100%;
    padding: 15px;
    font-size: 1rem;
}
.default-note {
    font-size: 0.82rem;
    color: var(--text-muted);
    margin-top: 10px;
    line-height: 1.6;
}
.code-container {
    position: relative;
    background: #090c10;
    border: 1px solid var(--border-color);
    border-radius: 8px;
    padding: 14px;
    margin-top: 10px;
    font-family: monospace;
    font-size: 0.85rem;
    color: #58a6ff;
    white-space: pre-wrap;
    overflow-x: auto;
}
.copy-btn {
    position: absolute;
    top: 8px;
    right: 8px;
    background: var(--bg-subtle);
    border: 1px solid var(--border-color);
    color: white;
    padding: 6px 10px;
    border-radius: 6px;
    cursor: pointer;
}
.status-footer {
    text-align: center;
    font-size: 0.82rem;
    color: var(--text-muted);
    padding: 6px 0;
    line-height: 1.5;
}
.ota-file {
    width: 100%;
    padding: 12px;
    margin: 12px 0;
    background: #0e1117;
    border: 1px dashed #475569;
    border-radius: 8px;
    color: var(--text-muted);
}
.progress-bar {
    display: none;
    width: 100%;
    height: 12px;
    background: #334155;
    border-radius: 6px;
    overflow: hidden;
    margin: 12px 0;
}
.progress-fill {
    width: 0%;
    height: 100%;
    background: #38bdf8;
}
/* D-Pad Styling */
.dpad-grid {
    display: grid;
    grid-template-columns: 80px 80px 80px;
    grid-template-rows: 60px 60px 60px;
    gap: 8px;
    justify-content: center;
    margin: 14px auto;
}
.dpad-btn {
    background: var(--bg-subtle);
    border: 1px solid var(--border-color);
    color: white;
    font-size: 1.4rem;
    border-radius: 10px;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    user-select: none;
    -webkit-user-select: none;
}
.dpad-btn:active {
    background: var(--primary);
}
.dpad-btn.stop-btn {
    background: rgba(213, 0, 0, 0.25);
    border-color: var(--danger);
    color: #ff5252;
    font-size: 0.85rem;
    font-weight: bold;
}
.dpad-btn.stop-btn:active {
    background: var(--danger);
    color: white;
}
</style>
</head>
<body>
<div class="app-container">

<header>
    <h1>Robot Servo Calibrator</h1>
    <div class="ip-badge">
        <div class="status-dot"></div>
        <span>IP: <strong id="ipDisplay">Loading...</strong></span>
    </div>
</header>

<nav class="tab-nav">
    <button class="tab-btn active shoulder-tab-btn" onclick="openTab('shoulders',this)">
        Shoulders (4)
    </button>
    <button class="tab-btn" onclick="openTab('elbows',this)">
        Elbows & Walk (4)
    </button>
    <button class="tab-btn pose-tab-btn" onclick="openTab('poses',this)">
        Poses & D-Pad
    </button>
    <button class="tab-btn spin-tab-btn" onclick="openTab('spin360',this)">
        360 Spin
    </button>
    <button class="tab-btn" onclick="openTab('export',this)">
        Export Offsets
    </button>
    <button class="tab-btn ota-tab-btn" onclick="openTab('ota',this)">
        OTA Update
    </button>
</nav>

<!-- ================================================= -->
<!-- SET ALL DEFAULTS BUTTON                            -->
<!-- ================================================= -->
<div class="card default-card">
    <button class="action-btn btn-green default-button" id="defaultButton" onclick="presetDefault()">
        SET ALL DEFAULTS
    </button>
    <p class="default-note">
        Restores all eight servos to your calibrated defaults:
        R1=90°, R2=0°, L1=0°, L2=90°,
        R3=180°, L4=0°, L3=0°, R4=180°.
        Stops walking, 360 spin, and poses first.
    </p>
</div>

<!-- ================================================= -->
<!-- SHOULDER TAB                                      -->
<!-- ================================================= -->
<div id="panel-shoulders" class="panel active">

    <div class="card">
        <div style="font-weight:700;margin-bottom:10px;color:#ffaa44;">
            Shoulder Presets
        </div>
        <div class="action-bar">
            <button class="action-btn btn-accent" onclick="presetShoulderWalk()">
                Shoulder Walk Stance (All 45°)
            </button>
            <button class="action-btn btn-dark" onclick="presetDefault()">
                Set All Defaults
            </button>
        </div>
    </div>

    <!-- R1 SHOULDER -->
    <div class="card shoulder-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Back-Right Shoulder (R1)</div>
                <div class="servo-pin">GPIO 12 · Allowed: 0°–90° · Default: 90°</div>
            </div>
            <div class="angle-display" id="disp_R1_SHOUL_Servo">90°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="R1_SHOUL_Servo" min="0" max="90" value="90" oninput="onSliderMove('R1_SHOUL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('R1_SHOUL_Servo',-5)">-5°</button>
            <button class="btn-chip" onclick="nudge('R1_SHOUL_Servo',-1)">-1°</button>
            <button class="btn-chip highlight-green" onclick="setExact('R1_SHOUL_Servo',90)">90° Default</button>
            <button class="btn-chip" onclick="nudge('R1_SHOUL_Servo',1)">+1°</button>
            <button class="btn-chip" onclick="nudge('R1_SHOUL_Servo',5)">+5°</button>
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="setExact('R1_SHOUL_Servo',0)">0°</button>
            <button class="btn-chip" onclick="setExact('R1_SHOUL_Servo',30)">30°</button>
            <button class="btn-chip highlight-orange" style="background:rgba(255,136,0,0.18);border-color:#ff8800;color:#ffaa44;" onclick="setExact('R1_SHOUL_Servo',45)">45° Walk</button>
            <button class="btn-chip" onclick="setExact('R1_SHOUL_Servo',60)">60°</button>
            <button class="btn-chip" onclick="setExact('R1_SHOUL_Servo',90)">90°</button>
        </div>
    </div>

    <!-- R2 SHOULDER -->
    <div class="card shoulder-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Back-Left Shoulder (R2)</div>
                <div class="servo-pin">GPIO 15 · Allowed: 0°–90° · Default: 0°</div>
            </div>
            <div class="angle-display" id="disp_R2_SHOUL_Servo">0°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="R2_SHOUL_Servo" min="0" max="90" value="0" oninput="onSliderMove('R2_SHOUL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('R2_SHOUL_Servo',-5)">-5°</button>
            <button class="btn-chip" onclick="nudge('R2_SHOUL_Servo',-1)">-1°</button>
            <button class="btn-chip highlight-green" onclick="setExact('R2_SHOUL_Servo',0)">0° Default</button>
            <button class="btn-chip" onclick="nudge('R2_SHOUL_Servo',1)">+1°</button>
            <button class="btn-chip" onclick="nudge('R2_SHOUL_Servo',5)">+5°</button>
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="setExact('R2_SHOUL_Servo',0)">0°</button>
            <button class="btn-chip" onclick="setExact('R2_SHOUL_Servo',30)">30°</button>
            <button class="btn-chip highlight-orange" style="background:rgba(255,136,0,0.18);border-color:#ff8800;color:#ffaa44;" onclick="setExact('R2_SHOUL_Servo',45)">45° Walk</button>
            <button class="btn-chip" onclick="setExact('R2_SHOUL_Servo',60)">60°</button>
            <button class="btn-chip" onclick="setExact('R2_SHOUL_Servo',90)">90°</button>
        </div>
    </div>

    <!-- L1 SHOULDER -->
    <div class="card shoulder-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Front-Left Shoulder (L1)</div>
                <div class="servo-pin">GPIO 13 · Allowed: 0°–90° · Default: 0°</div>
            </div>
            <div class="angle-display" id="disp_L1_SHOUL_Servo">0°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="L1_SHOUL_Servo" min="0" max="90" value="0" oninput="onSliderMove('L1_SHOUL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('L1_SHOUL_Servo',-5)">-5°</button>
            <button class="btn-chip" onclick="nudge('L1_SHOUL_Servo',-1)">-1°</button>
            <button class="btn-chip highlight-green" onclick="setExact('L1_SHOUL_Servo',0)">0° Default</button>
            <button class="btn-chip" onclick="nudge('L1_SHOUL_Servo',1)">+1°</button>
            <button class="btn-chip" onclick="nudge('L1_SHOUL_Servo',5)">+5°</button>
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="setExact('L1_SHOUL_Servo',0)">0°</button>
            <button class="btn-chip" onclick="setExact('L1_SHOUL_Servo',30)">30°</button>
            <button class="btn-chip highlight-orange" style="background:rgba(255,136,0,0.18);border-color:#ff8800;color:#ffaa44;" onclick="setExact('L1_SHOUL_Servo',45)">45° Walk</button>
            <button class="btn-chip highlight" onclick="setExact('L1_SHOUL_Servo',75)">75° Hi</button>
            <button class="btn-chip" onclick="setExact('L1_SHOUL_Servo',90)">90°</button>
        </div>
    </div>

    <!-- L2 SHOULDER -->
    <div class="card shoulder-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Front-Right Shoulder (L2)</div>
                <div class="servo-pin">GPIO 2 · Allowed: 0°–90° · Default: 90°</div>
            </div>
            <div class="angle-display" id="disp_L2_SHOUL_Servo">90°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="L2_SHOUL_Servo" min="0" max="90" value="90" oninput="onSliderMove('L2_SHOUL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('L2_SHOUL_Servo',-5)">-5°</button>
            <button class="btn-chip" onclick="nudge('L2_SHOUL_Servo',-1)">-1°</button>
            <button class="btn-chip highlight-green" onclick="setExact('L2_SHOUL_Servo',90)">90° Default</button>
            <button class="btn-chip" onclick="nudge('L2_SHOUL_Servo',1)">+1°</button>
            <button class="btn-chip" onclick="nudge('L2_SHOUL_Servo',5)">+5°</button>
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="setExact('L2_SHOUL_Servo',0)">0°</button>
            <button class="btn-chip" onclick="setExact('L2_SHOUL_Servo',30)">30°</button>
            <button class="btn-chip highlight-orange" style="background:rgba(255,136,0,0.18);border-color:#ff8800;color:#ffaa44;" onclick="setExact('L2_SHOUL_Servo',45)">45° Walk</button>
            <button class="btn-chip" onclick="setExact('L2_SHOUL_Servo',60)">60°</button>
            <button class="btn-chip" onclick="setExact('L2_SHOUL_Servo',90)">90°</button>
            <button class="btn-chip highlight-purple" onclick="start360()">360 (x23)</button>
            <button class="btn-chip" style="background:rgba(239,68,68,0.15);border-color:var(--danger);color:#f87171;" onclick="stop360()">Stop 360</button>
        </div>
    </div>

</div>

<!-- ================================================= -->
<!-- ELBOW AND WALKING TAB                              -->
<!-- ================================================= -->
<div id="panel-elbows" class="panel">

    <div class="card">
        <div style="font-weight:700;margin-bottom:10px;color:#4dc3ff;">
            Walking and Leg Presets
        </div>
        <div class="action-bar" style="margin-bottom:10px;">
            <button class="action-btn btn-green" onclick="startWalking()">START WALK</button>
            <button class="action-btn btn-red" onclick="stopWalking()">STOP WALK</button>
            <button class="action-btn btn-accent" onclick="startHi()">START HI</button>
            <button class="action-btn btn-red" onclick="stopHi()">STOP HI</button>
            <button class="action-btn btn-purple" onclick="start360()">360 (L2 x23)</button>
            <button class="action-btn btn-red" onclick="stop360()">STOP 360</button>
        </div>
        <div class="action-bar">
            <button class="action-btn btn-blue" onclick="presetVertical()">Vertical (160°/20°)</button>
            <button class="action-btn btn-blue" onclick="presetElbows45()">All Elbows 45°</button>
            <button class="action-btn btn-dark" onclick="presetHorizontal()">Horizontal (90°)</button>
            <button class="action-btn btn-accent" onclick="presetShoulderWalk()">Shoulder Walk (All 45°)</button>
            <button class="action-btn btn-dark" onclick="presetDefault()">Set All Defaults</button>
        </div>
    </div>

    <!-- R3 ELBOW -->
    <div class="card elbow-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Back-Right Leg (R3 Elbow)</div>
                <div class="servo-pin">GPIO 4 · Allowed: 0°–180° · Default: 180°</div>
            </div>
            <div class="angle-display" id="disp_R3_EL_Servo">180°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="R3_EL_Servo" min="0" max="180" value="180" oninput="onSliderMove('R3_EL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('R3_EL_Servo',-5)">-5°</button>
            <button class="btn-chip highlight-blue" onclick="setExact('R3_EL_Servo',180)">180° Default</button>
            <button class="btn-chip highlight-blue" onclick="setExact('R3_EL_Servo',160)">160° Vert</button>
            <button class="btn-chip highlight-blue" onclick="setExact('R3_EL_Servo',135)">135° (45°)</button>
            <button class="btn-chip highlight" onclick="setExact('R3_EL_Servo',115)">115° Walk</button>
            <button class="btn-chip" onclick="setExact('R3_EL_Servo',90)">90° Horiz</button>
            <button class="btn-chip" onclick="setExact('R3_EL_Servo',0)">0°</button>
            <button class="btn-chip" onclick="nudge('R3_EL_Servo',5)">+5°</button>
        </div>
    </div>

    <!-- L4 ELBOW -->
    <div class="card elbow-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Front-Left Leg (L4 Elbow)</div>
                <div class="servo-pin">GPIO 14 · Allowed: 0°–180° · Default: 0°</div>
            </div>
            <div class="angle-display" id="disp_L4_EL_Servo">0°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="L4_EL_Servo" min="0" max="180" value="0" oninput="onSliderMove('L4_EL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('L4_EL_Servo',-5)">-5°</button>
            <button class="btn-chip highlight-blue" onclick="setExact('L4_EL_Servo',0)">0° Default</button>
            <button class="btn-chip highlight-blue" onclick="setExact('L4_EL_Servo',20)">20° Vert</button>
            <button class="btn-chip highlight-blue" onclick="setExact('L4_EL_Servo',45)">45°</button>
            <button class="btn-chip highlight" onclick="setExact('L4_EL_Servo',60)">60° Walk</button>
            <button class="btn-chip" onclick="setExact('L4_EL_Servo',90)">90° Horiz</button>
            <button class="btn-chip highlight" onclick="startHi()">Start Hi</button>
            <button class="btn-chip" onclick="stopHi()">Stop Hi</button>
            <button class="btn-chip" onclick="nudge('L4_EL_Servo',5)">+5°</button>
        </div>
    </div>

    <!-- L3 ELBOW -->
    <div class="card elbow-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Back-Left Leg (L3 Elbow)</div>
                <div class="servo-pin">GPIO 5 · Allowed: 0°–180° · Default: 0°</div>
            </div>
            <div class="angle-display" id="disp_L3_EL_Servo">0°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="L3_EL_Servo" min="0" max="180" value="0" oninput="onSliderMove('L3_EL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('L3_EL_Servo',-5)">-5°</button>
            <button class="btn-chip highlight-blue" onclick="setExact('L3_EL_Servo',0)">0° Default</button>
            <button class="btn-chip highlight-blue" onclick="setExact('L3_EL_Servo',20)">20° Vert</button>
            <button class="btn-chip highlight-blue" onclick="setExact('L3_EL_Servo',45)">45°</button>
            <button class="btn-chip highlight" onclick="setExact('L3_EL_Servo',65)">65° Walk</button>
            <button class="btn-chip highlight" onclick="setExact('L3_EL_Servo',105)">105° Balance</button>
            <button class="btn-chip" onclick="setExact('L3_EL_Servo',90)">90° Horiz</button>
            <button class="btn-chip" onclick="nudge('L3_EL_Servo',5)">+5°</button>
        </div>
    </div>

    <!-- R4 ELBOW -->
    <div class="card elbow-card">
        <div class="card-header">
            <div class="servo-info">
                <div class="servo-title">Front-Right Leg (R4 Elbow)</div>
                <div class="servo-pin">GPIO 27 · Allowed: 0°–180° · Default: 180°</div>
            </div>
            <div class="angle-display" id="disp_R4_EL_Servo">180°</div>
        </div>
        <div class="slider-row">
            <input type="range" id="R4_EL_Servo" min="0" max="180" value="180" oninput="onSliderMove('R4_EL_Servo')">
        </div>
        <div class="btn-row">
            <button class="btn-chip" onclick="nudge('R4_EL_Servo',-5)">-5°</button>
            <button class="btn-chip highlight-blue" onclick="setExact('R4_EL_Servo',180)">180° Default</button>
            <button class="btn-chip highlight-blue" onclick="setExact('R4_EL_Servo',160)">160° Vert</button>
            <button class="btn-chip highlight-blue" onclick="setExact('R4_EL_Servo',135)">135° (45°)</button>
            <button class="btn-chip highlight" onclick="setExact('R4_EL_Servo',120)">120° Walk</button>
            <button class="btn-chip" onclick="setExact('R4_EL_Servo',90)">90° Horiz</button>
            <button class="btn-chip" onclick="setExact('R4_EL_Servo',0)">0°</button>
            <button class="btn-chip" onclick="nudge('R4_EL_Servo',5)">+5°</button>
        </div>
    </div>

</div>

<!-- ================================================= -->
<!-- POSES & D-PAD TAB (Firmware Features Integration)  -->
<!-- ================================================= -->
<div id="panel-poses" class="panel">

    <!-- MOVEMENT D-PAD -->
    <div class="card pose-card">
        <div style="font-weight:700;margin-bottom:6px;color:#a78bfa;">
            Movement Control (D-Pad)
        </div>
        <p style="font-size:0.8rem;color:var(--text-muted);margin-bottom:8px;">
            Press or hold arrows to walk and steer. Uses your calibrated non-colliding gait.
        </p>
        <div class="dpad-grid">
            <div></div>
            <button class="dpad-btn" onmousedown="move('forward')" onmouseup="stopCmd()" ontouchstart="move('forward')" ontouchend="stopCmd()">&#9650;</button>
            <div></div>
            <button class="dpad-btn" onmousedown="move('left')" onmouseup="stopCmd()" ontouchstart="move('left')" ontouchend="stopCmd()">&#9664;</button>
            <button class="dpad-btn stop-btn" onclick="stopCmd()">STOP</button>
            <button class="dpad-btn" onmousedown="move('right')" onmouseup="stopCmd()" ontouchstart="move('right')" ontouchend="stopCmd()">&#9654;</button>
            <div></div>
            <button class="dpad-btn" onmousedown="move('backward')" onmouseup="stopCmd()" ontouchstart="move('backward')" ontouchend="stopCmd()">&#9660;</button>
            <div></div>
        </div>
    </div>

    <!-- POSES & GESTURES GRID -->
    <div class="card pose-card">
        <div style="font-weight:700;margin-bottom:10px;color:#c084fc;">
            Robot Poses & Gestures
        </div>
        <div class="action-bar">
            <button class="action-btn btn-dark" onclick="pose('rest')">Rest (Flat)</button>
            <button class="action-btn btn-blue" onclick="pose('stand')">Stand</button>
            <button class="action-btn btn-accent" onclick="pose('wave')">Wave / Hi</button>
            <button class="action-btn btn-purple" onclick="pose('spin360')">360 (x23)</button>
            <button class="action-btn btn-dark" onclick="pose('dance')">Dance</button>
            <button class="action-btn btn-dark" onclick="pose('swim')">Swim</button>
            <button class="action-btn btn-dark" onclick="pose('point')">Point</button>
            <button class="action-btn btn-dark" onclick="pose('pushup')">Pushup</button>
            <button class="action-btn btn-dark" onclick="pose('bow')">Bow</button>
            <button class="action-btn btn-dark" onclick="pose('cute')">Cute</button>
            <button class="action-btn btn-dark" onclick="pose('freaky')">Freaky</button>
            <button class="action-btn btn-dark" onclick="pose('worm')">Worm</button>
            <button class="action-btn btn-dark" onclick="pose('shake')">Shake</button>
            <button class="action-btn btn-dark" onclick="pose('shrug')">Shrug</button>
            <button class="action-btn btn-dark" onclick="pose('dead')">Dead</button>
            <button class="action-btn btn-dark" onclick="pose('crab')">Crab</button>
        </div>
        <button class="action-btn btn-red" style="width:100%;margin-top:10px;" onclick="stopCmd()">
            STOP ALL ACTIONS
        </button>
    </div>

    <!-- OLED FACE CONTROL -->
    <div class="card" style="border-left:4px solid #ec4899;">
        <div style="font-weight:700;margin-bottom:10px;color:#f472b6;">
            OLED Face Expression
        </div>
        <div style="display:flex;gap:10px;align-items:center;">
            <select id="faceSelect" style="flex:1;padding:10px;background:var(--bg-subtle);color:white;border:1px solid var(--border-color);border-radius:8px;font-size:0.92rem;">
                <option value="default">Default</option>
                <option value="happy">Happy</option>
                <option value="sad">Sad</option>
                <option value="angry">Angry</option>
                <option value="surprised">Surprised</option>
                <option value="sleepy">Sleepy</option>
                <option value="love">Love</option>
                <option value="excited">Excited</option>
                <option value="confused">Confused</option>
                <option value="thinking">Thinking</option>
                <option value="dance">Dance</option>
                <option value="rest">Rest</option>
                <option value="idle">Idle</option>
            </select>
            <button class="action-btn btn-purple" style="padding:10px 18px;" onclick="sendFace()">
                Set Face
            </button>
        </div>
    </div>

    <!-- GAIT & ANIMATION SETTINGS -->
    <div class="card" style="border-left:4px solid #14b8a6;">
        <div style="font-weight:700;margin-bottom:10px;color:#2dd4bf;">
            Gait & Speed Settings
        </div>
        <div style="display:grid;grid-template-columns:1fr 1fr;gap:12px;">
            <div>
                <label style="font-size:0.82rem;color:var(--text-muted);">Stride Delay (ms):</label>
                <input type="number" id="settingFrameDelay" value="300" style="width:100%;padding:8px;background:var(--bg-subtle);color:white;border:1px solid var(--border-color);border-radius:6px;margin-top:4px;">
            </div>
            <div>
                <label style="font-size:0.82rem;color:var(--text-muted);">Walk Cycles:</label>
                <input type="number" id="settingWalkCycles" value="10" style="width:100%;padding:8px;background:var(--bg-subtle);color:white;border:1px solid var(--border-color);border-radius:6px;margin-top:4px;">
            </div>
        </div>
        <button class="action-btn btn-blue" style="width:100%;margin-top:12px;padding:10px;" onclick="saveSettings()">
            Save Settings
        </button>
    </div>

</div>

<!-- ================================================= -->
<!-- 360 SPIN CONTROL TAB                              -->
<!-- ================================================= -->
<div id="panel-spin360" class="panel">
    <div class="card" style="border-left:4px solid #8b5cf6;">
        <div style="font-weight:700;margin-bottom:8px;color:#c084fc;font-size:1.05rem;">
            360 Spin Controller
        </div>
        <p style="font-size:0.85rem;color:var(--text-muted);margin-bottom:14px;line-height:1.5;">
            Configure cycle count and choose which shoulder performs the 360 spin gesture.
        </p>

        <div style="margin-bottom:14px;">
            <label style="display:block;font-size:0.85rem;color:var(--text-muted);margin-bottom:6px;font-weight:600;">
                Number of Spin Cycles (Times):
            </label>
            <div style="display:flex;align-items:center;gap:10px;">
                <input type="number" id="spinCyclesInput" value="23" min="1" max="200"
                       style="background:var(--bg-subtle);border:2px solid #8b5cf6;color:#fff;padding:8px 12px;border-radius:8px;font-size:1.15rem;font-weight:700;width:110px;text-align:center;">
                <span style="color:var(--text-muted);font-size:0.85rem;">cycles (strokes)</span>
            </div>
            <div style="display:flex;gap:6px;flex-wrap:wrap;margin-top:10px;">
                <button class="btn-chip" onclick="setSpinCycles(5)">5x</button>
                <button class="btn-chip" onclick="setSpinCycles(10)">10x</button>
                <button class="btn-chip" onclick="setSpinCycles(15)">15x</button>
                <button class="btn-chip highlight-purple" onclick="setSpinCycles(23)">23x (Default)</button>
                <button class="btn-chip" onclick="setSpinCycles(35)">35x</button>
                <button class="btn-chip" onclick="setSpinCycles(50)">50x</button>
            </div>
        </div>

        <div style="margin-bottom:18px;">
            <label style="display:block;font-size:0.85rem;color:var(--text-muted);margin-bottom:6px;font-weight:600;">
                Active Spinning Shoulder:
            </label>
            <div style="display:flex;gap:6px;flex-wrap:wrap;">
                <button class="btn-chip highlight-purple" id="spinShBtn_L2" onclick="setSpinShoulder('L2')">L2 (Default)</button>
                <button class="btn-chip" id="spinShBtn_R1" onclick="setSpinShoulder('R1')">R1</button>
                <button class="btn-chip" id="spinShBtn_L1" onclick="setSpinShoulder('L1')">L1</button>
                <button class="btn-chip" id="spinShBtn_R2" onclick="setSpinShoulder('R2')">R2</button>
                <button class="btn-chip" id="spinShBtn_ALL" onclick="setSpinShoulder('ALL')">All 4 Shoulders</button>
            </div>
        </div>

        <div class="action-bar" style="margin-top:10px;">
            <button class="action-btn btn-purple" style="font-size:1.05rem;padding:12px;font-weight:700;" onclick="start360FromTab()">
                START 360 SPIN
            </button>
            <button class="action-btn btn-red" style="font-size:1.05rem;padding:12px;font-weight:700;" onclick="stop360()">
                STOP 360
            </button>
        </div>
        <div id="spinStatusDisplay" style="margin-top:12px;font-size:0.9rem;font-weight:600;color:#c084fc;">
            Ready
        </div>
    </div>

    <div class="card" style="border-left:4px solid #38bdf8;">
        <div style="font-weight:700;margin-bottom:8px;color:#38bdf8;">
            Configured 360 Stance
        </div>
        <div style="font-size:0.85rem;color:var(--text-muted);line-height:1.7;">
            <div><strong>Shoulders Stance:</strong> L2=70° | R1=70° | R2=20° | L1=20°</div>
            <div><strong>Elbows Stance:</strong> R3=160° | L4=20° | L3=20° | R4=180°</div>
            <div><strong>Behavior:</strong> The non-spinning legs hold stance at 70°/20° while the active leg strokes towards 0° and returns to stance each cycle.</div>
        </div>
    </div>
</div>

<!-- ================================================= -->
<!-- EXPORT CALIBRATION TAB                             -->
<!-- ================================================= -->
<div id="panel-export" class="panel">
    <div class="card">
        <div style="font-weight:700;margin-bottom:8px;color:#4dc3ff;">
            Calibrated Angle Definitions
        </div>
        <p style="font-size:0.85rem;color:var(--text-muted);margin-bottom:12px;line-height:1.5;">
            Adjust each servo, then copy these positions into your sketch.
            Shoulder angles are restricted to 0°–90°.
        </p>
        <div class="code-container">
            <button class="copy-btn" onclick="copyCode()">Copy Code</button>
            <pre id="exportedCode">// Calibrated Values</pre>
        </div>
    </div>
</div>

<!-- ================================================= -->
<!-- OTA UPDATE TAB                                     -->
<!-- ================================================= -->
<div id="panel-ota" class="panel">
    <div class="card" style="border-left:4px solid #38bdf8;">
        <div style="font-weight:700;margin-bottom:8px;color:#38bdf8;">
            Wireless Firmware Update
        </div>
        <p style="font-size:0.88rem;color:var(--text-muted);line-height:1.5;">
            Upload a compiled .bin firmware file over Wi-Fi.
        </p>
        <input class="ota-file" type="file" id="otaFileInput" accept=".bin">
        <div id="otaProgressBar" class="progress-bar">
            <div id="otaProgressFill" class="progress-fill"></div>
        </div>
        <div class="action-bar">
            <button class="action-btn btn-green" id="otaUploadBtn" onclick="uploadFirmware()">
                Upload & Flash Firmware
            </button>
            <button class="action-btn btn-dark" onclick="window.open('/update','_blank')">
                Dedicated Page
            </button>
        </div>
        <div id="otaStatusText" style="margin-top:14px;font-size:0.9rem;line-height:1.5;"></div>
        <p style="margin-top:16px;font-size:0.82rem;line-height:1.6;color:var(--text-muted);">
            In Arduino IDE, use Sketch → Export Compiled Binary.
            Select the generated .bin file here and start the upload.
        </p>
    </div>
</div>

<div class="status-footer" id="statusMessage">
    Connected to ESP32
</div>

<div style="text-align:center;padding:10px 0;">
    <a href="/update" style="color:#38bdf8;text-decoration:none;font-size:0.85rem;">
        Dedicated OTA Page
    </a>
</div>

</div>

<script>
// Tab Switching
function openTab(tabId, element) {
    document.querySelectorAll('.panel').forEach(panel => {
        panel.classList.remove('active');
    });
    document.querySelectorAll('.tab-btn').forEach(button => {
        button.classList.remove('active');
    });
    document.getElementById('panel-' + tabId).classList.add('active');
    if (element) {
        element.classList.add('active');
    }
    if (tabId === 'export') {
        updateExportCode();
    }
}

// Servo Defaults
const servoDefaults = {
    R1_SHOUL_Servo: 90,
    R2_SHOUL_Servo: 0,
    L1_SHOUL_Servo: 0,
    L2_SHOUL_Servo: 90,
    R3_EL_Servo: 180,
    L4_EL_Servo: 0,
    L3_EL_Servo: 0,
    R4_EL_Servo: 180
};

function getMaxForServo(name) {
    if (name.includes('_SHOUL_')) return 90;
    return 180;
}

function updateDisplay(servoName, angle) {
    const disp = document.getElementById('disp_' + servoName);
    if (disp) disp.innerText = angle + '°';
    const slider = document.getElementById(servoName);
    if (slider) slider.value = angle;
}

function setExact(servoName, angle) {
    const max = getMaxForServo(servoName);
    angle = Math.max(0, Math.min(max, parseInt(angle)));
    updateDisplay(servoName, angle);
    fetch('/servo?name=' + servoName + '&angle=' + angle)
        .then(r => r.text())
        .then(res => {
            document.getElementById('statusMessage').innerText = servoName + ' set to ' + angle + '°';
        })
        .catch(console.error);
}

function nudge(servoName, delta) {
    const slider = document.getElementById(servoName);
    if (!slider) return;
    const current = parseInt(slider.value);
    setExact(servoName, current + delta);
}

function onSliderMove(servoName) {
    const slider = document.getElementById(servoName);
    if (!slider) return;
    const angle = parseInt(slider.value);
    const disp = document.getElementById('disp_' + servoName);
    if (disp) disp.innerText = angle + '°';
    fetch('/servo?name=' + servoName + '&angle=' + angle).catch(console.error);
}

// Presets
function presetDefault() {
    fetch('/preset?mode=default')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = msg;
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

function presetVertical() {
    fetch('/preset?mode=vertical')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = 'Vertical stance set';
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

function presetElbows45() {
    fetch('/preset?mode=elbow_45')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = 'All elbows 45° set';
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

function presetHorizontal() {
    fetch('/preset?mode=horizontal')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = 'Horizontal (90°) set';
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

function presetShoulderWalk() {
    fetch('/preset?mode=shoulder_walk')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = 'Shoulder walk stance set';
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

// Walking
function startWalking() {
    fetch('/walk?state=1')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = 'Walking started.';
        })
        .catch(console.error);
}

function stopWalking() {
    fetch('/walk?state=0')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = 'Walking stopped.';
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

// Say Hi
function startHi() {
    fetch('/hi?state=1')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = msg;
        })
        .catch(console.error);
}

function stopHi() {
    fetch('/hi?state=0')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = msg;
            setTimeout(refreshAllAngles, 150);
        })
        .catch(console.error);
}

// 360 Spin
let selectedSpinShoulder = 'L2';

function setSpinCycles(n) {
    const inp = document.getElementById('spinCyclesInput');
    if (inp) inp.value = n;
}

function setSpinShoulder(sh) {
    selectedSpinShoulder = sh;
    ['L2', 'R1', 'L1', 'R2', 'ALL'].forEach(s => {
        const btn = document.getElementById('spinShBtn_' + s);
        if (btn) {
            if (s === sh) {
                btn.classList.add('highlight-purple');
            } else {
                btn.classList.remove('highlight-purple');
            }
        }
    });
}

function start360FromTab() {
    const inp = document.getElementById('spinCyclesInput');
    const cycles = inp ? (parseInt(inp.value) || 23) : 23;
    start360(cycles, selectedSpinShoulder);
}

function start360(customCycles, customShoulder) {
    let cycles = customCycles;
    if (!cycles) {
        const inp = document.getElementById('spinCyclesInput');
        cycles = inp ? (parseInt(inp.value) || 23) : 23;
    }
    let shoulder = customShoulder || selectedSpinShoulder || 'L2';
    let url = '/360?state=1&cycles=' + encodeURIComponent(cycles) + '&shoulder=' + encodeURIComponent(shoulder);
    fetch(url)
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = msg;
            const disp = document.getElementById('spinStatusDisplay');
            if (disp) disp.innerText = msg;
        })
        .catch(console.error);
}

function stop360() {
    fetch('/360?state=0')
        .then(r => r.text())
        .then(msg => {
            document.getElementById('statusMessage').innerText = msg;
            const disp = document.getElementById('spinStatusDisplay');
            if (disp) disp.innerText = '360 Spin stopped';
            setTimeout(refreshAllAngles, 150);
        })
        .catch(console.error);
}

// D-Pad and Poses Control
function move(dir) {
    fetch('/cmd?go=' + dir)
        .then(() => {
            document.getElementById('statusMessage').innerText = 'Moving: ' + dir;
        })
        .catch(console.error);
}

function pose(name) {
    fetch('/cmd?pose=' + name)
        .then(() => {
            document.getElementById('statusMessage').innerText = 'Pose: ' + name;
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

function stopCmd() {
    fetch('/cmd?stop=1')
        .then(() => {
            document.getElementById('statusMessage').innerText = 'Stopped.';
            setTimeout(refreshAllAngles, 100);
        })
        .catch(console.error);
}

function sendFace() {
    const face = document.getElementById('faceSelect').value;
    fetch('/cmd?face=' + face)
        .then(() => {
            document.getElementById('statusMessage').innerText = 'OLED Face set to: ' + face;
        })
        .catch(console.error);
}

function saveSettings() {
    const fd = document.getElementById('settingFrameDelay').value;
    const wc = document.getElementById('settingWalkCycles').value;
    fetch('/setSettings?frameDelay=' + fd + '&walkCycles=' + wc)
        .then(() => {
            document.getElementById('statusMessage').innerText = 'Gait settings updated.';
        })
        .catch(console.error);
}

// Refresh Angles from ESP32
function refreshAllAngles() {
    fetch('/angles')
        .then(r => r.json())
        .then(angles => {
            for (let name in angles) {
                updateDisplay(name, angles[name]);
            }
        })
        .catch(() => {});
}

// Export Offsets Code
function updateExportCode() {
    const r1 = document.getElementById('R1_SHOUL_Servo').value;
    const r2 = document.getElementById('R2_SHOUL_Servo').value;
    const l1 = document.getElementById('L1_SHOUL_Servo').value;
    const l2 = document.getElementById('L2_SHOUL_Servo').value;
    const r3 = document.getElementById('R3_EL_Servo').value;
    const l4 = document.getElementById('L4_EL_Servo').value;
    const l3 = document.getElementById('L3_EL_Servo').value;
    const r4 = document.getElementById('R4_EL_Servo').value;

    const code =
`// Calibrated shoulder positions (0-90 deg)
#define CALIB_R1_SHOULDER ${r1}  // GPIO 12
#define CALIB_R2_SHOULDER ${r2}  // GPIO 15
#define CALIB_L1_SHOULDER ${l1}  // GPIO 13
#define CALIB_L2_SHOULDER ${l2}  // GPIO 2

// Calibrated elbow positions (0-180 deg)
#define CALIB_R3_ELBOW    ${r3}  // GPIO 4
#define CALIB_L4_ELBOW    ${l4}  // GPIO 14
#define CALIB_L3_ELBOW    ${l3}  // GPIO 5
#define CALIB_R4_ELBOW    ${r4}  // GPIO 27`;

    document.getElementById('exportedCode').innerText = code;
}

function copyCode() {
    const text = document.getElementById('exportedCode').innerText;
    navigator.clipboard.writeText(text)
        .then(() => alert('Copied to clipboard!'))
        .catch(() => alert('Copy failed, please copy manually.'));
}

// Display IP
fetch('/ip')
    .then(r => r.text())
    .then(ip => { document.getElementById('ipDisplay').innerText = ip; })
    .catch(() => { document.getElementById('ipDisplay').innerText = 'Connected'; });

// Status Polling
setInterval(() => {
    fetch('/walkstatus')
        .then(r => r.text())
        .then(st => {
            const msg = document.getElementById('statusMessage');
            if (st === '1') msg.innerText = 'Walking in progress...';
            else if (st === '2') msg.innerText = 'Waving Hi...';
        })
        .catch(() => {});
}, 1000);

// Browser OTA Upload
function uploadFirmware() {
    const input = document.getElementById('otaFileInput');
    const file = input.files[0];
    if (!file) {
        alert('Please select a compiled .bin file first.');
        return;
    }
    const bar = document.getElementById('otaProgressBar');
    const fill = document.getElementById('otaProgressFill');
    const status = document.getElementById('otaStatusText');
    const button = document.getElementById('otaUploadBtn');

    bar.style.display = 'block';
    button.disabled = true;
    status.style.color = '#38bdf8';
    status.innerText = 'Uploading firmware...';

    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/update');
    xhr.upload.onprogress = function(e) {
        if (e.lengthComputable) {
            const pct = Math.round((e.loaded / e.total) * 100);
            fill.style.width = pct + '%';
            status.innerText = 'Flashing firmware: ' + pct + '%';
        }
    };
    xhr.onload = function() {
        if (xhr.status === 200) {
            status.style.color = '#00c853';
            status.innerText = 'Update successful! Robot is rebooting...';
            setTimeout(() => location.reload(), 8000);
        } else {
            status.style.color = '#d50000';
            status.innerText = 'Update failed: ' + xhr.responseText;
            button.disabled = false;
        }
    };
    xhr.onerror = function() {
        status.style.color = '#d50000';
        status.innerText = 'Upload error occurred.';
        button.disabled = false;
    };
    const fd = new FormData();
    fd.append('update', file);
    xhr.send(fd);
}

// Initial sync
refreshAllAngles();
</script>
</body>
</html>
)rawliteral";

// Dedicated OTA Update Page
const char OTA_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sesame Robot - Firmware OTA Update</title>
<style>
body { background: #0e1117; color: #f0f4f8; font-family: sans-serif; display: flex; justify-content: center; padding: 25px; }
.box { background: #181d27; border: 1px solid #2b3547; border-radius: 12px; padding: 24px; max-width: 450px; width: 100%; text-align: center; }
h2 { color: #38bdf8; margin-bottom: 15px; }
input[type=file] { margin: 15px 0; color: #8c9ba5; }
button { background: #00c853; color: white; border: none; padding: 12px 20px; border-radius: 8px; font-weight: bold; cursor: pointer; width: 100%; font-size: 1rem; }
.bar { display: none; width: 100%; height: 12px; background: #334155; border-radius: 6px; overflow: hidden; margin: 15px 0; }
.fill { width: 0%; height: 100%; background: #38bdf8; }
#status { margin-top: 15px; font-size: 0.9rem; }
a { color: #38bdf8; text-decoration: none; font-size: 0.85rem; display: inline-block; margin-top: 15px; }
</style>
</head>
<body>
<div class="box">
  <h2>Firmware OTA Update</h2>
  <p style="color:#8c9ba5;font-size:0.88rem;">Select a compiled <code>.bin</code> file and upload to flash wirelessly.</p>
  <input type="file" id="f" accept=".bin">
  <div class="bar" id="bar"><div class="fill" id="fill"></div></div>
  <button id="btn" onclick="up()">Upload & Flash</button>
  <div id="status"></div>
  <a href="/">← Return to Main Controller</a>
</div>
<script>
function up() {
  const f = document.getElementById('f').files[0];
  if (!f) return alert('Select .bin file');
  const bar = document.getElementById('bar');
  const fill = document.getElementById('fill');
  const st = document.getElementById('status');
  const btn = document.getElementById('btn');
  bar.style.display = 'block';
  btn.disabled = true;
  st.innerText = 'Uploading...';
  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/update');
  xhr.upload.onprogress = e => {
    if (e.lengthComputable) {
      const p = Math.round((e.loaded / e.total) * 100);
      fill.style.width = p + '%';
      st.innerText = 'Flashing: ' + p + '%';
    }
  };
  xhr.onload = () => {
    if (xhr.status === 200) {
      st.style.color = '#00c853';
      st.innerText = 'Success! Rebooting...';
      setTimeout(() => location.href = '/', 8000);
    } else {
      st.style.color = '#d50000';
      st.innerText = 'Failed: ' + xhr.responseText;
      btn.disabled = false;
    }
  };
  const fd = new FormData();
  fd.append('update', f);
  xhr.send(fd);
}
</script>
</body>
</html>
)rawliteral";

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>
#include <Update.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "face-bitmaps.h"
#include "movement-sequences.h"
#include "web-interface.h"

// --- Access Point Configuration ---
#define AP_SSID  "Sesame-Controller"
#define AP_PASS  "12345678" // Must be at least 8 characters

// --- Station Mode Configuration (from sesame_reobo.ino) ---
#define NETWORK_SSID "EMCAccess2.4"
#define NETWORK_PASS "@!~electro123"
#define ENABLE_NETWORK_MODE true  // Connects to local WiFi if available

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDR 0x3C

// I2C Pins for standard ESP32 (SDA = 21, SCL = 22)
#define I2C_SDA 21
#define I2C_SCL 22

DNSServer dnsServer;
const byte DNS_PORT = 53;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool hasDisplay = false; // Flag to indicate if SSD1306 display is present
WebServer server(80);

// Global state for animations
String currentCommand = "";
String currentFaceName = "default";
const unsigned char* const* currentFaceFrames = nullptr;
uint8_t currentFaceFrameCount = 0;
uint8_t currentFaceFrameIndex = 0;
unsigned long lastFaceFrameMs = 0;
int faceFps = 8;
FaceAnimMode currentFaceMode = FACE_ANIM_LOOP;
int8_t faceFrameDirection = 1;
bool faceAnimFinished = false;
int currentFaceFps = 0;
bool idleActive = false;
bool idleBlinkActive = false;
unsigned long nextIdleBlinkMs = 0;
uint8_t idleBlinkRepeatsLeft = 0;
int spin360Shoulder = L2;
int spin360Cycles = 23;

// WiFi Info Scrolling
unsigned long lastInputTime = 0;
bool firstInputReceived = false;
bool showingWifiInfo = false;
int wifiScrollPos = 0;
unsigned long lastWifiScrollMs = 0;
String wifiInfoText = "";

// Network Mode
bool networkConnected = false;
IPAddress networkIP;
String deviceHostname = "sesame-robot";
bool mdnsOk = false;

// Runtime WiFi provisioning
enum WifiSetupState { WIFI_SETUP_IDLE, WIFI_SETUP_QUEUED, WIFI_SETUP_CONNECTING };
WifiSetupState wifiSetupState = WIFI_SETUP_IDLE;
String wifiSetupSsid = "";
String wifiSetupPass = "";
String wifiSetupError = "";
unsigned long wifiSetupQueuedMs = 0;
unsigned long wifiSetupStartMs = 0;
bool wifiRestoreApOnly = false;
const uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
const uint32_t WIFI_SETUP_START_DELAY_MS = 300;

// ======================================================================
// HARDWARE GPIO PIN CONFIGURATION (preserved from sesame_reobo.ino)
//
// In movement-sequences.h:
// enum ServoName { R1=0, R2=1, L1=2, L2=3, R4=4, R3=5, L3=6, L4=7 };
//
// Mapping:
// #define R1_SHOULDER    2 -> servoPins[0] (R1)
// #define R2_SHOULDER    12 -> servoPins[1] (R2)
// #define L1_SHOULDER    15 -> servoPins[2] (L1)
// #define L2_SHOULDER    13  -> servoPins[3] (L2)
// #define R4_ELBOW       5 -> servoPins[4] (R4)
// #define R3_ELBOW       27  -> servoPins[5] (R3)
// #define L3_ELBOW       14 -> servoPins[6] (L3)
// #define L4_ELBOW       4 -> servoPins[7] (L4)
// ======================================================================
Servo servos[8];
const int servoPins[8] = {
    2,   // Index 0: R1 - Right shoulder
    12,  // Index 1: R2 - Right shoulder
    15,  // Index 2: L1 - Left shoulder
    13,  // Index 3: L2 - Left shoulder
    5,   // Index 4: R4 - Right elbow
    27,  // Index 5: R3 - Right elbow
    14,  // Index 6: L3 - Left elbow
    4    // Index 7: L4 - Left elbow
};
// Track current angles for UI synchronization
int currentAngles[8] = {90, 0, 0, 90, 180, 180, 0, 0};

// Subtrim values for each servo (offset in degrees)
int8_t servoSubtrim[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Animation constants (calibrated to match DD=300ms)
int frameDelay = 300;
int walkCycles = 10;
int motorCurrentDelay = 20;

struct FaceEntry {
  const char* name;
  const unsigned char* const* frames;
  uint8_t maxFrames;
};

static const uint8_t MAX_FACE_FRAMES = 6;

#define MAKE_FACE_FRAMES(name) \
  const unsigned char* const face_##name##_frames[] = { \
    epd_bitmap_##name, epd_bitmap_##name##_1, epd_bitmap_##name##_2, \
    epd_bitmap_##name##_3, epd_bitmap_##name##_4, epd_bitmap_##name##_5 \
  };

#define X(name) MAKE_FACE_FRAMES(name)
FACE_LIST
#undef X
#undef MAKE_FACE_FRAMES

const FaceEntry faceEntries[] = {
#define X(name) { #name, face_##name##_frames, MAX_FACE_FRAMES },
  FACE_LIST
#undef X
  { "default", face_defualt_frames, MAX_FACE_FRAMES }
};

struct FaceFpsEntry {
  const char* name;
  uint8_t fps;
};

const FaceFpsEntry faceFpsEntries[] = {
  { "walk", 1 },
  { "rest", 1 },
  { "swim", 1 },
  { "dance", 1 },
  { "wave", 1 },
  { "point", 5 },
  { "stand", 1 },
  { "cute", 1 },
  { "pushup", 1 },
  { "freaky", 1 },
  { "bow", 1 },
  { "worm", 1 },
  { "shake", 1 },
  { "shrug", 1 },
  { "dead", 2 },
  { "crab", 1 },
  { "idle", 1 },
  { "idle_blink", 7 },
  { "default", 1 },
  { "happy", 1 },
  { "talk_happy", 1 },
  { "sad", 1 },
  { "talk_sad", 1 },
  { "angry", 1 },
  { "talk_angry", 1 },
  { "surprised", 1 },
  { "talk_surprised", 1 },
  { "sleepy", 1 },
  { "talk_sleepy", 1 },
  { "love", 1 },
  { "talk_love", 1 },
  { "excited", 1 },
  { "talk_excited", 1 },
  { "confused", 1 },
  { "talk_confused", 1 },
  { "thinking", 1 },
  { "talk_thinking", 1 },
};

// Prototypes
void setServoAngle(uint8_t channel, int angle);
void updateFaceBitmap(const unsigned char* bitmap);
void setFace(const String& faceName);
void setFaceMode(FaceAnimMode mode);
void setFaceWithMode(const String& faceName, FaceAnimMode mode);
void updateAnimatedFace();
void delayWithFace(unsigned long ms);
void enterIdle();
void exitIdle();
void updateIdleBlink();
int getFaceFpsForName(const String& faceName);
bool pressingCheck(String cmd, int ms);

void handleRoot();
void handleServo();
void handlePreset();
void handleWalk();
void handleWalkStatus();
void handleHi();
void handle360();
void handleIP();
void handleAngles();

void handleCommandWeb();
void handleGetSettings();
void handleSetSettings();
void handleGetStatus();
void handleApiCommand();
void updateWifiInfoScroll();
void recordInput();
bool connectToWifi(const String& ssid, const String& pass, uint32_t timeoutMs = 10000);
void handleWifiScan();
void handleWifiConnect();
void handleWifiStatus();
void handleNotFound();
String jsonEscape(const String& s);
bool startMdns();
void announceNetwork(const String& ssid);
void setApOnlyInfoText();
void showWifiInfoNow();
void updateWifiSetup();
void finishWifiSetup(const String& err);

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// =====================================================
// HTTP HANDLERS FOR ORIGINAL CALIBRATOR UI
// =====================================================

void handleServo() {
  if (!server.hasArg("name") || !server.hasArg("angle")) {
    server.send(400, "text/plain", "Missing parameters");
    return;
  }
  String name = server.arg("name");
  int angle = server.arg("angle").toInt();
  int idx = -1;
  if (name == "R1_SHOUL_Servo") idx = R1;
  else if (name == "R2_SHOUL_Servo") idx = R2;
  else if (name == "L1_SHOUL_Servo") idx = L1;
  else if (name == "L2_SHOUL_Servo") idx = L2;
  else if (name == "R3_EL_Servo") idx = R3;
  else if (name == "L4_EL_Servo") idx = L4;
  else if (name == "L3_EL_Servo") idx = L3;
  else if (name == "R4_EL_Servo") idx = R4;

  if (idx >= 0) {
    setServoAngle(idx, angle);
    recordInput();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Unknown servo");
  }
}

void handlePreset() {
  if (!server.hasArg("mode")) {
    server.send(400, "text/plain", "Missing mode");
    return;
  }
  String mode = server.arg("mode");
  if (mode == "default" || mode == "defaults") {
    currentCommand = "";
    runRestPose();
    server.send(200, "text/plain", "All eight servos reset to calibrated defaults.");
  } else if (mode == "vertical") {
    currentCommand = "";
    setServoAngle(R3, 160);
    setServoAngle(L4, 20);
    setServoAngle(L3, 20);
    setServoAngle(R4, 160);
    server.send(200, "text/plain", "Vertical stance applied (R3=160°, L4=20°, L3=20°, R4=160°)");
  } else if (mode == "horizontal") {
    currentCommand = "";
    setServoAngle(R3, 90); setServoAngle(L4, 90); setServoAngle(L3, 90); setServoAngle(R4, 90);
    server.send(200, "text/plain", "Horizontal (90°) applied");
  } else if (mode == "elbow_45" || mode == "45") {
    currentCommand = "";
    setServoAngle(L4, 45); setServoAngle(L3, 45);
    setServoAngle(R4, 135); setServoAngle(R3, 135);
    server.send(200, "text/plain", "All Elbows 45° applied (L=45°, R=135°)");
  } else if (mode == "shoulder_walk") {
    setServoAngle(R1, 45); setServoAngle(R2, 45); setServoAngle(L1, 45); setServoAngle(L2, 45);
    server.send(200, "text/plain", "Shoulder walk stance applied (All 45°)");
  } else if (mode == "hi" || mode == "say_hi" || mode == "start_hi") {
    currentCommand = "wave";
    recordInput();
    server.send(200, "text/plain", "Say Hi started");
  } else if (mode == "stop_hi") {
    currentCommand = "";
    recordInput();
    runStandPose(1);
    server.send(200, "text/plain", "Say Hi stopped");
  } else if (mode.startsWith("360") || mode.startsWith("spin360") || mode == "start_360") {
    spin360Shoulder = L2;
    if (server.hasArg("shoulder")) {
      String sh = server.arg("shoulder");
      sh.toUpperCase();
      if (sh == "R1") spin360Shoulder = R1;
      else if (sh == "R2") spin360Shoulder = R2;
      else if (sh == "L1") spin360Shoulder = L1;
      else if (sh == "L2") spin360Shoulder = L2;
      else if (sh == "ALL" || sh == "4") spin360Shoulder = -1;
    } else if (mode == "360_r1" || mode == "spin360_r1") spin360Shoulder = R1;
    else if (mode == "360_r2" || mode == "spin360_r2") spin360Shoulder = R2;
    else if (mode == "360_l1" || mode == "spin360_l1") spin360Shoulder = L1;
    else if (mode == "360_l2" || mode == "spin360_l2") spin360Shoulder = L2;
    else if (mode == "360_all" || mode == "spin360_all") spin360Shoulder = -1;

    if (server.hasArg("cycles")) {
      int c = server.arg("cycles").toInt();
      if (c > 0) spin360Cycles = c;
    }

    currentCommand = "spin360";
    recordInput();
    exitIdle();
    String shName = (spin360Shoulder == -1) ? "All 4 Shoulders" : ServoNames[spin360Shoulder];
    server.send(200, "text/plain", "360 Spin started with " + shName + " (" + String(spin360Cycles) + " cycles)");
  } else if (mode == "stop_360" || mode == "stop360") {
    currentCommand = "";
    recordInput();
    runStandPose(1);
    server.send(200, "text/plain", "360 Spin stopped");
  } else {
    server.send(400, "text/plain", "Unknown preset mode");
  }
}

void handleWalk() {
  int state = 1;
  if (server.hasArg("state")) state = server.arg("state").toInt();
  if (state == 1) {
    currentCommand = "forward";
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "Walking");
  } else {
    currentCommand = "";
    recordInput();
    runStandPose(1);
    server.send(200, "text/plain", "Stopped");
  }
}

void handleWalkStatus() {
  if (currentCommand == "forward" || currentCommand == "backward" || currentCommand == "left" || currentCommand == "right") {
    server.send(200, "text/plain", "1");
  } else if (currentCommand == "wave") {
    server.send(200, "text/plain", "2");
  } else {
    server.send(200, "text/plain", "0");
  }
}

void handleHi() {
  int state = 1;
  if (server.hasArg("state")) state = server.arg("state").toInt();
  if (state == 1) {
    currentCommand = "wave";
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "Say Hi started");
  } else {
    currentCommand = "";
    recordInput();
    runStandPose(1);
    server.send(200, "text/plain", "Say Hi stopped");
  }
}

void handle360() {
  int state = 1;
  if (server.hasArg("state")) state = server.arg("state").toInt();
  if (server.hasArg("shoulder")) {
    String sh = server.arg("shoulder");
    sh.toUpperCase();
    if (sh == "R1") spin360Shoulder = R1;
    else if (sh == "R2") spin360Shoulder = R2;
    else if (sh == "L1") spin360Shoulder = L1;
    else if (sh == "L2") spin360Shoulder = L2;
    else if (sh == "ALL" || sh == "4") spin360Shoulder = -1;
  }
  if (server.hasArg("cycles")) {
    int c = server.arg("cycles").toInt();
    if (c > 0) spin360Cycles = c;
  }
  if (state == 1) {
    currentCommand = "spin360";
    recordInput();
    exitIdle();
    String shName = (spin360Shoulder == -1) ? "All 4 Shoulders" : ServoNames[spin360Shoulder];
    server.send(200, "text/plain", "360 Spin started with " + shName + " (" + String(spin360Cycles) + " cycles)");
  } else {
    currentCommand = "";
    recordInput();
    runStandPose(1);
    server.send(200, "text/plain", "360 Spin stopped");
  }
}

void handleIP() {
  if (networkConnected) {
    server.send(200, "text/plain", networkIP.toString());
  } else {
    server.send(200, "text/plain", WiFi.softAPIP().toString());
  }
}

void handleAngles() {
  String json = "{";
  json += "\"R1_SHOUL_Servo\":" + String(currentAngles[R1]) + ",";
  json += "\"R2_SHOUL_Servo\":" + String(currentAngles[R2]) + ",";
  json += "\"L1_SHOUL_Servo\":" + String(currentAngles[L1]) + ",";
  json += "\"L2_SHOUL_Servo\":" + String(currentAngles[L2]) + ",";
  json += "\"R3_EL_Servo\":" + String(currentAngles[R3]) + ",";
  json += "\"L4_EL_Servo\":" + String(currentAngles[L4]) + ",";
  json += "\"L3_EL_Servo\":" + String(currentAngles[L3]) + ",";
  json += "\"R4_EL_Servo\":" + String(currentAngles[R4]);
  json += "}";
  server.send(200, "application/json", json);
}

// =====================================================
// HTTP HANDLERS FOR FIRMWARE COMMANDS & API
// =====================================================

void handleCommandWeb() {
  if (server.hasArg("pose")) {
    currentCommand = server.arg("pose");
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "OK"); 
  } 
  else if (server.hasArg("go")) {
    currentCommand = server.arg("go");
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "OK");
  } 
  else if (server.hasArg("stop")) {
    currentCommand = "";
    recordInput();
    runStandPose(1);
    server.send(200, "text/plain", "OK");
  }
  else if (server.hasArg("face")) {
    setFace(server.arg("face"));
    recordInput();
    server.send(200, "text/plain", "OK");
  }
  else if (server.hasArg("motor") && server.hasArg("value")) {
    int motorNum = server.arg("motor").toInt();
    int servoIdx = servoNameToIndex(server.arg("motor"));
    int angle = server.arg("value").toInt();
    if (motorNum >= 1 && motorNum <= 8) {
      setServoAngle(motorNum - 1, angle);
      recordInput();
      server.send(200, "text/plain", "OK");
    } else if (servoIdx != -1) {
      setServoAngle(servoIdx, angle);
      recordInput();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid motor");
    }
  }
  else {
    server.send(400, "text/plain", "Bad Args");
  }
}

void handleGetSettings() {
  String json = "{";
  json += "\"frameDelay\":" + String(frameDelay) + ",";
  json += "\"walkCycles\":" + String(walkCycles) + ",";
  json += "\"motorCurrentDelay\":" + String(motorCurrentDelay) + ",";
  json += "\"faceFps\":" + String(faceFps);
  json += "}";
  server.send(200, "application/json", json);
}

void handleSetSettings() {
  if (server.hasArg("frameDelay")) frameDelay = server.arg("frameDelay").toInt();
  if (server.hasArg("walkCycles")) walkCycles = server.arg("walkCycles").toInt();
  if (server.hasArg("motorCurrentDelay")) motorCurrentDelay = server.arg("motorCurrentDelay").toInt();
  if (server.hasArg("faceFps")) faceFps = (int)max(1L, server.arg("faceFps").toInt());
  server.send(200, "text/plain", "OK");
}

void handleGetStatus() {
  String json = "{";
  json += "\"currentCommand\":\"" + currentCommand + "\",";
  json += "\"currentFace\":\"" + currentFaceName + "\",";
  json += "\"networkConnected\":" + String(networkConnected ? "true" : "false") + ",";
  json += "\"apIP\":\"" + WiFi.softAPIP().toString() + "\"";
  if (networkConnected) {
    json += ",\"networkIP\":\"" + networkIP.toString() + "\"";
  }
  json += "}";
  server.send(200, "application/json", json);
}

void handleApiCommand() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  String body = server.arg("plain");
  Serial.println("API Command: " + body);
  
  int faceStart = body.indexOf("\"face\":\"");
  if (faceStart == -1) faceStart = body.indexOf("\"face\": \"");
  if (faceStart > 0) {
    faceStart = body.indexOf("\"", faceStart + 6) + 1;
    int faceEnd = body.indexOf("\"", faceStart);
    if (faceEnd > faceStart) {
      setFace(body.substring(faceStart, faceEnd));
    }
  }

  int cmdStart = body.indexOf("\"command\":\"");
  if (cmdStart == -1) cmdStart = body.indexOf("\"command\": \"");
  if (cmdStart > 0) {
    cmdStart = body.indexOf("\"", cmdStart + 10) + 1;
    int cmdEnd = body.indexOf("\"", cmdStart);
    if (cmdEnd > cmdStart) {
      String cmd = body.substring(cmdStart, cmdEnd);
      if (cmd == "stop") {
        currentCommand = "";
        runStandPose(1);
      } else {
        currentCommand = cmd;
        exitIdle();
      }
      recordInput();
      server.send(200, "application/json", "{\"status\":\"ok\"}");
      return;
    }
  }
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

String jsonEscape(const String& s) {
  String out;
  out.reserve(s.length() + 8);
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\\') out += "\\\\";
    else if (c == '"') out += "\\\"";
    else if ((uint8_t)c < 0x20) {
      char buf[8];
      snprintf(buf, sizeof(buf), "\\u%04x", (uint8_t)c);
      out += buf;
    } else out += c;
  }
  return out;
}

bool startMdns() {
  mdnsOk = MDNS.begin(deviceHostname.c_str());
  if (mdnsOk) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started: http://" + deviceHostname + ".local");
  }
  return mdnsOk;
}

void announceNetwork(const String& ssid) {
  MDNS.end();
  startMdns();
  wifiInfoText = "AP: " + String(AP_SSID) + " (" + WiFi.softAPIP().toString() +
                 ")  |  Network: " + ssid + " (" + networkIP.toString() + ") or " +
                 deviceHostname + ".local  |  ";
}

void setApOnlyInfoText() {
  wifiInfoText = "Connect to WiFi: " + String(AP_SSID) + "  |  Pass: " + String(AP_PASS) +
                 "  |  IP: " + WiFi.softAPIP().toString() + "  |  Captive Portal will auto-open!  |  ";
}

void showWifiInfoNow() {
  firstInputReceived = false;
  lastInputTime = millis() - 30000;
  showingWifiInfo = false;
}

bool connectToWifi(const String& ssid, const String& pass, uint32_t timeoutMs) {
  if (ssid.length() == 0) return false;
  Serial.println("Connecting to WiFi network: " + ssid);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setHostname(deviceHostname.c_str());
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long start = millis();
  while ((millis() - start) < timeoutMs) {
    wl_status_t st = WiFi.status();
    if (st == WL_CONNECTED) break;
    if ((millis() - start) > 1000 && (st == WL_CONNECT_FAILED || st == WL_NO_SSID_AVAIL)) break;
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connect failed.");
    WiFi.disconnect();
    return false;
  }

  networkConnected = true;
  networkIP = WiFi.localIP();
  Serial.print("Connected! IP: ");
  Serial.println(networkIP);
  return true;
}

void finishWifiSetup(const String& err) {
  wifiSetupError = err;
  wifiSetupPass = "";
  wifiSetupState = WIFI_SETUP_IDLE;
}

void updateWifiSetup() {
  if (wifiSetupState == WIFI_SETUP_IDLE) {
    static unsigned long lastCheckMs = 0;
    if (millis() - lastCheckMs >= 5000) {
      lastCheckMs = millis();
      bool live = (WiFi.status() == WL_CONNECTED);
      if (live != networkConnected) {
        networkConnected = live;
        if (live) {
          networkIP = WiFi.localIP();
          announceNetwork(WiFi.SSID());
        }
      }
    }
    return;
  }

  if (wifiSetupState == WIFI_SETUP_QUEUED) {
    if (millis() - wifiSetupQueuedMs < WIFI_SETUP_START_DELAY_MS) return;
    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname(deviceHostname.c_str());
    WiFi.begin(wifiSetupSsid.c_str(), wifiSetupPass.c_str());
    wifiSetupStartMs = millis();
    wifiSetupState = WIFI_SETUP_CONNECTING;
    return;
  }

  wl_status_t st = WiFi.status();
  if (st == WL_CONNECTED) {
    networkConnected = true;
    networkIP = WiFi.localIP();
    announceNetwork(wifiSetupSsid);
    showWifiInfoNow();
    finishWifiSetup("");
    return;
  }
  
  bool terminal = (millis() - wifiSetupStartMs > 1000) &&
                  (st == WL_CONNECT_FAILED || st == WL_NO_SSID_AVAIL);
  if (terminal || (millis() - wifiSetupStartMs) >= WIFI_CONNECT_TIMEOUT_MS) {
    String err = (st == WL_NO_SSID_AVAIL)  ? "Network not found"
               : (st == WL_CONNECT_FAILED) ? "Wrong password"
                                           : "Timed out";
    WiFi.disconnect();
    networkConnected = false;
    setApOnlyInfoText();
    finishWifiSetup(err);
  }
}

void handleWifiScan() {
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_RUNNING) {
    server.send(200, "application/json", "{\"scanning\":true}");
    return;
  }
  if (n < 0) {
    if (wifiSetupState != WIFI_SETUP_IDLE) {
      server.send(200, "application/json", "{\"scanning\":true}");
      return;
    }
    if (WiFi.getMode() == WIFI_AP) {
      WiFi.mode(WIFI_AP_STA);
      wifiRestoreApOnly = true;
    }
    WiFi.scanNetworks(true);
    server.send(200, "application/json", "{\"scanning\":true}");
    return;
  }

  String json = "[";
  json.reserve(n * 64 + 2);
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"secure\":" + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true") + "}";
  }
  json += "]";
  WiFi.scanDelete();
  if (wifiRestoreApOnly && wifiSetupState == WIFI_SETUP_IDLE && WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
  }
  wifiRestoreApOnly = false;
  server.send(200, "application/json", json);
}

void handleWifiConnect() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"success\":false,\"error\":\"Method not allowed\"}");
    return;
  }
  String ssid = server.arg("ssid");
  if (ssid.length() == 0) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"SSID required\"}");
    return;
  }
  if (wifiSetupState != WIFI_SETUP_IDLE) {
    server.send(409, "application/json", "{\"success\":false,\"error\":\"In progress\"}");
    return;
  }
  wifiSetupSsid = ssid;
  wifiSetupPass = server.arg("password");
  wifiSetupError = "";
  wifiSetupQueuedMs = millis();
  wifiSetupState = WIFI_SETUP_QUEUED;
  wifiRestoreApOnly = false;
  server.send(200, "application/json", "{\"success\":true,\"pending\":true}");
}

void handleWifiStatus() {
  bool connected = (WiFi.status() == WL_CONNECTED);
  String json = "{\"connected\":" + String(connected ? "true" : "false");
  json += ",\"connecting\":" + String(wifiSetupState != WIFI_SETUP_IDLE ? "true" : "false");
  if (wifiSetupError.length() > 0) {
    json += ",\"lastError\":\"" + jsonEscape(wifiSetupError) + "\"";
  }
  if (connected) {
    json += ",\"ssid\":\"" + jsonEscape(WiFi.SSID()) + "\"";
    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += ",\"host\":\"" + deviceHostname + ".local\"";
    json += ",\"mdns\":" + String(mdnsOk ? "true" : "false");
    json += ",\"rssi\":" + String(WiFi.RSSI());
  }
  json += "}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  if (server.uri().startsWith("/api/")) {
    server.send(404, "application/json", "{\"error\":\"Not found\"}");
    return;
  }
  handleRoot();
}

void setup() {
  // Disable brownout detector to prevent ESP32 rebooting under high current
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  randomSeed(micros());
  
  // I2C Init for ESP32
  Wire.begin(I2C_SDA, I2C_SCL);

  // OLED Init (non-fatal if no display is connected)
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    hasDisplay = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Setting up WiFi..."));
    display.display();
    Serial.println(F("SSD1306 OLED initialized."));
  } else {
    hasDisplay = false;
    Serial.println(F("SSD1306 OLED not found. Running headless."));
  }

  // --- WIFI CONFIGURATION ---
  WiFi.persistent(false);

  if (ENABLE_NETWORK_MODE && String(NETWORK_SSID).length() > 0) {
    if (!connectToWifi(NETWORK_SSID, NETWORK_PASS)) {
      Serial.println("Failed to connect to network. AP-only mode.");
      WiFi.mode(WIFI_AP);
    }
  } else {
    WiFi.mode(WIFI_AP);
  }
  
  // SoftAP
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP Created. IP: ");
  Serial.println(myIP);

  if (networkConnected) {
    announceNetwork(NETWORK_SSID);
  } else {
    setApOnlyInfoText();
    startMdns();
  }

  // ArduinoOTA Setup
  ArduinoOTA.setHostname(deviceHostname.c_str());
  ArduinoOTA.begin();

  lastInputTime = millis();
  firstInputReceived = false;
  showingWifiInfo = false;

  // DNS Server for Captive Portal
  dnsServer.start(DNS_PORT, "*", myIP);

  // Original Calibrator UI routes
  server.on("/", handleRoot);
  server.on("/servo", handleServo);
  server.on("/preset", handlePreset);
  server.on("/walk", handleWalk);
  server.on("/walkstatus", handleWalkStatus);
  server.on("/walkStatus", handleWalkStatus);
  server.on("/hi", handleHi);
  server.on("/360", handle360);
  server.on("/ip", handleIP);
  server.on("/angles", handleAngles);

  // Browser OTA Update routes
  server.on("/update", HTTP_GET, []() {
    server.send_P(200, "text/html", OTA_page);
  });
  server.on(
    "/update",
    HTTP_POST,
    []() {
      server.sendHeader("Connection", "close");
      if (Update.hasError()) {
        server.send(500, "text/plain", "Firmware update failed.");
      } else {
        server.send(200, "text/plain", "Firmware update successful. Rebooting...");
        delay(500);
        ESP.restart();
      }
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Browser OTA started: %s\n", upload.filename.c_str());
        currentCommand = "";
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Browser OTA successful: %u bytes\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    }
  );
  
  // Firmware command & API routes
  server.on("/cmd", handleCommandWeb);
  server.on("/getSettings", handleGetSettings);
  server.on("/setSettings", handleSetSettings);
  server.on("/api/status", handleGetStatus);
  server.on("/api/command", handleApiCommand);
  server.on("/api/wifi/scan", handleWifiScan);
  server.on("/api/wifi/connect", handleWifiConnect);
  server.on("/api/wifi/status", handleWifiStatus);
  
  // Captive portal catch-all
  server.onNotFound(handleNotFound);
  
  server.begin();

  // PWM Init
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  for (int i = 0; i < 8; i++) {
    servos[i].setPeriodHertz(50);
    // Standard hobby servo pulse range (544-2400us). Prevents motor collision!
    servos[i].attach(servoPins[i]);
  }
  delay(10);
  
  // Power-on calibration stance: safe Rest Pose
  runRestPose();
  setFace("rest");
  
  Serial.println(F("Sesame Robot Controller & Calibrator online."));
}

void loop() {
  ArduinoOTA.handle();

  // Captive portal DNS requests
  dnsServer.processNextRequest();
  
  server.handleClient();
  updateWifiSetup();
  updateAnimatedFace();
  updateIdleBlink();
  updateWifiInfoScroll();

  if (currentCommand != "") {
    String cmd = currentCommand;
    if (cmd == "forward") runWalkPose();
    else if (cmd == "backward") runWalkBackward();
    else if (cmd == "left") runTurnLeft();
    else if (cmd == "right") runTurnRight();
    else if (cmd == "rest") { runRestPose(); if (currentCommand == "rest") currentCommand = ""; }
    else if (cmd == "stand") { runStandPose(1); if (currentCommand == "stand") currentCommand = ""; }
    else if (cmd == "wave") runWavePose();
    else if (cmd == "dance") runDancePose();
    else if (cmd == "swim") runSwimPose();
    else if (cmd == "point") runPointPose();
    else if (cmd == "pushup") runPushupPose();
    else if (cmd == "bow") runBowPose();
    else if (cmd == "cute") runCutePose();
    else if (cmd == "freaky") runFreakyPose();
    else if (cmd == "worm") runWormPose();
    else if (cmd == "shake") runShakePose();
    else if (cmd == "shrug") runShrugPose();
    else if (cmd == "dead") runDeadPose();
    else if (cmd == "crab") runCrabPose();
    else if (cmd == "spin360" || cmd == "360") run360Pose();
  }
  
  // Serial CLI for diagnostics & manual angle control
  if (Serial.available()) {
    static char command_buffer[32];
    static byte buffer_pos = 0;
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (buffer_pos > 0) {
        command_buffer[buffer_pos] = '\0';
        int motorNum, angle;
        recordInput();
        if(strcmp(command_buffer, "run walk") == 0 || strcmp(command_buffer, "rn wf") == 0) { currentCommand = "forward"; runWalkPose(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn wb") == 0) { currentCommand = "backward"; runWalkBackward(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn tl") == 0) { currentCommand = "left"; runTurnLeft(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn tr") == 0) { currentCommand = "right"; runTurnRight(); currentCommand = ""; }
        else if(strcmp(command_buffer, "run rest") == 0 || strcmp(command_buffer, "rn rs") == 0) runRestPose();
        else if(strcmp(command_buffer, "run stand") == 0 || strcmp(command_buffer, "rn st") == 0) runStandPose(1);
        else if(strcmp(command_buffer, "rn wv") == 0) { currentCommand = "wave"; runWavePose(); }
        else if(strcmp(command_buffer, "rn dn") == 0) { currentCommand = "dance"; runDancePose(); }
        else if(strcmp(command_buffer, "rn sw") == 0) { currentCommand = "swim"; runSwimPose(); }
        else if(strcmp(command_buffer, "rn pt") == 0) { currentCommand = "point"; runPointPose(); }
        else if(strcmp(command_buffer, "rn pu") == 0) { currentCommand = "pushup"; runPushupPose(); }
        else if(strcmp(command_buffer, "rn bw") == 0) { currentCommand = "bow"; runBowPose(); }
        else if(strcmp(command_buffer, "rn ct") == 0) { currentCommand = "cute"; runCutePose(); }
        else if(strcmp(command_buffer, "rn fk") == 0) { currentCommand = "freaky"; runFreakyPose(); }
        else if(strcmp(command_buffer, "rn wm") == 0) { currentCommand = "worm"; runWormPose(); }
        else if(strcmp(command_buffer, "rn sk") == 0) { currentCommand = "shake"; runShakePose(); }
        else if(strcmp(command_buffer, "rn sg") == 0) { currentCommand = "shrug"; runShrugPose(); }
        else if(strcmp(command_buffer, "rn dd") == 0) { currentCommand = "dead"; runDeadPose(); }
        else if(strcmp(command_buffer, "rn cb") == 0) { currentCommand = "crab"; runCrabPose(); }
        else if(strcmp(command_buffer, "rn 360") == 0 || strcmp(command_buffer, "run 360") == 0) { currentCommand = "spin360"; run360Pose(); }
        else if(strncmp(command_buffer, "rn 360 ", 7) == 0 || strncmp(command_buffer, "run 360 ", 8) == 0) {
          const char* p = (command_buffer[1] == 'n') ? command_buffer + 7 : command_buffer + 8;
          int c = atoi(p);
          if (c > 0) spin360Cycles = c;
          currentCommand = "spin360";
          run360Pose();
        }
        else if (strncmp(command_buffer, "face ", 5) == 0 || strncmp(command_buffer, "fc ", 3) == 0) {
          const char* faceName = (command_buffer[1] == 'c') ? command_buffer + 3 : command_buffer + 5;
          if (strlen(faceName) > 0) {
            currentCommand = "";
            setFace(String(faceName));
            Serial.print("Face set to ");
            Serial.println(faceName);
          }
        }
        else if (strcmp(command_buffer, "subtrim") == 0 || strcmp(command_buffer, "st") == 0) {
          Serial.println("Subtrim values:");
          for (int i = 0; i < 8; i++) {
            Serial.print("Motor "); Serial.print(i); Serial.print(": ");
            if (servoSubtrim[i] >= 0) Serial.print("+");
            Serial.println(servoSubtrim[i]);
          }
        }
        else if (strncmp(command_buffer, "all ", 4) == 0) {
          if (sscanf(command_buffer + 4, "%d", &angle) == 1) {
            for (int i = 0; i < 8; i++) setServoAngle(i, angle);
            Serial.print("All servos set to "); Serial.println(angle);
          }
        }
        else if (sscanf(command_buffer, "%d %d", &motorNum, &angle) == 2) {
          if (motorNum >= 0 && motorNum < 8) {
            setServoAngle(motorNum, angle);
            Serial.print("Servo "); Serial.print(motorNum); Serial.print(" set to "); Serial.println(angle);
          }
        }
        buffer_pos = 0;
      }
    } else if (buffer_pos < sizeof(command_buffer) - 1) {
      command_buffer[buffer_pos++] = c;
    }
  }
}

// Function to update the robot's face
void updateFaceBitmap(const unsigned char* bitmap) {
  if (!hasDisplay) return;
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap, 128, 64, SSD1306_WHITE);
  display.display();
}

uint8_t countFrames(const unsigned char* const* frames, uint8_t maxFrames) {
  if (frames == nullptr || frames[0] == nullptr) return 0;
  uint8_t count = 0;
  for (uint8_t i = 0; i < maxFrames; i++) {
    if (frames[i] == nullptr) break;
    count++;
  }
  return count;
}

void setFace(const String& faceName) {
  if (faceName == currentFaceName && currentFaceFrames != nullptr) return;

  currentFaceName = faceName;
  currentFaceFrameIndex = 0;
  lastFaceFrameMs = 0;
  faceFrameDirection = 1;
  faceAnimFinished = false;
  currentFaceFps = getFaceFpsForName(faceName);

  currentFaceFrames = face_defualt_frames;
  currentFaceFrameCount = countFrames(face_defualt_frames, MAX_FACE_FRAMES);

  for (size_t i = 0; i < (sizeof(faceEntries) / sizeof(faceEntries[0])); i++) {
    if (faceName.equalsIgnoreCase(faceEntries[i].name)) {
      currentFaceFrames = faceEntries[i].frames;
      currentFaceFrameCount = countFrames(faceEntries[i].frames, faceEntries[i].maxFrames);
      break;
    }
  }

  if (currentFaceFrameCount == 0) {
    currentFaceFrames = face_defualt_frames;
    currentFaceFrameCount = countFrames(face_defualt_frames, MAX_FACE_FRAMES);
    currentFaceName = "default";
    currentFaceFps = getFaceFpsForName(currentFaceName);
  }

  if (currentFaceFrameCount > 0 && currentFaceFrames[0] != nullptr) {
    updateFaceBitmap(currentFaceFrames[0]);
  }
}

void setFaceMode(FaceAnimMode mode) {
  currentFaceMode = mode;
  faceFrameDirection = 1;
  faceAnimFinished = false;
}

void setFaceWithMode(const String& faceName, FaceAnimMode mode) {
  setFaceMode(mode);
  setFace(faceName);
}

int getFaceFpsForName(const String& faceName) {
  for (size_t i = 0; i < (sizeof(faceFpsEntries) / sizeof(faceFpsEntries[0])); i++) {
    if (faceName.equalsIgnoreCase(faceFpsEntries[i].name)) {
      return faceFpsEntries[i].fps;
    }
  }
  return faceFps;
}

void updateAnimatedFace() {
  if (currentFaceFrames == nullptr || currentFaceFrameCount <= 1) return;
  if (currentFaceMode == FACE_ANIM_ONCE && faceAnimFinished) return;

  unsigned long now = millis();
  int fps = max(1, (currentFaceFps > 0 ? currentFaceFps : faceFps));
  unsigned long interval = 1000UL / fps;
  if (now - lastFaceFrameMs >= interval) {
    lastFaceFrameMs = now;
    if (currentFaceMode == FACE_ANIM_LOOP) {
      currentFaceFrameIndex = (currentFaceFrameIndex + 1) % currentFaceFrameCount;
    } else if (currentFaceMode == FACE_ANIM_ONCE) {
      if (currentFaceFrameIndex + 1 >= currentFaceFrameCount) {
        currentFaceFrameIndex = currentFaceFrameCount - 1;
        faceAnimFinished = true;
      } else {
        currentFaceFrameIndex++;
      }
    } else {
      if (faceFrameDirection > 0) {
        if (currentFaceFrameIndex + 1 >= currentFaceFrameCount) {
          faceFrameDirection = -1;
          if (currentFaceFrameIndex > 0) currentFaceFrameIndex--;
        } else {
          currentFaceFrameIndex++;
        }
      } else {
        if (currentFaceFrameIndex == 0) {
          faceFrameDirection = 1;
          if (currentFaceFrameCount > 1) currentFaceFrameIndex++;
        } else {
          currentFaceFrameIndex--;
        }
      }
    }
    updateFaceBitmap(currentFaceFrames[currentFaceFrameIndex]);
  }
}

void delayWithFace(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    updateAnimatedFace();
    server.handleClient();
    dnsServer.processNextRequest();
    ArduinoOTA.handle();
    delay(5);
  }
}

void scheduleNextIdleBlink(unsigned long minMs, unsigned long maxMs) {
  unsigned long now = millis();
  unsigned long interval = (unsigned long)random(minMs, maxMs);
  nextIdleBlinkMs = now + interval;
}

void enterIdle() {
  idleActive = true;
  idleBlinkActive = false;
  idleBlinkRepeatsLeft = 0;
  setFaceWithMode("idle", FACE_ANIM_BOOMERANG);
  scheduleNextIdleBlink(3000, 7000);
}

void exitIdle() {
  idleActive = false;
  idleBlinkActive = false;
}

void updateIdleBlink() {
  if (!idleActive) return;

  if (!idleBlinkActive) {
    if (millis() >= nextIdleBlinkMs) {
      idleBlinkActive = true;
      if (idleBlinkRepeatsLeft == 0 && random(0, 100) < 30) {
        idleBlinkRepeatsLeft = 1; // double blink
      }
      setFaceWithMode("idle_blink", FACE_ANIM_ONCE);
    }
    return;
  }

  if (currentFaceMode == FACE_ANIM_ONCE && faceAnimFinished) {
    idleBlinkActive = false;
    setFaceWithMode("idle", FACE_ANIM_BOOMERANG);
    if (idleBlinkRepeatsLeft > 0) {
      idleBlinkRepeatsLeft--;
      scheduleNextIdleBlink(120, 220);
    } else {
      scheduleNextIdleBlink(3000, 7000);
    }
  }
}

// ====== HELPERS ======
void setServoAngle(uint8_t channel, int angle) { 
  if (channel < 8) {
    int adjustedAngle = angle + servoSubtrim[channel];
    // Strict safety limits: Shoulders (0..3) strictly 0 to 90 degrees
    if (channel < 4) {
      adjustedAngle = constrain(adjustedAngle, 0, 90);
    } else {
      adjustedAngle = constrain(adjustedAngle, 0, 180);
    }
    servos[channel].write(adjustedAngle);
    currentAngles[channel] = adjustedAngle;
    delayWithFace(motorCurrentDelay);
  }
}

bool pressingCheck(String cmd, int ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    server.handleClient();
    dnsServer.processNextRequest();
    ArduinoOTA.handle();
    updateAnimatedFace();
    if (currentCommand != cmd) {
      runStandPose(1);
      return false;
    }
    yield();
  }
  return true;
}

void recordInput() {
  lastInputTime = millis();
  if (!firstInputReceived) {
    firstInputReceived = true;
    showingWifiInfo = false;
  }
}

void updateWifiInfoScroll() {
  if (!hasDisplay) return;
  if (firstInputReceived) {
    if (showingWifiInfo) {
      showingWifiInfo = false;
      if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
        updateFaceBitmap(currentFaceFrames[currentFaceFrameIndex]);
      }
    }
    return;
  }
  
  unsigned long now = millis();
  if (!showingWifiInfo && (now - lastInputTime >= 30000)) {
    showingWifiInfo = true;
    wifiScrollPos = 0;
    lastWifiScrollMs = now;
  }
  
  if (!showingWifiInfo) return;
  
  if (now - lastWifiScrollMs >= 150) {
    lastWifiScrollMs = now;
    display.clearDisplay();
    if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
      display.drawBitmap(0, 0, currentFaceFrames[currentFaceFrameIndex], 128, 64, SSD1306_WHITE);
    }
    display.fillRect(0, 0, 128, 10, SSD1306_BLACK);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    display.setCursor(-wifiScrollPos, 1);
    display.print(wifiInfoText);
    display.setTextWrap(true);
    display.display();
    wifiScrollPos += 2;
    if (wifiScrollPos >= (int)(wifiInfoText.length() * 6)) {
      wifiScrollPos = 0;
    }
  }
}
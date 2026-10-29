#pragma once

#include <Arduino.h>

// ======================================================================
// SERVO ENUM & MAPPING (Matched to sesame_reobo.ino hardware)
// ======================================================================
enum ServoName : uint8_t {
  R1 = 0, // R1 Shoulder (GPIO 12) [0 - 90 deg]
  R2 = 1, // R2 Shoulder (GPIO 15) [0 - 90 deg]
  L1 = 2, // L1 Shoulder (GPIO 13) [0 - 90 deg]
  L2 = 3, // L2 Shoulder (GPIO 2)  [0 - 90 deg]
  R4 = 4, // R4 Elbow    (GPIO 27) [0 - 180 deg]
  R3 = 5, // R3 Elbow    (GPIO 4)  [0 - 180 deg]
  L3 = 6, // L3 Elbow    (GPIO 5)  [0 - 180 deg]
  L4 = 7  // L4 Elbow    (GPIO 14) [0 - 180 deg]
};

const String ServoNames[] = {"R1", "R2", "L1", "L2", "R4", "R3", "L3", "L4"};

inline int servoNameToIndex(const String& servo) {
  if (servo == "L1") return L1;
  if (servo == "L2") return L2;
  if (servo == "L3") return L3;
  if (servo == "L4") return L4;
  if (servo == "R1") return R1;
  if (servo == "R2") return R2;
  if (servo == "R3") return R3;
  if (servo == "R4") return R4;
  return -1;
}

enum FaceAnimMode : uint8_t {
  FACE_ANIM_LOOP = 0,
  FACE_ANIM_ONCE = 1,
  FACE_ANIM_BOOMERANG = 2
};

// External globals and helpers used by movement/pose sequences
extern int frameDelay;
extern int walkCycles;
extern String currentCommand;
extern int spin360Shoulder;
extern int spin360Cycles;

extern void setServoAngle(uint8_t channel, int angle);
extern void setFace(const String& faceName);
extern void setFaceMode(FaceAnimMode mode);
extern void setFaceWithMode(const String& faceName, FaceAnimMode mode);
extern void delayWithFace(unsigned long ms);
extern void enterIdle();
extern bool pressingCheck(String cmd, int ms);

// Pose/animation prototypes
void runRestPose();
void runStandPose(int face = 1);
void runWavePose();
void runDancePose();
void runSwimPose();
void runPointPose();
void runPushupPose();
void runBowPose();
void runCutePose();
void runFreakyPose();
void runWormPose();
void runShakePose();
void runShrugPose();
void runDeadPose();
void runCrabPose();
void runWalkPose();
void runWalkBackward();
void runTurnLeft();
void runTurnRight();
void run360Pose(int shoulder = -999, int cycles = -1);

// ======================================================================
// CALIBRATED POSES (Using exact sesame_reobo.ino mechanical angles)
//
// Shoulders strictly 0 to 90 degrees:
//   Default:  R1=90, R2=0,   L1=0,   L2=90
//   Walking:  R1=45, R2=45,  L1=45,  L2=45
//
// Elbows 0 to 180 degrees (Right = 180° - Left):
//   Default:  R3=180, L4=0,   L3=0,   R4=180
//   Vertical: R3=160, L4=20,  L3=20,  R4=160
//   Diag 45°: R3=135, L4=45,  L3=45,  R4=135
//   Horiz:    R3=90,  L4=90,  L3=90,  R4=90
//   Walk Trot:
//     Phase 0: L4=15, L3=65,  R4=120, R3=175
//     Phase 1: L4=60, L3=5,   R4=165, R3=115
// ======================================================================

// ====== REST POSE (Default Flat / Parked) ======
inline void runRestPose() { 
  Serial.println(F("REST")); 
  setFaceWithMode("rest", FACE_ANIM_BOOMERANG); 
  // Shoulders to default
  setServoAngle(R1, 90); 
  setServoAngle(R2, 0); 
  setServoAngle(L1, 0); 
  setServoAngle(L2, 90); 
  // Elbows to default
  setServoAngle(R4, 180); 
  setServoAngle(R3, 180); 
  setServoAngle(L3, 0); 
  setServoAngle(L4, 0); 
}

// ====== STAND POSE (Calibrated Vertical Stance) ======
inline void runStandPose(int face) { 
  Serial.println(F("STAND")); 
  if (face == 1) setFaceWithMode("stand", FACE_ANIM_ONCE); 
  // Shoulders in calibrated walking stance (All 45 deg)
  setServoAngle(R1, 45); 
  setServoAngle(R2, 45); 
  setServoAngle(L1, 45); 
  setServoAngle(L2, 45); 
  // Elbows in calibrated vertical stance: R3=160, L4=20, L3=20, R4=160
  setServoAngle(R4, 160); 
  setServoAngle(R3, 160); 
  setServoAngle(L3, 20); 
  setServoAngle(L4, 20); 
  if (face == 1) enterIdle();
}

// ====== WAVE POSE (User's Calibrated "Say Hi" Gesture) ======
inline void runWavePose() { 
  Serial.println(F("WAVE / SAY HI")); 
  setFaceWithMode("wave", FACE_ANIM_ONCE); 
  runStandPose(0); 
  delayWithFace(200);

  // Tilt onto L3 for balance
  setServoAngle(L3, 105); 
  delayWithFace(260);

  // Raise L1 shoulder
  setServoAngle(L1, 75); 
  delayWithFace(260);

  // Wave L4 paw 3 times between 170 and 135 deg
  for (int i = 0; i < 3; i++) { 
    if (currentCommand != "wave") break;
    setServoAngle(L4, 170); delayWithFace(260); 
    setServoAngle(L4, 135); delayWithFace(260); 
  } 

  runStandPose(1); 
  if (currentCommand == "wave") currentCommand = "";
}

// ====== 360 SPIN GESTURE (Vertical Stance, Shoulders L2/R1=70 R2/L1=20, Selectable Shoulder or All 4) ======
inline void run360Pose(int shoulder, int cycles) {
  if (shoulder == -999) shoulder = spin360Shoulder;
  if (cycles <= 0) cycles = spin360Cycles;
  Serial.printf("360 SPIN (shoulder mode: %d, cycles: %d)\n", shoulder, cycles);
  setFaceWithMode("dance", FACE_ANIM_LOOP);

  // Set shoulders for 360 (L2=70, R1=70, R2=20, L1=20)
  setServoAngle(R1, 70);
  setServoAngle(R2, 20);
  setServoAngle(L1, 20);
  setServoAngle(L2, 70);

  // Set elbows / edges to vertical stance (R3=160, L4=20, L3=20, R4=180)
  setServoAngle(R3, 160);
  setServoAngle(L4, 20);
  setServoAngle(L3, 20);
  setServoAngle(R4, 180);
  delayWithFace(200);

  for (int cycle = 0; cycle < cycles; cycle++) {
    if (currentCommand != "spin360" && currentCommand != "360" &&
        !currentCommand.startsWith("360_") && !currentCommand.startsWith("spin360_")) break;

    // Stroke Phase A: Sweep towards stroke angle
    if (shoulder == -1) {
      setServoAngle(L2, 0);
      setServoAngle(R1, 0);
      setServoAngle(L1, 90);
      setServoAngle(R2, 90);
    } else if (shoulder == L1 || shoulder == R2) {
      setServoAngle(shoulder, 90);
    } else {
      setServoAngle(shoulder, 0);
    }

    if (!pressingCheck("spin360", 180) && !pressingCheck("360", 0)) {
      if (currentCommand != "spin360" && currentCommand != "360" &&
          !currentCommand.startsWith("360_") && !currentCommand.startsWith("spin360_")) break;
    }

    // Stroke Phase B: Return towards 360 stance
    if (shoulder == -1) {
      setServoAngle(L2, 70);
      setServoAngle(R1, 70);
      setServoAngle(L1, 20);
      setServoAngle(R2, 20);
    } else if (shoulder == L1 || shoulder == R2) {
      setServoAngle(shoulder, 20);
    } else {
      setServoAngle(shoulder, 70);
    }

    if (!pressingCheck("spin360", 180) && !pressingCheck("360", 0)) {
      if (currentCommand != "spin360" && currentCommand != "360" &&
          !currentCommand.startsWith("360_") && !currentCommand.startsWith("spin360_")) break;
    }
  }

  // Restore all shoulders to 360 stance
  setServoAngle(R1, 70);
  setServoAngle(R2, 20);
  setServoAngle(L1, 20);
  setServoAngle(L2, 70);

  if (currentCommand == "spin360" || currentCommand == "360" ||
      currentCommand.startsWith("360_") || currentCommand.startsWith("spin360_")) {
    currentCommand = "";
    runStandPose(1);
  }
}

// ====== DANCE POSE ======
inline void runDancePose() { 
  Serial.println(F("DANCE")); 
  setFaceWithMode("dance", FACE_ANIM_LOOP); 
  runStandPose(0);
  delayWithFace(200); 
  for (int i = 0; i < 5; i++) { 
    if (currentCommand != "dance") break;
    // Step A: Left lifts L4 (40) and plants L3 (10). Right mirrors: lifts R4 (140) and plants R3 (170)
    setServoAngle(R4, 140); setServoAngle(R3, 170); 
    setServoAngle(L3, 10);  setServoAngle(L4, 40); 
    delayWithFace(250); 
    // Step B: Left plants L4 (10) and lifts L3 (40). Right mirrors: plants R4 (170) and lifts R3 (140)
    setServoAngle(R4, 170); setServoAngle(R3, 140); 
    setServoAngle(L3, 40);  setServoAngle(L4, 10); 
    delayWithFace(250); 
  } 
  runStandPose(1); 
  if (currentCommand == "dance") currentCommand = "";
}

// ====== SWIM POSE ======
inline void runSwimPose() { 
  Serial.println(F("SWIM")); 
  setFaceWithMode("swim", FACE_ANIM_ONCE); 
  runRestPose();
  delayWithFace(300);
  for (int i = 0; i < 4; i++) { 
    if (currentCommand != "swim") break;
    setServoAngle(R1, 60); setServoAngle(R2, 30); 
    setServoAngle(L1, 30); setServoAngle(L2, 60); 
    delayWithFace(350); 
    setServoAngle(R1, 90); setServoAngle(R2, 0); 
    setServoAngle(L1, 0);  setServoAngle(L2, 90); 
    delayWithFace(350); 
  } 
  runStandPose(1); 
  if (currentCommand == "swim") currentCommand = "";
}

// ====== POINT POSE ======
inline void runPointPose() { 
  Serial.println(F("POINT")); 
  setFaceWithMode("point", FACE_ANIM_BOOMERANG); 
  runStandPose(0);
  delayWithFace(200);
  setServoAngle(L1, 70); 
  setServoAngle(L4, 150); 
  delayWithFace(1500); 
  runStandPose(1); 
  if (currentCommand == "point") currentCommand = "";
}

// ====== PUSHUP POSE ======
inline void runPushupPose() {
  Serial.println(F("PUSHUP"));
  setFaceWithMode("pushup", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  for (int i = 0; i < 4; i++) {
    if (currentCommand != "pushup") break;
    // Lower front
    setServoAngle(R4, 110); setServoAngle(L4, 70);
    delayWithFace(400);
    // Raise front (back to vertical)
    setServoAngle(R4, 160); setServoAngle(L4, 20);
    delayWithFace(400);
  }
  runStandPose(1);
  if (currentCommand == "pushup") currentCommand = "";
}

// ====== BOW POSE ======
inline void runBowPose() {
  Serial.println(F("BOW"));
  setFaceWithMode("bow", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  // Bow front down, back straight in vertical stance
  setServoAngle(R4, 110); setServoAngle(L4, 70);
  setServoAngle(R3, 160); setServoAngle(L3, 20);
  delayWithFace(1500);
  runStandPose(1);
  if (currentCommand == "bow") currentCommand = "";
}

// ====== CUTE POSE ======
inline void runCutePose() {
  Serial.println(F("CUTE"));
  setFaceWithMode("cute", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  for (int i = 0; i < 4; i++) {
    if (currentCommand != "cute") break;
    setServoAngle(L1, 45); setServoAngle(R1, 75);
    delayWithFace(300);
    setServoAngle(L1, 15); setServoAngle(R1, 90);
    delayWithFace(300);
  }
  runStandPose(1);
  if (currentCommand == "cute") currentCommand = "";
}

// ====== FREAKY POSE ======
inline void runFreakyPose() {
  Serial.println(F("FREAKY"));
  setFaceWithMode("freaky", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  for (int i = 0; i < 3; i++) {
    if (currentCommand != "freaky") break;
    // R3 wiggles between 135° (slight forward bend) and 90° (horizontal kick)
    setServoAngle(R3, 135); delayWithFace(250);
    setServoAngle(R3, 90); delayWithFace(250);
  }
  runStandPose(1);
  if (currentCommand == "freaky") currentCommand = "";
}

// ====== WORM POSE ======
inline void runWormPose() {
  Serial.println(F("WORM"));
  setFaceWithMode("worm", FACE_ANIM_ONCE);
  runStandPose(0);
  delayWithFace(200);
  for (int i = 0; i < 4; i++) {
    if (currentCommand != "worm") break;
    // Step A: All bend slightly forward (L=40, R=140)
    setServoAngle(R3, 140); setServoAngle(L3, 40);
    setServoAngle(R4, 140); setServoAngle(L4, 40);
    delayWithFace(300);
    // Step B: All push back near vertical (L=10, R=170)
    setServoAngle(R3, 170); setServoAngle(L3, 10);
    setServoAngle(R4, 170); setServoAngle(L4, 10);
    delayWithFace(300);
  }
  runStandPose(1);
  if (currentCommand == "worm") currentCommand = "";
}

// ====== SHAKE POSE ======
inline void runShakePose() {
  Serial.println(F("SHAKE"));
  setFaceWithMode("shake", FACE_ANIM_ONCE);
  runStandPose(0);
  delayWithFace(200);
  for (int i = 0; i < 5; i++) {
    if (currentCommand != "shake") break;
    setServoAngle(R1, 80); setServoAngle(L1, 25);
    delayWithFace(200);
    setServoAngle(R1, 90); setServoAngle(L1, 5);
    delayWithFace(200);
  }
  runStandPose(1);
  if (currentCommand == "shake") currentCommand = "";
}

// ====== SHRUG POSE ======
inline void runShrugPose() {
  Serial.println(F("SHRUG"));
  setFaceWithMode("shrug", FACE_ANIM_ONCE);
  runStandPose(0);
  delayWithFace(200);
  // Both front paws lift to symmetric shrug (L4=60, R4=120)
  setServoAngle(R4, 120); setServoAngle(L4, 60);
  delayWithFace(1000);
  runStandPose(1);
  if (currentCommand == "shrug") currentCommand = "";
}

// ====== DEAD POSE ======
inline void runDeadPose() {
  Serial.println(F("DEAD"));
  setFaceWithMode("dead", FACE_ANIM_BOOMERANG);
  // Lay flat on ground
  setServoAngle(R3, 90); setServoAngle(L4, 90);
  setServoAngle(L3, 90); setServoAngle(R4, 90);
  if (currentCommand == "dead") currentCommand = "";
}

// ====== CRAB POSE ======
inline void runCrabPose() {
  Serial.println(F("CRAB"));
  setFaceWithMode("crab", FACE_ANIM_ONCE);
  runStandPose(0);
  delayWithFace(200);
  for (int i = 0; i < 4; i++) {
    if (currentCommand != "crab") break;
    setServoAngle(R2, 30); setServoAngle(L1, 0);
    delayWithFace(250);
    setServoAngle(R2, 0);  setServoAngle(L1, 30);
    delayWithFace(250);
  }
  runStandPose(1);
  if (currentCommand == "crab") currentCommand = "";
}

// ======================================================================
// CALIBRATED WALKING LOOPS (from sesame_reobo.ino original gait)
//
// Shoulder Stance: R1=45, R2=45, L1=45, L2=45
//
// Phase 0: Left: L4=15, L3=65 | Right: R4=120, R3=175
// Phase 1: Left: L4=60, L3=5  | Right: R4=165, R3=115
// Stride Delay: 300ms
// ======================================================================

inline void runWalkPose() {
  Serial.println(F("WALK FORWARD"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);

  // Set calibrated walking shoulder stance (All 45 deg)
  setServoAngle(R1, 45);
  setServoAngle(R2, 45);
  setServoAngle(L1, 45);
  setServoAngle(L2, 45);
  delayWithFace(60);

  for (int i = 0; i < walkCycles; i++) {
    // Phase 0: Left back/lift (L4=15, L3=65), Right forward/push (R4=120, R3=175)
    setServoAngle(L4, 15);
    setServoAngle(L3, 65);
    setServoAngle(R4, 120);
    setServoAngle(R3, 175);
    if (!pressingCheck("forward", frameDelay)) {
      runStandPose(1);
      return;
    }

    // Phase 1: Left forward/push (L4=60, L3=5), Right back/lift (R4=165, R3=115)
    setServoAngle(L4, 60);
    setServoAngle(L3, 5);
    setServoAngle(R4, 165);
    setServoAngle(R3, 115);
    if (!pressingCheck("forward", frameDelay)) {
      runStandPose(1);
      return;
    }
  }

  // Restore vertical standing stance on finish
  runStandPose(1);
}

inline void runWalkBackward() {
  Serial.println(F("WALK BACKWARD"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);

  // Set calibrated walking shoulder stance (All 45 deg)
  setServoAngle(R1, 45);
  setServoAngle(R2, 45);
  setServoAngle(L1, 45);
  setServoAngle(L2, 45);
  delayWithFace(60);

  for (int i = 0; i < walkCycles; i++) {
    // Phase 1 first
    setServoAngle(L4, 60);
    setServoAngle(L3, 5);
    setServoAngle(R4, 165);
    setServoAngle(R3, 115);
    if (!pressingCheck("backward", frameDelay)) {
      runStandPose(1);
      return;
    }

    // Phase 0 second
    setServoAngle(L4, 15);
    setServoAngle(L3, 65);
    setServoAngle(R4, 120);
    setServoAngle(R3, 175);
    if (!pressingCheck("backward", frameDelay)) {
      runStandPose(1);
      return;
    }
  }

  runStandPose(1);
}

inline void runTurnLeft() {
  Serial.println(F("TURN LEFT"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);

  // Shoulder stance: All 45 deg
  setServoAngle(R1, 45);
  setServoAngle(R2, 45);
  setServoAngle(L1, 45);
  setServoAngle(L2, 45);
  delayWithFace(60);

  for (int i = 0; i < walkCycles; i++) {
    // Right side drives forward while left stays planted in vertical stance (L4=20, L3=20)
    setServoAngle(R4, 120);
    setServoAngle(R3, 175);
    setServoAngle(L4, 20);
    setServoAngle(L3, 20);
    if (!pressingCheck("left", frameDelay)) {
      runStandPose(1);
      return;
    }

    setServoAngle(R4, 165);
    setServoAngle(R3, 115);
    setServoAngle(L4, 20);
    setServoAngle(L3, 20);
    if (!pressingCheck("left", frameDelay)) {
      runStandPose(1);
      return;
    }
  }

  runStandPose(1);
}

inline void runTurnRight() {
  Serial.println(F("TURN RIGHT"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);

  // Shoulder stance: All 45 deg
  setServoAngle(R1, 45);
  setServoAngle(R2, 45);
  setServoAngle(L1, 45);
  setServoAngle(L2, 45);
  delayWithFace(60);

  for (int i = 0; i < walkCycles; i++) {
    // Left side drives forward while right stays planted in vertical stance (R4=160, R3=160)
    setServoAngle(L4, 15);
    setServoAngle(L3, 65);
    setServoAngle(R4, 160);
    setServoAngle(R3, 160);
    if (!pressingCheck("right", frameDelay)) {
      runStandPose(1);
      return;
    }

    setServoAngle(L4, 60);
    setServoAngle(L3, 5);
    setServoAngle(R4, 160);
    setServoAngle(R3, 160);
    if (!pressingCheck("right", frameDelay)) {
      runStandPose(1);
      return;
    }
  }

  runStandPose(1);
}

# SERVO MOTOR & BATTERY TESTING GUIDE

## 1. Components covered

This procedure uses the component list you supplied. Verify the exact servo model and wiring from its label or datasheet before applying power.

| Product                                                                 | Unit Price (₹) | Quantity | Line Total (₹) | LInks                                                                                                                                         |
| ----------------------------------------------------------------------- | -------------- | -------- | -------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| TP4056 Battery Charging Protection Module (Type C)                      | ₹18.00         | 1        | ₹18.00         | [link](https://quartzcomponents.com/collections/all/products/tp4056-battery-charging-protection-module-type-c)                                |
| WATTNINE® 7.4V 10000mAh Rechargeable Lithium Battery Pack with Warranty | ₹1,536.00      | 1        | ₹1,536.00      | [link](https://quartzcomponents.com/collections/all/products/7-4v-10000mah-rechargeable-lithium-battery-pack-with-warranty-includes-bms-2s2p) |
| Wire Stripper and Cutter - 130mm                                        | ₹35.00         | 1        | ₹35.00         | [link](https://quartzcomponents.com/collections/all/products/wire-stripper-and-cutter-130mm)                                                  |
| Flux Core Solder Wire 18SWG (50Gms) 40/60                               | ₹170.00        | 1        | ₹170.00        | [link](https://quartzcomponents.com/collections/all/products/flux-core-solder-wire-18swg-50gms)                                               |
| Soldering and Desoldering Flux Paste (Yellow)                           | ₹22.00         | 1        | ₹22.00         | [link](https://quartzcomponents.com/collections/all/products/soldering-and-desoldering-flux-paste-white)                                      |
| Soldering Iron Stand                                                    | ₹55.00         | 1        | ₹55.00         | [link](https://quartzcomponents.com/collections/all/products/soldering-iron-stand)                                                            |
| Soldering Bit Cleaner Foam Pad                                          | ₹23.00         | 1        | ₹23.00         | [link](https://quartzcomponents.com/collections/all/products/soldering-bit-cleaner-foam-pad)                                                  |
| Heat Shrink Sleeve Tube - 2mm Diameter - Black - 1 meter                | ₹7.00          | 3        | ₹21.00         | [link](https://quartzcomponents.com/collections/all/products/heat-shrink-tube-3-mm-diameter-black-1-meter)                                    |
| 40 Pin Straight Female Header Pins - Berg Strips                        | ₹10.00         | 3        | ₹30.00         | [link](https://quartzcomponents.com/collections/all/products/40-pin-straight-female-berg-strips)                                              |
| Ribbon Cable / Multi-Strand Cable (1 Meter) Good Quality                | ₹49.00         | 2        | ₹98.00         | [link](https://quartzcomponents.com/collections/all/products/ribbon-cable-multi-strand-cable-1-meter)                                         |
| ESP32-C3 Super Mini Development Board - HW-466AB                        | ₹235.00        | 1        | ₹235.00        | [link](https://quartzcomponents.com/products/esp32-c3-super-mini-development-board-with-soldered-headers-hw-466ab)                            |
| 9x15cm Double Sided Universal PCB Prototype Board                       | ₹108.00        | 1        | ₹108.00        | [link](https://quartzcomponents.com/collections/all/products/double-sided-universal-pcb-prototype-board-9x15cm)                               |
| 3x7cm Double Sided Universal PCB Prototype Board                        | ₹25.00         | 1        | ₹25.00         | [link](https://quartzcomponents.com/collections/all/products/double-sided-universal-pcb-prototype-board-3x7cm)                                |
| MINI560 5V 5A DC-DC Step-Down Stabilized Voltage Module                 | ₹65.00         | 1        | ₹65.00         | [link](https://quartzcomponents.com/collections/all/products/mini560-dc-dc-step-down-stabilized-voltage-module)                               |
| TowardPro MG90S Mini Digital Servo Motor (180° Rotation) - High Quality | ₹279.00        | 9        | ₹2,511.00      | [link](https://robu.in/product/towardpro-mg90s-mini-digital-servo-2-2kg-0-08sec-13g/)                                                         |

## 2. Important safety rules

The 2S battery pack is nominally 7.4 V and typically reaches 8.4 V when fully charged. Never connect it directly to a 5 V servo or the ESP32 5 V pin.

Before using the MINI560, confirm its manufacturer documentation supports the pack's maximum input voltage (at least 8.4 V) and that it can provide the required continuous and peak current. Do not assume the advertised 5 A rating is available under every condition.

Set and measure the converter output to 5.0 V before connecting a servo. If the converter's output is adjustable, adjust it with no servo connected and verify polarity.

Do not assume a 5 A supply can safely drive all 10 servos at once. Servo startup/stall current can be high. Test one at a time; use a suitably rated supply and wiring for multi-servo operation.

Power the servos from the external regulated 5 V output, not from the ESP32 board. Connect ESP32 GND, converter output GND, and servo GND together.

Disconnect battery power before changing wiring. Prevent exposed wires from touching. Never short battery terminals or use a multimeter in current mode directly across a battery.

Stop using a pack that is swollen, leaking, damaged, unusually hot, or has damaged leads. Use only the supplied/appropriate charger designed for this exact 2S pack; charge on a non-flammable surface and do not leave unattended.

Do not open the battery pack or test individual internal cells unless you are trained and the pack is designed to be serviced.

## 3. Test the MINI560 converter first

With the battery disconnected, inspect the converter and confirm its input/output terminals and polarity from its markings or datasheet.

Connect the battery pack to the converter input only if the converter input rating explicitly supports up to 8.4 V.

Set the multimeter to DC volts. Measure the converter output and confirm approximately 5.0 V before connecting the ESP32 or any servo.

If output voltage is wrong, unstable, reversed, or the module heats rapidly, disconnect it and do not proceed.

If the converter is fixed-output, still measure it before attaching components.

## 4. Test each servo motor individually

### 4.1 Wiring

Servo signal (often orange/yellow/white) → ESP32 GPIO 13 for this test.

Servo positive (often red) → converter regulated +5 V output.

Servo ground (often brown/black) → converter output GND.

ESP32 GND → converter output GND (common ground).

Connect ESP32 to the computer using the micro USB data cable. Keep the servo unloaded and clear of obstructions.

Wire colors vary; verify the actual servo pinout. Never connect the servo's +5 V lead to a GPIO.

### 4.2 Procedure and pass/fail criteria

Label the first servo S01. Inspect its case, shaft, cable, and connector.

Power off, connect that servo using Section 4.1, and double-check polarity and common ground.

Power on and observe at least five cycles between 60°, 90°, and 120°.

PASS if motion is smooth and repeatable, with no abnormal grinding, persistent buzzing, stalling, excessive heat, or ESP32 resets.

FAIL / isolate if it does not move, moves erratically, repeatedly resets the ESP32, stalls, smells burnt, or becomes hot. Disconnect power promptly.

Power off before disconnecting it. Repeat for S02 through S10 and record every result in Section 7.

If a servo fails, test the same wiring with a known-good servo and test the suspect servo on a known-good channel/supply to isolate the cause.

## 5. Battery pack checks

This is a basic inspection and voltage check, not a capacity test. A voltage reading alone cannot confirm the pack still delivers its rated 1000 mAh.

Read the pack label and verify it says 2S / 7.4 V nominal. Do not rely only on the product title.

Inspect the pack, connector, and wires. Do not use it if swollen, punctured, leaking, crushed, corroded, or unusually hot.

Disconnect the pack from the circuit. Set the multimeter to DC volts and measure across the pack output terminals, taking care not to bridge them with the probes.

Record the voltage and compare it with the battery manufacturer's permitted range. A typical fully charged 2S Li-ion pack is around 8.4 V, but the label/datasheet and protection circuitry take priority.

If voltage is 0 V, unexpectedly low, above the pack's specified maximum, or fluctuates abnormally, do not use or attempt to revive it; have it checked with an appropriate battery tester or supplier.

Use only the correct charger for this exact 2S pack. Do not charge through the MINI560 converter and do not bypass pack protection.

For capacity/runtime testing, use a suitable battery analyzer/electronic load with the correct cutoff settings, or ask the supplier to test it. Do not improvise a short-circuit or high-current load test.

## 6. Optional integrated power test

Only after the converter and each servo have passed individual tests, connect one servo to the 5 V converter output and run several cycles.

Measure the 5 V rail during movement if you can do so safely. Large voltage dips, converter shutdown, hot wires, or ESP32 resets indicate the supply/wiring may be inadequate.

Add further servos one at a time while monitoring the 5 V rail and converter temperature. Stop if voltage becomes unstable or anything heats abnormally.

Do not run all 10 servos from this battery/converter setup until the actual servo current requirements, converter thermal limits, battery discharge rating, and wiring ratings have been verified.

## 7. Servo test record

Mark each field after testing. Add notes for any unusual behavior.

| Servo ID | Smooth movement | Noise / heat OK | 5+ cycles OK | Result | Notes |
| --- | --- | --- | --- | --- | --- |
| S01 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |
| S02 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |
| S03 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |
| S04 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |
| S05 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |
| S06 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |
| S07 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |
| S08 | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Yes ☐ No | ☐ Pass ☐ Fail |  |

## 8. Battery and converter record

| Item | Reading / observation | Expected / reference | Result | Notes |
| --- | --- | --- | --- | --- |
| 2S battery pack voltage |  | Compare with pack label/datasheet | ☐ Pass ☐ Hold |  |
| Battery physical condition |  | No damage, swelling, leakage, unusual heat | ☐ Pass ☐ Hold |  |
| MINI560 input rating |  | Supports pack maximum voltage (up to 8.4 V) | ☐ Pass ☐ Hold |  |
| MINI560 output voltage |  | Approximately 5.0 V before load | ☐ Pass ☐ Fail |  |
| 5 V rail under one-servo movement |  | Stable; no reset or shutdown | ☐ Pass ☐ Fail |  |

## 9. Final checklist

- ☐ All 10 servos have individual test results and IDs.

- ☐ Battery pack inspected and voltage recorded.

- ☐ MINI560 input rating verified for a fully charged 2S pack.

- ☐ Converter output verified at approximately 5.0 V.

- ☐ Common ground and polarity checked.

- ☐ Any failed or suspicious component is isolated and not installed.

- ☐ Current rating of the converter, battery, and wires is confirmed before multi-servo operation.

Tested by: ________________________    Date: ________________

Project / robot: _______________________________________________

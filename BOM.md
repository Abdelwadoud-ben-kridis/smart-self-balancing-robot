# BOM — Self-Balancing Articulated Leg Robot (DocPRB-SBR-001, Rev A)

## 1. Microcontrollers & Compute

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 1 | ESP32-WROOM-32 Dev Board | Dual-core 240 MHz, Wi-Fi/BT | 1 | Real-time balance loop @ 100 Hz |
| 2 | Raspberry Pi 5 | 4 GB RAM | 1 | Vision pipeline + behavior state machine |

## 2. Sensors

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 3 | MPU-6500 IMU Module | 6-axis (gyro + accel), SPI | 1 | Complementary filter (98% gyro / 2% accel) |
| 4 | USB Webcam | USB 2.0, compatible with Pi 5 | 1 | Vision input for YOLOv8n / MediaPipe |

## 3. Motors & Actuators

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 5 | MG996R Servo Motor | 11.0 kg-cm torque, metal gear | 4 | Hip (x2) and knee (x2) joints |
| 6 | JGA25-370 DC Geared Motor | 11.1V direct drive, ~1.2A peak | 2 | Wheel drive motors |

## 4. Motor Drivers & PWM

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 7 | L298N Dual H-Bridge Module | 2-ch, up to 2A/ch | 1 | PWM+DIR for 2x JGA25-370 |
| 8 | PCA9685 16-Ch PWM Driver | I2C, 12-bit resolution | 1 | 4 channels used for MG996R servos |

## 5. Power System

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 9 | 18650 Li-Ion Cell | 3.7V, 2600 mAh | 3 | 3S1P config = 11.1V nominal |
| 10 | 3S 18650 Battery Holder | Series holder for 3 cells | 1 | — |
| 11 | 3S BMS Board | 11.1V, balance + under-voltage protection | 1 | Cell-level cutoff |
| 12 | DC-DC Buck Converter (6V) | Input 7-12V to 6V out, >=2.5A | 1 | Servo rail (MG996R) |
| 13 | DC-DC Buck Converter (5V) | Input 7-12V to 5V out, >=3A | 1 | Logic rail (Pi 5 + ESP32) |
| 14 | Power Switch | SPST toggle, >=5A rated | 1 | Main power on/off |

## 6. Display & I/O

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 15 | 3.5" SPI TFT Display | 3.3V logic, SPI interface | 1 | Status/diagnostics readout |

## 7. Wheels & Mechanical

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 16 | Rubber Wheel | 65 mm diameter | 2 | Press-fit or hub-mount to JGA25-370 |
| 17 | Motor Mounting Bracket (JGA25-370) | Aluminum/3D-printed | 2 | Secure DC motors to chassis |
| 18 | Servo Horn / Arm | 97 mm effective length, MG996R compatible | 4 | Four-bar linkage arms |
| 19 | 20T Spur Gear | 1:1 ratio pair | 4 pairs | Linkage gear train |

## 8. 3D-Printed Parts (PLA or PETG, <=30% infill)

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 20 | Main Body / Chassis | 18 x 15 cm footprint, 2 internal floors | 1 | Top + bottom PCB mounting layers |
| 21 | Upper Leg Link | 97 mm arm length | 2 | Left + right |
| 22 | Lower Leg Link | 97 mm arm length | 2 | Left + right |
| 23 | Foot Pad | Flat contact surface | 2 | <=4 mm drift at full extension |
| 24 | Servo Mounts | MG996R compatible | 4 | Hip and knee positions |
| 25 | Pi 5 / ESP32 Mount Plate | Standoff compatible | 1 | Internal floor mounting |
| 26 | Battery Compartment | Fits 3S 18650 holder | 1 | Accessible for charging/swap |

## 9. Wiring & Connectors

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 27 | Dupont Jumper Wires (M-F) | 20 cm assorted | ~30 | I2C, SPI, GPIO connections |
| 28 | Dupont Jumper Wires (F-F) | 20 cm assorted | ~10 | Sensor/display hookup |
| 29 | Silicone Wire 18 AWG (Red) | High-current power runs | ~1 m | Battery to BMS to buck to L298N |
| 30 | Silicone Wire 18 AWG (Black) | Ground runs | ~1 m | Power ground distribution |
| 31 | Silicone Wire 22 AWG (assorted) | Signal-level wiring | ~2 m | Servo, UART, misc |
| 32 | JST-XH Connectors (2/3/4-pin) | Crimp connectors | ~10 | Clean modular connections |
| 33 | XT30 or XT60 Connector Pair | Battery plug | 1 pair | Main battery disconnect |
| 34 | UART Jumper Cable | 3-wire (TX/RX/GND) | 1 | Pi 5 to ESP32 @ 115200 baud |

## 10. Fasteners & Hardware

| # | Component | Spec | Qty | Notes |
|---|-----------|------|:---:|-------|
| 35 | M3x8 mm Socket Head Screws | Stainless steel | ~30 | Servo mounting, bracket attachment |
| 36 | M3x12 mm Socket Head Screws | Stainless steel | ~10 | Motor brackets, chassis assembly |
| 37 | M3 Nuts | Stainless steel | ~40 | General fastening |
| 38 | M3 Lock Nuts (Nyloc) | Stainless steel | ~10 | Vibration-prone joints |
| 39 | M2.5x10 mm Standoffs | Brass/nylon | ~8 | PCB/board mounting |
| 40 | M2.5 Screws + Nuts | Stainless steel | ~16 | Standoff securing |
| 41 | Zip Ties (small) | 100 mm | ~10 | Cable management |
| 42 | Heat Shrink Tubing | Assorted diameters | ~0.5 m | Solder joint insulation |

## Summary

| Category | Item Count |
|----------|:----------:|
| Electronics & Compute | 8 unique components |
| Motors & Actuators | 6 units total |
| Power System | 6 unique components |
| 3D-Printed Parts | ~7 custom parts |
| Wiring & Connectors | ~8 line items |
| Fasteners | ~8 line items |
| **Total unique line items** | **~42** |

**Target assembled mass:** ~471 g (body only, excl. battery)
**Standing height:** ~220 mm | **Sitting height:** ~66 mm | **Footprint:** 18 x 15 cm

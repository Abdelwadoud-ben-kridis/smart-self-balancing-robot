# Self-Balancing Articulated Leg Robot — Complete Wiring

Two-floor robot body (18×15cm). Floor 1 (top): Raspberry Pi 5, ESP32, MPU-6500 GY module, 3.5" SPI screen. Floor 2 (bottom): 3S 18650 battery, BMS, power switch, 2× buck converters, L298N, PCA9685, signal conditioning PCB.

---

## POWER CHAIN

Battery (3S 18650, 11.1V nominal) → BMS → power switch → main 11.1V rail.

11.1V rail splits three ways:
- → Buck converter 1 → 6V → PCA9685 V+ rail and all 4 MG996R servos
- → Buck converter 2 → 5V → Raspberry Pi 5 (USB-C or GPIO 5V pin) and ESP32 VIN
- → L298N VSS (motor power, direct 11.1V)

ESP32 onboard regulator → 3.3V → MPU-6500 VCC, PCA9685 logic VCC, signal conditioning PCB +3V3 rail

---

## SIGNAL CONDITIONING PCB (57×60mm)

Receives:
- J6 pin 1: +3V3 from ESP32 3V3
- J6 pin 2: GND
- J6 pin 3: +6V from Buck 1

### Encoder 1 (left motor JGA25-370)

- J1 pin 1: +3V3 → encoder VCC
- J1 pin 2: GND
- J1 pin 3: ENC1_A → R1 (10kΩ pull-up to +3V3) → C2 (1nF to GND) → filtered ENC1_A
- J1 pin 4: ENC1_B → R2 (10kΩ pull-up to +3V3) → C3 (1nF to GND) → filtered ENC1_B

### Encoder 2 (right motor JGA25-370)

- J2 pin 1: +3V3 → encoder VCC
- J2 pin 2: GND
- J2 pin 3: ENC2_A → R3 (10kΩ pull-up to +3V3) → C6 (1nF to GND) → filtered ENC2_A
- J2 pin 4: ENC2_B → R4 (10kΩ pull-up to +3V3) → C7 (1nF to GND) → filtered ENC2_B

RC filter cutoff: 1/(2π × 10kΩ × 1nF) ≈ 16kHz — kills motor PWM switching noise.

### IMU (MPU-6500 GY module)

- J8 pin 1: +3V3 → IMU VCC
- J8 pin 2: GND
- J8 pin 3: I2C_SDA
- J8 pin 4: I2C_SCL
- J8 pin 5: MPU_INT → R7 (4.7kΩ pull-up to +3V3) → MPU_INT net

### PCA9685 I2C (J3, 4-pin socket)

- pin 1: +3V3
- pin 2: GND
- pin 3: I2C_SDA
- pin 4: I2C_SCL

### PCA9685 power (J4, 2-pin socket)

- pin 1: +6V
- pin 2: GND

### ESP32 output header (J5, 9-pin)

- pin 1: +3V3
- pin 2: GND
- pin 3: I2C_SDA → ESP32 GPIO 21
- pin 4: I2C_SCL → ESP32 GPIO 22
- pin 5: ENC1_A (filtered) → ESP32 GPIO (TBD)
- pin 6: ENC1_B (filtered) → ESP32 GPIO (TBD)
- pin 7: ENC2_A (filtered) → ESP32 GPIO (TBD)
- pin 8: ENC2_B (filtered) → ESP32 GPIO (TBD)
- pin 9: MPU_INT → ESP32 GPIO (TBD)

---

## ESP32 GPIO ASSIGNMENTS

| GPIO | Connected to | Function |
|------|-------------|----------|
| 21 | Signal PCB J5 pin 3 / IMU SDA / PCA9685 SDA | I2C SDA (400kHz) |
| 22 | Signal PCB J5 pin 4 / IMU SCL / PCA9685 SCL | I2C SCL (400kHz) |
| 25 | L298N ENA | Left motor speed (PWM, 1kHz) |
| 26 | L298N IN1 | Left motor direction 1 |
| 27 | L298N IN2 | Left motor direction 2 |
| 32 | L298N ENB | Right motor speed (PWM, 1kHz) |
| 33 | L298N IN3 | Right motor direction 1 |
| 4 | L298N IN4 | Right motor direction 2 |
| TBD | Signal PCB J5 pin 5 | ENC1_A |
| TBD | Signal PCB J5 pin 6 | ENC1_B |
| TBD | Signal PCB J5 pin 7 | ENC2_A |
| TBD | Signal PCB J5 pin 8 | ENC2_B |
| TBD | Signal PCB J5 pin 9 | MPU_INT |

---

## L298N MOTOR DRIVER

| L298N pin | Connects to | Notes |
|-----------|------------|-------|
| VSS | 11.1V battery | Motor power |
| VCC | 5V | Logic power |
| GND | GND | |
| ENA | ESP32 GPIO 25 | Left motor enable/speed (PWM) |
| IN1 | ESP32 GPIO 26 | Left motor direction |
| IN2 | ESP32 GPIO 27 | Left motor direction |
| ENB | ESP32 GPIO 32 | Right motor enable/speed (PWM) |
| IN3 | ESP32 GPIO 33 | Right motor direction |
| IN4 | ESP32 GPIO 4 | Right motor direction |
| OUT1 + OUT2 | Left JGA25-370 motor terminals | |
| OUT3 + OUT4 | Right JGA25-370 motor terminals | |

---

## PCA9685 SERVO DRIVER

I2C address 0x40. Powered by 6V (V+) and 3.3V (VCC logic). I2C from ESP32 via signal PCB.

| Channel | Servo | Direction |
|---------|-------|-----------|
| CH0 | Left knee MG996R | Normal (pulse 150–600) |
| CH1 | Right knee MG996R | Reversed (pulse = 750 − normal) |
| CH2 | Left hip MG996R | Normal |
| CH3 | Right hip MG996R | Reversed (pulse = 750 − normal) |
| CH4–15 | Unused | |

Pulse map: 0° = 150, 90° = 375, 180° = 600. Sit: knee 80°, hip −80°. Stand: knee 0°, hip 0°.

---

## MPU-6500 IMU (GY module)

Plugs into signal PCB J8 (5-pin socket). I2C address 0x68.
- Axis convention: pitch rate = gyro Y (gy), pitch angle = atan2(ax, az)
- INT pin → R7 pull-up → J5 pin 9 → ESP32

---

## RASPBERRY PI 5

- Power: 5V from Buck 2 → GPIO header pin 4 (or USB-C)
- 3.5" SPI screen: plugs directly onto RPi 40-pin GPIO header (HAT style, no PCB involved)
  - Display SPI0: BCM 8 (CS), BCM 9 (MISO), BCM 10 (MOSI), BCM 11 (SCLK)
  - Control: BCM 25 (DC), BCM 27 (RST)
  - Touch: BCM 7 (CS), BCM 17 (IRQ)
- Communication with ESP32: UART or USB (TBD)

---

## JGA25-370 MOTORS WITH ENCODERS

Each motor has 6 wires:
- 2 motor power wires → L298N OUT pins
- Red → encoder +3V3 (via signal PCB J1/J2 pin 1)
- Black → GND (via signal PCB J1/J2 pin 2)
- Green → encoder channel A (via signal PCB J1/J2 pin 3)
- White → encoder channel B (via signal PCB J1/J2 pin 4)

---

## MG996R SERVOS

Each servo has 3 wires (standard servo connector):
- Brown → GND
- Red → +6V (from Buck 1 via PCA9685 V+ rail)
- Orange → PWM signal from PCA9685 channel output

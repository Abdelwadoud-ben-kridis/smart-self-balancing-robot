# Self-Balancing Articulated Leg Robot — Complete Wiring
**Revision C — Updated to match verified PCB robot_pcb Rev C**

Two-floor robot body (18×15cm).
- **Floor 1 (top):** Raspberry Pi 5, ESP32 DevKit, GY-6500 IMU module, 3.5" SPI screen
- **Floor 2 (bottom):** 3S 18650 battery + BMS, power switch, 2× XL4016 buck converters, L298N H-bridge, PCA9685 PWM driver, distribution/interface PCB

> **Changes from previous wiring doc:**
> - PCB correctly identified as 4-layer distribution board (94.6 × 58.1mm), not "57×60mm signal conditioning PCB"
> - IMU corrected to **SPI** (not I2C) — GY-6500 uses SPI_CLK/MOSI/MISO/CS
> - All TBD GPIO assignments filled in from verified PCB
> - L298N GPIO table corrected (was wrong on GPIO26/27/32/4)
> - Power chain corrected — PCB generates +3V3 internally; ESP32 receives 5V from PCB
> - Encoder filter corrected — series RC (1kΩ + 10nF), not pull-up + 1nF
> - PCA9685 logic VCC = +5V (not 3V3)


---

## 1. POWER CHAIN

```
3S 18650 (11.1V nom, 9–12.6V)
    → BMS
    → Power switch
    → 11.1V main rail
        │
        ├──→ XL4016-A (set to 6.0V before connecting)
        │       → +6V_SERVO rail
        │           → PCA9685 V+ (via PCB J2.P2/P3)    [⚠ pending J6.P5 fix]
        │           → 4× MG996R servo red wire
        │
        ├──→ XL4016-B (set to 5.0V before connecting)
        │       → +5V rail
        │           → PCB J3 → AMS1117 → +3V3 (PCB internal)
        │           → PCB J3 → ESP32 VIN (via PCB J13.P15)
        │           → PCB J3 → RPi 5 GPIO pin 4 (via PCB J10)
        │
        └──→ L298N VSS (11.1V motor power, direct — NOT through PCB)
```

**+3V3 generation:** AMS1117-3.3 on PCB takes +5V → outputs +3V3/1A.
+3V3 rail feeds: ESP32 3V3 pin, IMU VCC, encoder VCC, I2C pull-ups.

---

## 2. DISTRIBUTION & INTERFACE PCB (94.6 × 58.1mm, 4-layer)

### PCB connector summary

| Connector | Type | Function |
|-----------|------|----------|
| J1 | 2-pin 2.54mm | Battery input: +VBAT + GND |
| J2 | 4-pin 2.54mm | Buck A interface: VBAT_SW out / 6V_SERVO in |
| J3 | 2-pin 2.54mm | +5V input from XL4016-B |
| J5 | 8-pin 2.54mm socket | GY-6500 IMU (SPI) |
| J6 | 6-pin 2.54mm socket | PCA9685 module (I2C + power) |
| J7 | 4-pin Phoenix 5.08mm | Encoder 1 (left motor) |
| J8 | 4-pin Phoenix 5.08mm | Encoder 2 (right motor) |
| J9 | 6-pin Phoenix 5.08mm | L298N control signals only (no power) |
| J10 | 2-pin 2.54mm | RPi 5V power |
| J11 | 2-pin 2.54mm | RPi → ESP32 UART |
| J12 | 15-pin socket 2.54mm | ESP32 left header |
| J13 | 15-pin socket 2.54mm | ESP32 right header |

---

### J2 — Buck Converter A Interface (4-pin)

| Pin | Net | Direction | Wire to |
|-----|-----|-----------|---------|
| 1 | /VBAT_SWITCHED | OUT | XL4016-A input terminal |
| 2 | +6V_SERVO | IN | XL4016-A output terminal |
| 3 | +6V_SERVO | IN | PCA9685 V+ (parallel to P2) |
| 4 | GND | — | XL4016-A GND |

---

### J5 — GY-6500 IMU (SPI mode, 8-pin)

| Pin | Net | GY-6500 pin | Notes |
|-----|-----|-------------|-------|
| 1 | +3V3 | VCC | Module power |
| 2 | GND | GND | |
| 3 | Net-(J5-3) → R7 → /SPI_CLK | SCL | SPI clock |
| 4 | Net-(J5-4) → R8 → /SPI_MOSI | SDA | SPI MOSI |
| 5 | Net-(J5-5) → R9 → /SPI_MISO | SDO/AD0 | SPI MISO |
| 6 | Net-(J5-6) → R10 → /SPI_CS_IMU | CSB/NCS | Chip select — must be LOW for SPI mode |
| 7 | /IMU_INT | INT | Interrupt → ESP32 GPIO4 |
| 8 | GND | FSYNC | Tied to GND (no frame sync needed) |

33Ω series damping resistors R7–R10 on all SPI lines.

---

### J6 — PCA9685 Module (6-pin)

| Pin | Net | PCA9685 pin | Notes |
|-----|-----|-------------|-------|
| 1 | +5V | VCC | Logic supply |
| 2 | GND | GND | |
| 3 | /I2C_SDA | SDA | Shared I2C bus |
| 4 | /I2C_SCL | SCL | Shared I2C bus |
| 5 | ⚠ +5V (must be +6V_SERVO) | V+ | Servo power — PCB error pending fix in schematic |
| 6 | GND | GND | Servo power return |

> ⚠ **J6.P5 PCB error not yet fixed.** Servos currently receive 5V, not 6V. Fix schematic before ordering.

---

### J7 / J8 — Encoder Connectors (Phoenix 5.08mm, 4-pin)

| Pin | Net | Wire to |
|-----|-----|---------|
| 1 | +3V3 | Encoder VCC (red) |
| 2 | GND | Encoder GND (black) |
| 3 | Net-(Jx-3) → 1kΩ → /ENCx_A → 10nF → GND | Encoder Ch A (green) |
| 4 | Net-(Jx-4) → 1kΩ → /ENCx_B → 10nF → GND | Encoder Ch B (white) |

RC filter: 1kΩ series + 10nF shunt → fc = 15.9kHz (kills motor PWM noise)

---

### J9 — L298N Control (Phoenix 5.08mm, 6-pin)

**No power through J9. L298N VSS and GND wire directly to battery.**

| Pin | Net | L298N pin |
|-----|-----|-----------|
| 1 | /H_ENA | ENA |
| 2 | /H_ENB | ENB |
| 3 | /H_IN1 | IN1 |
| 4 | /H_IN2 | IN2 |
| 5 | /H_IN3 | IN3 |
| 6 | /H_IN4 | IN4 |

---

### J10 — RPi Power (2-pin)

| Pin | Net | Wire to |
|-----|-----|---------|
| 1 | +5V | RPi GPIO header pin 4 |
| 2 | GND | RPi GPIO header pin 6 |

---

### J11 — RPi UART (2-pin)

| Pin | Net | Wire to |
|-----|-----|---------|
| 1 | /RPi_RX | RPi UART TX (GPIO14/TXD, BCM14) |
| 2 | GND | RPi GND |

One-way: RPi → ESP32 only. ESP32 receives on GPIO35 (input-only pin).

---

## 3. ESP32 GPIO ASSIGNMENTS (Complete)

| GPIO | Net on PCB | Connected to | Function |
|------|-----------|--------------|----------|
| 4 | /IMU_INT | J5.P7 → GY-6500 INT | MPU-6500 interrupt |
| 5 | /SPI_CS_IMU | J5.P6 via R10 | IMU chip select (HIGH = idle) |
| 13 | /ENC1_A | J7 via R13+C7 | Encoder 1 Ch A |
| 14 | /ENC1_B | J7 via R14+C8 | Encoder 1 Ch B |
| 15 | /H_IN4 | J9.P6 → L298N IN4 | ⚠ HIGH at boot (internal pull-up) — write LOW in setup() |
| 16 | /ENC2_A | J8 via R15+C9 | Encoder 2 Ch A (RX2 pin, used as GPIO) |
| 17 | /ENC2_B | J8 via R16+C10 | Encoder 2 Ch B (TX2 pin, used as GPIO) |
| 18 | /SPI_CLK | J5.P3 via R7 | SPI clock to IMU |
| 19 | /SPI_MISO | J5.P5 via R9 | SPI MISO from IMU |
| 21 | /I2C_SDA | J6.P3, PCA9685 | I2C data (400kHz) |
| 22 | /I2C_SCL | J6.P4, PCA9685 | I2C clock (400kHz) |
| 23 | /SPI_MOSI | J5.P4 via R8 | SPI MOSI to IMU |
| 25 | /H_ENA | J9.P1 → L298N ENA | Left motor enable / PWM speed |
| 26 | /H_ENB | J9.P2 → L298N ENB | Right motor enable / PWM speed |
| 27 | /H_IN1 | J9.P3 → L298N IN1 | Left motor direction A |
| 32 | /H_IN2 | J9.P4 → L298N IN2 | Left motor direction B |
| 33 | /H_IN3 | J9.P5 → L298N IN3 | Right motor direction A |
| 35 | /RPi_RX | J11.P1 → RPi TX | UART from RPi (input-only GPIO) |
| 12 | NC | — | Boot-strap: NC, internal pull-down → LOW at boot ✓ |
| 2 | NC | — | Unused (onboard LED on some DevKits) |

---

## 4. L298N MOTOR DRIVER

| L298N pin | Connects to | Notes |
|-----------|------------|-------|
| VSS | 11.1V battery (direct) | Motor power — NOT through PCB |
| VCC | +5V rail | Logic power |
| GND | GND | |
| ENA | ESP32 GPIO25 via PCB J9.P1 | Left motor enable / PWM (1kHz) |
| IN1 | ESP32 GPIO27 via PCB J9.P3 | Left motor direction A |
| IN2 | ESP32 GPIO32 via PCB J9.P4 | Left motor direction B |
| ENB | ESP32 GPIO26 via PCB J9.P2 | Right motor enable / PWM (1kHz) |
| IN3 | ESP32 GPIO33 via PCB J9.P5 | Right motor direction A |
| IN4 | ESP32 GPIO15 via PCB J9.P6 | Right motor direction B — ⚠ HIGH at boot |
| OUT1 + OUT2 | Left JGA25-370 motor terminals | |
| OUT3 + OUT4 | Right JGA25-370 motor terminals | |

> **Motor control logic:**
> - Forward: ENA/ENB PWM, IN1=HIGH IN2=LOW, IN3=HIGH IN4=LOW
> - Reverse: ENA/ENB PWM, IN1=LOW IN2=HIGH, IN3=LOW IN4=HIGH
> - Stop: ENA=ENB=LOW

---

## 5. PCA9685 SERVO DRIVER

I2C address 0x40. Logic VCC = 5V. Servo power V+ = 6V (via J6.P5 after fix).
I2C from ESP32 GPIO21/22 via PCB.

| Channel | Servo | Mounting | Pulse range |
|---------|-------|----------|-------------|
| CH0 | Left knee MG996R | Normal | 150–600 |
| CH1 | Right knee MG996R | Mirrored | Reversed (750 − normal) |
| CH2 | Left hip MG996R | Normal | 150–600 |
| CH3 | Right hip MG996R | Mirrored | Reversed (750 − normal) |
| CH4–15 | Unused | — | — |

Pulse map: 0° = 150 · 90° = 375 · 180° = 600
Sit position: knee 80°, hip −80° · Stand: knee 0°, hip 0°

---

## 6. MPU-6500 IMU — GY-6500 Module (SPI Mode)

Plugs into PCB J5 (8-pin socket). **SPI mode — NOT I2C.**

| GY-6500 pin | PCB J5 pin | Signal | Notes |
|-------------|-----------|--------|-------|
| VCC | 1 | +3V3 | |
| GND | 2 | GND | |
| SCL | 3 | /SPI_CLK (via R7) | SPI clock from GPIO18 |
| SDA | 4 | /SPI_MOSI (via R8) | SPI MOSI from GPIO23 |
| SDO/AD0 | 5 | /SPI_MISO (via R9) | SPI MISO to GPIO19 |
| CSB/NCS | 6 | /SPI_CS_IMU (via R10) | CS from GPIO5 — hold LOW for SPI mode |
| INT | 7 | /IMU_INT | Interrupt to GPIO4 |
| FSYNC | 8 | GND | Tie to GND |

Axis convention: pitch rate = gyro Y · pitch angle = atan2(ax, az)

---

## 7. RASPBERRY PI 5

| Connection | From | To | Notes |
|-----------|------|----|-------|
| Power 5V | PCB J10.P1 | RPi GPIO pin 4 | +5V from XL4016-B |
| Power GND | PCB J10.P2 | RPi GPIO pin 6 | |
| UART TX | RPi GPIO14/BCM14 | PCB J11.P1 → ESP32 GPIO35 | RPi → ESP32 one-way |
| SPI screen | RPi 40-pin header | HAT direct | No PCB involved |

**3.5" SPI Screen (HAT on RPi):**

| Signal | BCM GPIO | Function |
|--------|----------|----------|
| BCM8 | SPI0 CE0 | Display CS |
| BCM9 | SPI0 MISO | |
| BCM10 | SPI0 MOSI | |
| BCM11 | SPI0 SCLK | |
| BCM25 | GPIO out | Display DC |
| BCM27 | GPIO out | Display RST |
| BCM7 | SPI0 CE1 | Touch CS |
| BCM17 | GPIO in | Touch IRQ |

---

## 8. JGA25-370 MOTORS WITH ENCODERS

Each motor has 6 wires:

| Wire colour | Connects to | Via |
|-------------|------------|-----|
| Motor power A | L298N OUT1 or OUT3 | Direct |
| Motor power B | L298N OUT2 or OUT4 | Direct |
| Red | +3V3 | PCB J7/J8 pin 1 |
| Black | GND | PCB J7/J8 pin 2 |
| Green | Encoder Ch A | PCB J7/J8 pin 3 → 1kΩ → 10nF filter → ESP32 |
| White | Encoder Ch B | PCB J7/J8 pin 4 → 1kΩ → 10nF filter → ESP32 |

Encoder GPIO summary:

| Motor | Ch A GPIO | Ch B GPIO |
|-------|-----------|-----------|
| Left (Encoder 1) | GPIO13 | GPIO14 |
| Right (Encoder 2) | GPIO16 | GPIO17 |

---

## 9. MG996R SERVOS

| Wire | Connects to |
|------|------------|
| Brown | GND |
| Red | +6V from PCA9685 V+ rail (Buck 1 output) |
| Orange | PWM from PCA9685 channel output |

---

## 10. QUICK-REFERENCE WIRING CHECKLIST

Before powering on:
- [ ] XL4016-A output set to **6.00V** (measure before connecting)
- [ ] XL4016-B output set to **5.00V** (measure before connecting)
- [ ] L298N ENA/ENB jumpers **removed** (controlled by PCB)
- [ ] L298N VSS wired directly to **11.1V** (not through PCB)
- [ ] GY-6500 CSB/NCS pin confirmed **LOW** (SPI mode active)
- [ ] ESP32 setup() drives all motor GPIOs **LOW** before L298N enable
- [ ] J6.P5 schematic fix confirmed before ordering PCB

---

*End of wiring document — Revision C*

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ── Pin map: verified Rev C distribution PCB (robot_pcb) ───────────
// ESP32 DevKit plugs into PCB sockets J12/J13. Do not change pins
// without changing the PCB.

// ── I2C (PCA9685 servo driver only) ─────────────────────────────────
#define PIN_SDA             21
#define PIN_SCL             22

// ── MPU-6500 IMU — GY-6500 module on PCB J5, SPI mode ───────────────
#define PIN_IMU_CS           5      // /SPI_CS_IMU (idle HIGH)
#define PIN_IMU_CLK         18      // /SPI_CLK
#define PIN_IMU_MISO        19      // /SPI_MISO
#define PIN_IMU_MOSI        23      // /SPI_MOSI
#define PIN_IMU_INT          4      // /IMU_INT (unused for now, polled @ 100 Hz)
#define IMU_SPI_HZ          1000000 // 1 MHz — safe for all MPU-6500 registers
#define IMU_WHO_AM_I_VAL    0x70    // MPU-6500
#define COMP_FILTER_ALPHA   0.98f   // 98 % gyro, 2 % accel
#define GYRO_LSB_PER_DPS    65.5f   // ±500 dps full scale
#define GYRO_CAL_SAMPLES    500     // bias calibration at boot (robot must be still)

// ── PCA9685 servo driver ────────────────────────────────────────────
#define PCA9685_ADDR        0x40
#define PCA9685_FREQ        50      // Hz — standard servo PWM

// Channel map per WIRING.md Rev C (CH0/1 = knees, CH2/3 = hips)
#define SERVO_CH_KNEE_L     0
#define SERVO_CH_KNEE_R     1
#define SERVO_CH_HIP_L      2
#define SERVO_CH_HIP_R      3

#define SERVO_MIN_US        500     // 0.5 ms pulse = 0°
#define SERVO_MAX_US        2500    // 2.5 ms pulse = 180°
// Hard clamp — MG996R clones jam/stall outside this window. Verify the
// real mechanical endpoints with the robot unloaded before widening.
#define SERVO_SAFE_MIN_US   600
#define SERVO_SAFE_MAX_US   2400

// ── Stance angles (degrees) ─────────────────────────────────────────
#define SIT_HIP_ANGLE       160
#define SIT_KNEE_ANGLE      20
#define STAND_HIP_ANGLE     90
#define STAND_KNEE_ANGLE    90

// ── Stand-up sequence ───────────────────────────────────────────────
#define STANDUP_STEPS       200
#define STANDUP_STEP_MS     10      // 200 × 10 ms = 2 s total
#define PID_ENABLE_STEP     100     // PID on at 50 % of sequence

// ── TB6612FNG motor driver (via PCB J9) ─────────────────────────────
// VM = 11.1 V direct, VCC + STBY tied to +3V3 on the module.
#define PIN_PWMA            25      // left motor PWM      (/H_ENA → PWMA)
#define PIN_AIN1            27      // left motor dir A    (/H_IN1 → AIN1)
#define PIN_AIN2            32      // left motor dir B    (/H_IN2 → AIN2)
#define PIN_PWMB            26      // right motor PWM     (/H_ENB → PWMB)
#define PIN_BIN1            33      // right motor dir A   (/H_IN3 → BIN1)
#define PIN_BIN2            15      // right motor dir B   (/H_IN4 → BIN2) ⚠ HIGH at boot

#define MOTOR_PWM_FREQ      1000    // Hz
#define MOTOR_PWM_RES       8       // bits → 0-255
#define MOTOR_MAX_PWM       255

// ── Wheel encoders (JGB37-520 hall quadrature, via PCB J7/J8) ───────
#define PIN_ENC1_A          13      // left  encoder Ch A
#define PIN_ENC1_B          14      // left  encoder Ch B
#define PIN_ENC2_A          16      // right encoder Ch A
#define PIN_ENC2_B          17      // right encoder Ch B

// ── UART from Raspberry Pi 5 (Serial2) ──────────────────────────────
// Rev C harness is ONE-WAY: Pi TX → PCB J11 → GPIO35 (input-only).
// No telemetry wire exists yet — set PIN_UART_TX to a free GPIO once
// an ESP32→Pi wire is added; -1 leaves TX unassigned.
#define PIN_UART_RX         35
#define PIN_UART_TX         (-1)
#define UART_BAUD           115200
#define TELEM_INTERVAL_MS   50      // 20 Hz telemetry (USB serial)

// ── PID defaults ────────────────────────────────────────────────────
#define DEFAULT_KP          25.0f
#define DEFAULT_KI           1.0f
#define DEFAULT_KD           1.5f
#define PID_OUTPUT_MIN      -255
#define PID_OUTPUT_MAX       255
#define BALANCE_ANGLE        0.0f   // target pitch (vertical)
#define D_FILTER_ALPHA       0.7f   // low-pass on derivative term

// ── Timing ──────────────────────────────────────────────────────────
#define LOOP_INTERVAL_US    10000   // 10 ms = 100 Hz

// ── Safety ──────────────────────────────────────────────────────────
#define FALL_ANGLE_THRESH   45.0f   // |pitch| > 45° → SIT

// ── Status LED ──────────────────────────────────────────────────────
#define PIN_LED              2

// ── Robot state ─────────────────────────────────────────────────────
enum RobotState {
    STATE_SIT,
    STATE_STANDING,
    STATE_BALANCE
};

#endif // CONFIG_H

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ── I2C (shared bus: MPU-6500 + PCA9685) ────────────────────────────
#define PIN_SDA             21
#define PIN_SCL             22

// ── MPU-6500 IMU ────────────────────────────────────────────────────
#define MPU_ADDR            0x68
#define COMP_FILTER_ALPHA   0.98f   // 98 % gyro, 2 % accel

// ── PCA9685 servo driver ────────────────────────────────────────────
#define PCA9685_ADDR        0x40
#define PCA9685_FREQ        50      // Hz — standard servo PWM

#define SERVO_CH_HIP_L      0
#define SERVO_CH_KNEE_L     1
#define SERVO_CH_HIP_R      2
#define SERVO_CH_KNEE_R     3

#define SERVO_MIN_US        500     // 0.5 ms pulse
#define SERVO_MAX_US        2500    // 2.5 ms pulse

// ── Stance angles (degrees) ─────────────────────────────────────────
#define SIT_HIP_ANGLE       160
#define SIT_KNEE_ANGLE      20
#define STAND_HIP_ANGLE     90
#define STAND_KNEE_ANGLE    90

// ── Stand-up sequence ───────────────────────────────────────────────
#define STANDUP_STEPS       200
#define STANDUP_STEP_MS     10      // 200 × 10 ms = 2 s total
#define PID_ENABLE_STEP     100     // PID on at 50 % of sequence

// ── TB6612FNG motor driver (PWMA/PWMB + AIN/BIN, STBY tied high) ────
#define PIN_ENA             25      // left motor PWM
#define PIN_IN1             26      // left motor dir A
#define PIN_IN2             27      // left motor dir B
#define PIN_ENB             32      // right motor PWM
#define PIN_IN3             33      // right motor dir A
#define PIN_IN4              4      // right motor dir B

#define MOTOR_PWM_FREQ      1000    // Hz
#define MOTOR_PWM_RES       8       // bits → 0-255
#define MOTOR_MAX_PWM       255

// ── UART to Raspberry Pi 5 (Serial2) ────────────────────────────────
#define PIN_UART_RX         16
#define PIN_UART_TX         17
#define UART_BAUD           115200
#define TELEM_INTERVAL_MS   50      // 20 Hz telemetry

// ── PID defaults ────────────────────────────────────────────────────
#define DEFAULT_KP          25.0f
#define DEFAULT_KI           1.0f
#define DEFAULT_KD           1.5f
#define PID_OUTPUT_MIN      -255
#define PID_OUTPUT_MAX       255
#define BALANCE_ANGLE        0.0f   // target pitch (vertical)

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

#include "config.h"
#include "sensors.h"
#include "servos.h"
#include "motors.h"
#include "comms.h"

RobotState    state      = STATE_SIT;
Command       cmd        = {};
float         kp         = DEFAULT_KP;
float         ki         = DEFAULT_KI;
float         kd         = DEFAULT_KD;
float         integral   = 0.0f;
float         prev_error = 0.0f;
int16_t       motor_l    = 0;
int16_t       motor_r    = 0;
unsigned long loop_timer = 0;
unsigned long led_timer  = 0;
bool          led_on     = false;

static const float dt = LOOP_INTERVAL_US * 1e-6f;

void setup() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    imu_init();
    servos_init();
    motors_init();
    comms_init();

    loop_timer = micros();

    Serial.println("SBR control v1.0 — ready");
    Serial.println("Commands: W=stand S=sit P=print +/-=Kp E=stop");
}

void loop() {
    if (micros() - loop_timer < LOOP_INTERVAL_US) return;
    loop_timer = micros();

    // ── Step 1: read IMU ────────────────────────────────────────────
    imu_read();
    float pitch = imu_pitch();

    // ── Step 2: parse commands ──────────────────────────────────────
    comms_parse(cmd);

    // ── Step 3: emergency stop (hard rule #5) ───────────────────────
    if (cmd.emergency) {
        motors_stop();
        servos_set_stance(SIT_HIP_ANGLE, SIT_KNEE_ANGLE);
        state      = STATE_SIT;
        integral   = 0.0f;
        prev_error = 0.0f;
        motor_l    = 0;
        motor_r    = 0;
        Serial.println("EMERGENCY STOP");
        comms_send_telemetry(pitch, motor_l, motor_r, state);
        return;
    }

    // ── Step 4: fall detection (hard rule #7) ───────────────────────
    if (state == STATE_BALANCE && fabsf(pitch) > FALL_ANGLE_THRESH) {
        motors_stop();
        servos_start_sit();
        state      = STATE_STANDING;
        integral   = 0.0f;
        prev_error = 0.0f;
        motor_l    = 0;
        motor_r    = 0;
        Serial.println("FALL DETECTED — sitting down");
    }

    // ── Step 5: state machine ───────────────────────────────────────
    switch (state) {

    case STATE_SIT:
        motor_l = 0;
        motor_r = 0;
        digitalWrite(PIN_LED, LOW);

        if (cmd.stand_request) {
            servos_start_stand();
            state = STATE_STANDING;
            Serial.println("Standing up...");
        }
        break;

    case STATE_STANDING:
        motor_l = 0;
        motor_r = 0;

        if (millis() - led_timer >= 250) {
            led_on = !led_on;
            digitalWrite(PIN_LED, led_on);
            led_timer = millis();
        }

        if (cmd.sit_request) {
            servos_start_sit();
        }

        {
            bool done = servos_interpolate_step();

            if (servos_get_dir() >= 0 && servos_get_step() >= PID_ENABLE_STEP) {
                integral   = 0.0f;
                prev_error = 0.0f;
                state = STATE_BALANCE;
                Serial.println("PID enabled — balancing");
            }

            if (servos_get_dir() < 0 && done) {
                state = STATE_SIT;
                Serial.println("Seated");
            }
        }
        break;

    case STATE_BALANCE:
        digitalWrite(PIN_LED, HIGH);

        if (cmd.sit_request) {
            motors_stop();
            servos_start_sit();
            state      = STATE_STANDING;
            integral   = 0.0f;
            prev_error = 0.0f;
            motor_l    = 0;
            motor_r    = 0;
            Serial.println("Sitting down...");
            break;
        }

        {
            float error = BALANCE_ANGLE - pitch;

            integral += error * dt;
            integral  = constrain(integral, -100.0f, 100.0f);

            float derivative = (error - prev_error) / dt;
            prev_error = error;

            float output = kp * error + ki * integral + kd * derivative;
            output = constrain(output, (float)PID_OUTPUT_MIN, (float)PID_OUTPUT_MAX);

            motor_l = (int16_t)output;
            motor_r = (int16_t)output;

            if (cmd.drive_active) {
                motor_l += cmd.drive_speed + cmd.drive_turn;
                motor_r += cmd.drive_speed - cmd.drive_turn;
            }

            motors_drive(motor_l, motor_r);
        }
        break;
    }

    // ── Step 6: gain adjustment ─────────────────────────────────────
    if (cmd.kp_adjust != 0.0f) {
        kp += cmd.kp_adjust;
        if (kp < 0.0f) kp = 0.0f;
        Serial.printf("Kp = %.1f\n", kp);
    }

    if (cmd.gain_set) {
        if (strcmp(cmd.gain_key, "KP") == 0) kp = cmd.gain_value;
        if (strcmp(cmd.gain_key, "KI") == 0) ki = cmd.gain_value;
        if (strcmp(cmd.gain_key, "KD") == 0) kd = cmd.gain_value;
        Serial.printf("Set %s = %.2f\n", cmd.gain_key, cmd.gain_value);
    }

    // ── Step 7: telemetry + debug ───────────────────────────────────
    comms_send_telemetry(pitch, motor_l, motor_r, state);

    if (cmd.print_request) {
        comms_print_debug(pitch, kp, ki, kd);
    }
}

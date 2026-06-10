#ifndef MOTORS_H
#define MOTORS_H

#include "config.h"

// TB6612FNG: INx pair sets direction, PWMx sets speed.
// Both INx LOW = coast. STBY is tied to +3V3 on the module, so the
// driver is live as soon as the rail is — motor pins must be LOW first.

static void motors_stop();

// Called as the very first thing in setup(): GPIO15 (BIN2) boots HIGH
// from its internal pull-up, which would pulse the right motor the
// moment VM comes up. Force every motor pin LOW before anything else.
static void motors_failsafe() {
    pinMode(PIN_AIN1, OUTPUT);  digitalWrite(PIN_AIN1, LOW);
    pinMode(PIN_AIN2, OUTPUT);  digitalWrite(PIN_AIN2, LOW);
    pinMode(PIN_BIN1, OUTPUT);  digitalWrite(PIN_BIN1, LOW);
    pinMode(PIN_BIN2, OUTPUT);  digitalWrite(PIN_BIN2, LOW);
    pinMode(PIN_PWMA, OUTPUT);  digitalWrite(PIN_PWMA, LOW);
    pinMode(PIN_PWMB, OUTPUT);  digitalWrite(PIN_PWMB, LOW);
}

static void _motor_set(uint8_t in_a, uint8_t in_b, uint8_t pwm_pin, int16_t val) {
    if (val > 0) {
        digitalWrite(in_a, HIGH);
        digitalWrite(in_b, LOW);
        ledcWrite(pwm_pin, val);
    } else if (val < 0) {
        digitalWrite(in_a, LOW);
        digitalWrite(in_b, HIGH);
        ledcWrite(pwm_pin, -val);
    } else {
        digitalWrite(in_a, LOW);
        digitalWrite(in_b, LOW);
        ledcWrite(pwm_pin, 0);
    }
}

static void motors_init() {
    ledcAttach(PIN_PWMA, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
    ledcAttach(PIN_PWMB, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
    motors_stop();
}

static void motors_drive(int16_t left, int16_t right) {
    left  = constrain(left,  -MOTOR_MAX_PWM, MOTOR_MAX_PWM);
    right = constrain(right, -MOTOR_MAX_PWM, MOTOR_MAX_PWM);

    _motor_set(PIN_AIN1, PIN_AIN2, PIN_PWMA, left);
    _motor_set(PIN_BIN1, PIN_BIN2, PIN_PWMB, right);
}

static void motors_balance(int16_t output) {
    motors_drive(output, output);
}

static void motors_stop() {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);
    digitalWrite(PIN_BIN1, LOW);
    digitalWrite(PIN_BIN2, LOW);
    ledcWrite(PIN_PWMA, 0);
    ledcWrite(PIN_PWMB, 0);
}

#endif // MOTORS_H

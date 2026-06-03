#ifndef MOTORS_H
#define MOTORS_H

#include "config.h"

static void motors_stop();

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
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);

    ledcAttach(PIN_ENA, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
    ledcAttach(PIN_ENB, MOTOR_PWM_FREQ, MOTOR_PWM_RES);

    motors_stop();
}

static void motors_drive(int16_t left, int16_t right) {
    left  = constrain(left,  -MOTOR_MAX_PWM, MOTOR_MAX_PWM);
    right = constrain(right, -MOTOR_MAX_PWM, MOTOR_MAX_PWM);

    _motor_set(PIN_IN1, PIN_IN2, PIN_ENA, left);
    _motor_set(PIN_IN3, PIN_IN4, PIN_ENB, right);
}

static void motors_balance(int16_t output) {
    motors_drive(output, output);
}

static void motors_stop() {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, LOW);
    ledcWrite(PIN_ENA, 0);
    ledcWrite(PIN_ENB, 0);
}

#endif // MOTORS_H

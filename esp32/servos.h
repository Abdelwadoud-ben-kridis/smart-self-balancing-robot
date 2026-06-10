#ifndef SERVOS_H
#define SERVOS_H

#include <Adafruit_PWMServoDriver.h>
#include "config.h"

static Adafruit_PWMServoDriver _pca(PCA9685_ADDR);
static uint16_t _interp_step = 0;
static int8_t   _interp_dir  = 0;

static void servo_write_angle(uint8_t channel, uint16_t angle) {
    uint16_t pulse_us = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
    // never command past the MG996R's safe window — a jammed servo
    // stalls at full current on the 6 V rail
    pulse_us = constrain(pulse_us, SERVO_SAFE_MIN_US, SERVO_SAFE_MAX_US);
    uint16_t tick = (uint16_t)((uint32_t)pulse_us * 4096 / 20000);
    _pca.setPWM(channel, 0, tick);
}

static void servos_set_stance(uint16_t hip_angle, uint16_t knee_angle) {
    servo_write_angle(SERVO_CH_HIP_L,  hip_angle);
    servo_write_angle(SERVO_CH_KNEE_L, knee_angle);
    servo_write_angle(SERVO_CH_HIP_R,  180 - hip_angle);
    servo_write_angle(SERVO_CH_KNEE_R, 180 - knee_angle);
}

static void servos_init() {
    _pca.begin();
    _pca.setPWMFreq(PCA9685_FREQ);
    delay(10);
    servos_set_stance(SIT_HIP_ANGLE, SIT_KNEE_ANGLE);
    _interp_step = 0;
    _interp_dir  = 0;
}

static void servos_start_stand() {
    _interp_step = 0;
    _interp_dir  = 1;
}

static void servos_start_sit() {
    if (_interp_step == 0) return;
    _interp_dir = -1;
}

static uint16_t servos_get_step() {
    return _interp_step;
}

static int8_t servos_get_dir() {
    return _interp_dir;
}

static bool servos_interpolate_step() {
    if (_interp_dir == 0) return true;

    float t = (1.0f - cosf(PI * (float)_interp_step / STANDUP_STEPS)) * 0.5f;

    uint16_t hip  = SIT_HIP_ANGLE  + (int16_t)((STAND_HIP_ANGLE  - SIT_HIP_ANGLE)  * t);
    uint16_t knee = SIT_KNEE_ANGLE + (int16_t)((STAND_KNEE_ANGLE - SIT_KNEE_ANGLE) * t);

    servos_set_stance(hip, knee);

    _interp_step += _interp_dir;

    if (_interp_dir > 0 && _interp_step > STANDUP_STEPS) {
        _interp_step = STANDUP_STEPS;
        _interp_dir  = 0;
        return true;
    }
    if (_interp_dir < 0 && _interp_step == 0) {
        _interp_dir = 0;
        return true;
    }

    return false;
}

#endif // SERVOS_H

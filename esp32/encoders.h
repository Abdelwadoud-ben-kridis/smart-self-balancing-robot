#ifndef ENCODERS_H
#define ENCODERS_H

#include "config.h"

// Quadrature decode (x2: count on every Ch A edge, direction from Ch B).
// JGB37-520 hall encoders, 3V3, RC-filtered on the PCB (fc ≈ 15.9 kHz).

static volatile int32_t _enc_left  = 0;
static volatile int32_t _enc_right = 0;

static void IRAM_ATTR _enc1_isr() {
    bool a = digitalRead(PIN_ENC1_A);
    bool b = digitalRead(PIN_ENC1_B);
    _enc_left += (a == b) ? 1 : -1;
}

static void IRAM_ATTR _enc2_isr() {
    bool a = digitalRead(PIN_ENC2_A);
    bool b = digitalRead(PIN_ENC2_B);
    _enc_right += (a == b) ? -1 : 1;   // mirrored mounting
}

static void encoders_init() {
    pinMode(PIN_ENC1_A, INPUT);
    pinMode(PIN_ENC1_B, INPUT);
    pinMode(PIN_ENC2_A, INPUT);
    pinMode(PIN_ENC2_B, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC1_A), _enc1_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC2_A), _enc2_isr, CHANGE);
}

static int32_t encoder_left() {
    noInterrupts();
    int32_t v = _enc_left;
    interrupts();
    return v;
}

static int32_t encoder_right() {
    noInterrupts();
    int32_t v = _enc_right;
    interrupts();
    return v;
}

static void encoders_reset() {
    noInterrupts();
    _enc_left  = 0;
    _enc_right = 0;
    interrupts();
}

#endif // ENCODERS_H

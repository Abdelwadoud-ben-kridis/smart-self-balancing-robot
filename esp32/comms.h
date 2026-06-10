#ifndef COMMS_H
#define COMMS_H

#include "config.h"

struct Command {
    bool     emergency;
    bool     stand_request;
    bool     sit_request;
    bool     print_request;
    int16_t  drive_speed;
    int16_t  drive_turn;
    bool     drive_active;
    float    kp_adjust;
    float    gain_value;
    char     gain_key[4];
    bool     gain_set;
};

static char    _rx_buf[64];
static uint8_t _rx_idx = 0;
static unsigned long _last_telem = 0;

static void _comms_parse_line(const char *buf, Command &cmd) {
    if (buf[0] == 'E' && buf[1] == ':') {
        cmd.emergency = true;
    } else if (buf[0] == 'M' && buf[1] == ':') {
        if (sscanf(buf + 2, "%hd,%hd", &cmd.drive_speed, &cmd.drive_turn) == 2)
            cmd.drive_active = true;
    } else if (strncmp(buf, "P:STAND", 7) == 0) {
        cmd.stand_request = true;
    } else if (strncmp(buf, "P:SIT", 5) == 0) {
        cmd.sit_request = true;
    } else if (buf[0] == 'G' && buf[1] == ':') {
        if (sscanf(buf + 2, "%3[^,],%f", cmd.gain_key, &cmd.gain_value) == 2)
            cmd.gain_set = true;
    }
}

static void comms_init() {
    Serial.begin(UART_BAUD);
    // Rev C harness: Pi TX → GPIO35 only. TX = -1 leaves the pin
    // unassigned until an ESP32→Pi telemetry wire is added.
    Serial2.begin(UART_BAUD, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);
    _rx_idx = 0;
    _last_telem = 0;
}

static void comms_parse(Command &cmd) {
    cmd.emergency     = false;
    cmd.stand_request = false;
    cmd.sit_request   = false;
    cmd.print_request = false;
    cmd.drive_active  = false;
    cmd.drive_speed   = 0;
    cmd.drive_turn    = 0;
    cmd.kp_adjust     = 0.0f;
    cmd.gain_set      = false;

    while (Serial.available()) {
        char c = Serial.read();
        switch (c) {
            case 'W': cmd.stand_request = true; break;
            case 'S': cmd.sit_request   = true; break;
            case 'P': cmd.print_request = true; break;
            case 'E': cmd.emergency     = true; break;
            case '+': cmd.kp_adjust     = 1.0f; break;
            case '-': cmd.kp_adjust     = -1.0f; break;
        }
    }

    while (Serial2.available()) {
        char c = Serial2.read();
        if (c == '\n' || c == '\r') {
            if (_rx_idx > 0) {
                _rx_buf[_rx_idx] = '\0';
                _comms_parse_line(_rx_buf, cmd);
                _rx_idx = 0;
            }
        } else {
            if (_rx_idx < sizeof(_rx_buf) - 1) {
                _rx_buf[_rx_idx++] = c;
            } else {
                _rx_idx = 0;
            }
        }
    }
}

static void comms_send_telemetry(float pitch, int16_t motor_l, int16_t motor_r, RobotState state) {
    unsigned long now = millis();
    if (now - _last_telem < TELEM_INTERVAL_MS) return;
    _last_telem = now;

    const char *s;
    switch (state) {
        case STATE_SIT:      s = "SIT"; break;
        case STATE_STANDING: s = "STD"; break;
        case STATE_BALANCE:  s = "BAL"; break;
        default:             s = "UNK"; break;
    }

#if (PIN_UART_TX >= 0)
    Serial2.printf("T:%.1f,%d,%d,%s\n", pitch, motor_l, motor_r, s);
#endif
    Serial.printf("T:%.1f,%d,%d,%s\n", pitch, motor_l, motor_r, s);
}

static void comms_print_debug(float pitch, float kp, float ki, float kd,
                              int32_t enc_l, int32_t enc_r, bool imu_ok) {
    Serial.printf("Pitch: %.2f  Kp: %.1f  Ki: %.1f  Kd: %.1f  EncL: %ld  EncR: %ld  IMU: %s\n",
                  pitch, kp, ki, kd, (long)enc_l, (long)enc_r, imu_ok ? "OK" : "FAULT");
}

#endif // COMMS_H

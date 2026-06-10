#ifndef SENSORS_H
#define SENSORS_H

#include <SPI.h>
#include "config.h"

// MPU-6500 registers
#define MPU_REG_SMPLRT_DIV   0x19
#define MPU_REG_CONFIG       0x1A
#define MPU_REG_GYRO_CONFIG  0x1B
#define MPU_REG_ACCEL_CONFIG 0x1C
#define MPU_REG_ACCEL_XOUT_H 0x3B
#define MPU_REG_USER_CTRL    0x6A
#define MPU_REG_PWR_MGMT_1   0x6B
#define MPU_REG_WHO_AM_I     0x75

static float    _pitch       = 0.0f;
static float    _gyro_bias_y = 0.0f;
static bool     _imu_ok      = false;
static unsigned long _last_us = 0;
static int16_t  _ax, _ay, _az;
static int16_t  _gx, _gy, _gz;

static const SPISettings _imu_spi(IMU_SPI_HZ, MSBFIRST, SPI_MODE3);

static void _imu_write(uint8_t reg, uint8_t val) {
    SPI.beginTransaction(_imu_spi);
    digitalWrite(PIN_IMU_CS, LOW);
    SPI.transfer(reg);
    SPI.transfer(val);
    digitalWrite(PIN_IMU_CS, HIGH);
    SPI.endTransaction();
}

static void _imu_read_bytes(uint8_t reg, uint8_t *buf, uint8_t len) {
    SPI.beginTransaction(_imu_spi);
    digitalWrite(PIN_IMU_CS, LOW);
    SPI.transfer(reg | 0x80);              // MSB set = read
    for (uint8_t i = 0; i < len; i++) buf[i] = SPI.transfer(0x00);
    digitalWrite(PIN_IMU_CS, HIGH);
    SPI.endTransaction();
}

static uint8_t _imu_read_reg(uint8_t reg) {
    uint8_t v;
    _imu_read_bytes(reg, &v, 1);
    return v;
}

static void imu_read() {
    uint8_t b[14];
    _imu_read_bytes(MPU_REG_ACCEL_XOUT_H, b, 14);
    _ax = (b[0]  << 8) | b[1];
    _ay = (b[2]  << 8) | b[3];
    _az = (b[4]  << 8) | b[5];
    // b[6..7] = temperature, skipped
    _gx = (b[8]  << 8) | b[9];
    _gy = (b[10] << 8) | b[11];
    _gz = (b[12] << 8) | b[13];
}

// Average the gyro at rest to remove constant bias. Robot is seated
// and motionless at boot, so this runs there (~0.5 s).
static void _imu_calibrate_gyro() {
    int32_t sum = 0;
    for (int i = 0; i < GYRO_CAL_SAMPLES; i++) {
        imu_read();
        sum += _gy;
        delay(1);
    }
    _gyro_bias_y = (float)sum / GYRO_CAL_SAMPLES;
}

static void imu_init() {
    pinMode(PIN_IMU_CS, OUTPUT);
    digitalWrite(PIN_IMU_CS, HIGH);
    SPI.begin(PIN_IMU_CLK, PIN_IMU_MISO, PIN_IMU_MOSI, PIN_IMU_CS);

    _imu_write(MPU_REG_PWR_MGMT_1, 0x80);  // device reset
    delay(100);
    _imu_write(MPU_REG_PWR_MGMT_1, 0x01);  // clock = PLL
    _imu_write(MPU_REG_USER_CTRL,  0x10);  // I2C_IF_DIS — SPI-only from here on
    _imu_write(MPU_REG_CONFIG,      0x03); // DLPF 41 Hz — keeps PWM noise out of D term
    _imu_write(MPU_REG_SMPLRT_DIV,  0x00); // 1 kHz internal rate
    _imu_write(MPU_REG_GYRO_CONFIG, 0x08); // ±500 dps
    _imu_write(MPU_REG_ACCEL_CONFIG,0x00); // ±2 g
    delay(100);

    _imu_ok = (_imu_read_reg(MPU_REG_WHO_AM_I) == IMU_WHO_AM_I_VAL);
    if (!_imu_ok) {
        Serial.println("IMU ERROR: WHO_AM_I mismatch — check J5 wiring / CS line");
    }

    _imu_calibrate_gyro();
    _last_us = micros();
}

static bool imu_ok() {
    return _imu_ok;
}

static float imu_pitch() {
    unsigned long now = micros();
    float dt = (now - _last_us) * 1e-6f;
    _last_us = now;

    float accel_pitch = atan2f((float)_ax, (float)_az) * 57.2957795f;
    float gyro_rate   = ((float)_gy - _gyro_bias_y) / GYRO_LSB_PER_DPS;

    _pitch = COMP_FILTER_ALPHA * (_pitch + gyro_rate * dt)
           + (1.0f - COMP_FILTER_ALPHA) * accel_pitch;

    return _pitch;
}

#endif // SENSORS_H

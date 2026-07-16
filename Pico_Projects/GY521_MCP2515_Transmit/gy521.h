#pragma once

#include <stdio.h>
#include <tuple>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// AD0 tied to 3.3V -> slave address is 0x69.
// (If you ever rewire AD0 to GND, this becomes 0x68 -- change only this line.)
#define MPU6050_ADDR      0x68

#define PWR_MGMT_1        0x6B
#define WHO_AM_I_REG      0x75
#define GYRO_CONFIG       0x1B
#define GYRO_XOUT_H       0x43

// This is the fixed silicon ID reported by WHO_AM_I. It does NOT change
// with AD0 -- AD0 only changes which bus address you must query at.
#define MPU6050_WHO_AM_I_VAL 0x68

// Returns true if init + WHO_AM_I check succeeded.
bool mpu_init()
{
    // Wake the device up (clear sleep bit in PWR_MGMT_1).
    uint8_t buf[2] = { PWR_MGMT_1, 0x00 };
    int w = i2c_write_blocking(i2c0, MPU6050_ADDR, buf, 2, false);
    if (w < 0) {
        printf("mpu_init: failed to write PWR_MGMT_1 (no ACK from 0x%02X)\n", MPU6050_ADDR);
        return false;
    }

    sleep_ms(50); // let it come out of reset/sleep cleanly

    // Optional: set gyro full-scale range to +-250 dps (default, matches /131.0f scaling below)
    uint8_t gcfg[2] = { GYRO_CONFIG, 0x00 };
    i2c_write_blocking(i2c0, MPU6050_ADDR, gcfg, 2, false);

    // WHO_AM_I check -- query at the real slave address, expect the fixed silicon value.
    uint8_t reg = WHO_AM_I_REG;
    uint8_t who = 0;
    int wr = i2c_write_blocking(i2c0, MPU6050_ADDR, &reg, 1, true);
    int rd = i2c_read_blocking(i2c0, MPU6050_ADDR, &who, 1, false);

    if (wr < 0 || rd < 0) {
        printf("mpu_init: WHO_AM_I read failed (wr=%d rd=%d)\n", wr, rd);
        return false;
    }

    printf("WHO_AM_I = 0x%02X (expected 0x%02X)\n", who, MPU6050_WHO_AM_I_VAL);

    if (who != MPU6050_WHO_AM_I_VAL) {
        printf("mpu_init: unexpected WHO_AM_I value -- wrong device or bad wiring?\n");
        return false;
    }

    return true;
}

// Returns {gx, gy, gz} in deg/s, and whether the read actually succeeded.
// On failure, values are 0.0f -- never uninitialized garbage.
std::tuple<float, float, float, bool> read_gyro()
{
    uint8_t reg = GYRO_XOUT_H;
    uint8_t data[6] = {0};

    int w = i2c_write_blocking(i2c0, MPU6050_ADDR, &reg, 1, true);
    int r = i2c_read_blocking(i2c0, MPU6050_ADDR, data, 6, false);

    if (w < 0 || r < 0) {
        printf("read_gyro: I2C error (w=%d r=%d)\n", w, r);
        return {0.0f, 0.0f, 0.0f, false};
    }

    int16_t gx = (int16_t)((data[0] << 8) | data[1]);
    int16_t gy = (int16_t)((data[2] << 8) | data[3]);
    int16_t gz = (int16_t)((data[4] << 8) | data[5]);

    // 131.0 LSB/(deg/s) is correct for the default +-250 dps range set above.
    float gx_dps = gx / 131.0f;
    float gy_dps = gy / 131.0f;
    float gz_dps = gz / 131.0f;

    return {gx_dps, gy_dps, gz_dps, true};
}
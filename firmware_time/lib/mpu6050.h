#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
} mpu6050_data;

void i2c_init(void);
bool mpu6050_init(void);               // Retorna true se ok
bool mpu6050_read(mpu6050_data *d);    // Retorna true se ok

#endif


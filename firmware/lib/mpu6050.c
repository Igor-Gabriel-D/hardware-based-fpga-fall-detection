#include "mpu6050.h"
#include <generated/csr.h>
#include <system.h>



static uint32_t i2c_w_reg = 0;

/* ---- I2C low-level control (IGUAL AO SEU CÓDIGO) ---- */

static void i2c_set_scl(int val) {
    if(val) i2c_w_reg |= (1 << CSR_I2C_W_SCL_OFFSET);
    else    i2c_w_reg &= ~(1 << CSR_I2C_W_SCL_OFFSET);
    i2c_w_write(i2c_w_reg);
}

static void i2c_set_sda(int val) {
    if(val) i2c_w_reg |= (1 << CSR_I2C_W_SDA_OFFSET);
    else    i2c_w_reg &= ~(1 << CSR_I2C_W_SDA_OFFSET);
    i2c_w_write(i2c_w_reg);
}

static void i2c_set_oe(int val) {
    if(val) i2c_w_reg |= (1 << CSR_I2C_W_OE_OFFSET);
    else    i2c_w_reg &= ~(1 << CSR_I2C_W_OE_OFFSET);
    i2c_w_write(i2c_w_reg);
}

void i2c_init(void) {
    i2c_set_oe(1);
    i2c_set_scl(1);
    i2c_set_sda(1);
    busy_wait_us(1000);
}

static void i2c_start(void) {
    i2c_set_sda(1);
    i2c_set_oe(1);
    i2c_set_scl(1);
    busy_wait_us(5);
    i2c_set_sda(0);
    busy_wait_us(5);
    i2c_set_scl(0);
}

static void i2c_stop(void) {
    i2c_set_sda(0);
    i2c_set_oe(1);
    i2c_set_scl(1);
    busy_wait_us(5);
    i2c_set_sda(1);
    busy_wait_us(5);
}

static bool i2c_write_byte(uint8_t byte) {
    for(int i=0;i<8;i++) {
        i2c_set_sda((byte & 0x80) != 0);
        busy_wait_us(5);
        i2c_set_scl(1);
        busy_wait_us(5);
        i2c_set_scl(0);
        byte <<= 1;
    }
    i2c_set_oe(0);
    i2c_set_sda(1);
    busy_wait_us(5);
    i2c_set_scl(1);
    busy_wait_us(5);
    bool ack = !(i2c_r_read() & (1 << CSR_I2C_R_SDA_OFFSET));
    i2c_set_scl(0);
    i2c_set_oe(1);
    return ack;
}

static uint8_t i2c_read_byte(bool ack) {
    uint8_t byte = 0;
    i2c_set_oe(0);
    i2c_set_sda(1);
    for(int i=0;i<8;i++) {
        byte <<= 1;
        i2c_set_scl(1);
        busy_wait_us(5);
        if(i2c_r_read() & (1 << CSR_I2C_R_SDA_OFFSET)) byte |= 1;
        i2c_set_scl(0);
        busy_wait_us(5);
    }
    i2c_set_oe(1);
    i2c_set_sda(!ack);
    i2c_set_scl(1);
    busy_wait_us(5);
    i2c_set_scl(0);
    return byte;
}

/*typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} mpu6050_data;
*/

#define MPU6050_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_XOUT_H 0x3B
#define REG_GYRO_XOUT_H  0x43

bool mpu6050_init(void) {
    i2c_start();
    if(!i2c_write_byte(MPU6050_ADDR << 1 | 0)) { i2c_stop(); return false; }
    if(!i2c_write_byte(REG_PWR_MGMT_1)) { i2c_stop(); return false; }
    if(!i2c_write_byte(0x00)) { i2c_stop(); return false; }  // wake up
    i2c_stop();
    busy_wait_us(100000); // espera 100ms
    return true;
}

static int16_t mpu6050_read_word(uint8_t reg) {
    uint8_t high, low;
    i2c_start();
    i2c_write_byte(MPU6050_ADDR << 1 | 0);
    i2c_write_byte(reg);
    i2c_stop();

    i2c_start();
    i2c_write_byte(MPU6050_ADDR << 1 | 1);
    high = i2c_read_byte(true);
    low  = i2c_read_byte(false);
    i2c_stop();
    return (int16_t)((high << 8) | low);
}

bool mpu6050_read(mpu6050_data *d) {
    if(!d) return false;
    d->ax = mpu6050_read_word(REG_ACCEL_XOUT_H);
    d->ay = mpu6050_read_word(REG_ACCEL_XOUT_H + 2);
    d->az = mpu6050_read_word(REG_ACCEL_XOUT_H + 4);

    d->gx = mpu6050_read_word(REG_GYRO_XOUT_H);
    d->gy = mpu6050_read_word(REG_GYRO_XOUT_H + 2);
    d->gz = mpu6050_read_word(REG_GYRO_XOUT_H + 4);
    return true;
}

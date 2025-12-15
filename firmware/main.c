#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include <irq.h>
#include <uart.h>
#include <generated/csr.h>
#include <generated/soc.h>
#include <system.h>

#include "lib/mpu6050.h"
#include "lib/rfm96.h"

// thresholds em "raw" do MPU6050
#define IMPACT_THRESHOLD   40960   // ~2.5g
#define STILL_THRESHOLD     6500   // ~0.4g
#define STILL_TIME_MS        300   // tempo parado após impacto

static uint32_t impact_time = 0;

bool fall_detect(mpu6050_data *d) {
    float ax = d->ax;
    float ay = d->ay;
    float az = d->az;

    uint32_t mag = (uint32_t)sqrtf(ax*ax + ay*ay + az*az);

    if (mag > IMPACT_THRESHOLD && impact_time == 0) {
        impact_time = timer0_value_read();
        printf("Impacto detectado!");
    }

    if (impact_time != 0) {
        uint32_t now = timer0_value_read();

        if (mag < STILL_THRESHOLD) {
            if ((impact_time - now) > STILL_TIME_MS * 1000) {
                impact_time = 0;
                return true;
            }
        } else {
            impact_time = 0;
        }
    }

    return false;
}

int main(void) {
#ifdef CONFIG_CPU_HAS_INTERRUPT
    irq_setmask(0);
    irq_setie(1);
#endif

    uart_init();

#ifdef CSR_TIMER0_BASE
    timer0_en_write(0);
    timer0_reload_write(0);
    timer0_load_write(CONFIG_CLOCK_FREQUENCY / 1000000);
    timer0_en_write(1);
    timer0_update_value_write(1);
#endif

    busy_wait_us(300000);

    printf("==============================");
    printf("  MPU6050 + LoRa | Queda");
    printf("==============================");

    i2c_init();
    if (!mpu6050_init()) {
        printf("Falha ao inicializar MPU6050");
        while(1);
    }

    if (!rfm96_init()) {
        printf("Falha ao inicializar LoRa");
        while(1);
    }

    printf("Sistema iniciado. Monitorando...");

    while (1) {
        mpu6050_data d;

        if (!mpu6050_read(&d)) {
            printf("Erro de leitura MPU6050");
            continue;
        }

        if (fall_detect(&d)) {
            printf(">>> QUEDA DETECTADA! <<<");

            const char *msg = "FALL";
            rfm96_send((uint8_t*)msg, 4);
        }

        busy_wait_us(20000); // 20 ms
    }

    return 0;
}


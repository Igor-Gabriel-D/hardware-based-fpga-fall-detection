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
//#define IMPACT_THRESHOLD   40960   // ~2.5g
//#define STILL_THRESHOLD     6500   // ~0.4g
//#define STILL_TIME_MS        300   // tempo parado após impacto

#define IMPACT_THRESHOLD      40960
#define STILL_THRESHOLD        6500

#define IMPACT_THRESHOLD_SQ  ((uint32_t)(IMPACT_THRESHOLD) * (IMPACT_THRESHOLD))
#define STILL_THRESHOLD_SQ   ((uint32_t)(STILL_THRESHOLD)  * (STILL_THRESHOLD))

#define STILL_TIME_MS         300

static uint32_t impact_time = 0;

static uint32_t get_timer_value(void) {
    timer0_update_value_write(1);
    return timer0_value_read();
}

static void start_timer(void) {
    timer0_en_write(0);
    timer0_load_write(0xffffffff);
    timer0_reload_write(0);
    timer0_en_write(1);
}

bool fall_detect(mpu6050_data *d) {

    uint32_t start, end, duration;   
    start_timer();
    start = get_timer_value();

    float ax = d->ax;
    float ay = d->ay;
    float az = d->az;

    uint32_t mag = (uint32_t)sqrtf(ax*ax + ay*ay + az*az);
    end = get_timer_value();
    duration = start - end;
    printf("[HW] Ciclos de processamento: %lu\n", (long unsigned int)duration);
    if (mag > IMPACT_THRESHOLD && impact_time == 0) {
        impact_time = timer0_value_read();
        printf("Impacto detectado!\n");
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
	printf("Teste timer...\n");

	uint32_t a, b;

	timer0_update_value_write(1);
	a = timer0_value_read();

	busy_wait_us(100000); // 100 ms

	timer0_update_value_write(1);
	b = timer0_value_read();

	printf("Timer diff = %lu\n", a - b);
    while (1) {
        mpu6050_data d;

        if (!mpu6050_read(&d)) {
            printf("Erro de leitura MPU6050");
            continue;
        }
   	//printf("ANTES SEND\n");
	//rfm96_send((uint8_t*)"FALL", 4);
	//printf("DEPOIS SEND\n");
        if (fall_detect(&d)) {
            printf(">>> QUEDA DETECTADA! <<<\n");

   
        }
        busy_wait_us(20000); // 20 ms
    }

    return 0;
}

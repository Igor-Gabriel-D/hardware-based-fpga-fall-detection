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

#define FREEFALL_THRESHOLD        3000     // ~0.15g
#define FREEFALL_THRESHOLD_SQ     (FREEFALL_THRESHOLD * FREEFALL_THRESHOLD)


static uint32_t impact_time = 0;

static inline uint64_t rdcycle64(void) {
    uint32_t lo, hi;
    asm volatile ("rdcycle %0"  : "=r"(lo));
    asm volatile ("rdcycleh %0" : "=r"(hi));
    return ((uint64_t)hi << 32) | lo;
}

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

typedef enum {
    STATE_IDLE = 0,
    STATE_FREEFALL,
    STATE_IMPACT
} fall_state_t;

static fall_state_t fall_state = STATE_IDLE;
static uint32_t freefall_time = 0;
//static uint32_t impact_time = 0;

#define LOOP_PERIOD_MS      20
#define STILL_TIME_MS       300

#define STILL_COUNT   (STILL_TIME_MS / LOOP_PERIOD_MS)
#define FREEFALL_COUNT_MIN  2    // 2 * 20ms = 40ms

static uint32_t impact_cnt = 0;
static uint32_t still_cnt = 0;
static uint32_t freefall_cnt = 0;
static bool freefall_armed = false;
static bool freefall_reported = false;


bool fall_detect(mpu6050_data *d)
{
    int32_t ax = d->ax;
    int32_t ay = d->ay;
    int32_t az = d->az;

    uint32_t mag_sq;
    uint32_t start, end, duration;

    start_timer();
    start = get_timer_value();
    /* acelerador */
    fall_detect_ax_write(ax);
    fall_detect_ay_write(ay);
    fall_detect_az_write(az);

    fall_detect_data_valid_write(1);
    fall_detect_data_valid_write(0);

    mag_sq = fall_detect_mag_sq_read();
    end = get_timer_value();
    duration = start - end;
    //printf("[HW] Ciclos de processamento: %lu\n", (long unsigned int)duration);
    /* ===============================
       1. QUEDA LIVRE
       =============================== */
    if (mag_sq < FREEFALL_THRESHOLD_SQ) {
        if (freefall_cnt < 255)
            freefall_cnt++;

        if (freefall_cnt >= FREEFALL_COUNT_MIN){
	    freefall_armed = true;
	    if (!freefall_reported) {
                freefall_reported = true;
                printf(">>> FREE FALL <<<\n");
                //rfm96_send((uint8_t*)"FREE", 4);
            }
	}
            //freefall_armed = true;
    } else {
        freefall_cnt = 0;
    }

    /* ===============================
       2. IMPACTO (INALTERADO)
       =============================== */
    if (mag_sq > IMPACT_THRESHOLD_SQ && impact_cnt == 0) {
        impact_cnt = 1;
        still_cnt = 0;
        freefall_reported = false;
        
	if (freefall_armed) {
            printf("IMPACT AFTER FREE FALL\n");
            rfm96_send((uint8_t*)"FALL", 4);
        } else {
            printf("IMPACT ONLY\n");
        }
    }

    /* ===============================
       3. IMOBILIDADE
       =============================== */
    if (impact_cnt) {
        if (mag_sq < STILL_THRESHOLD_SQ) {
            still_cnt++;
            if (still_cnt >= STILL_COUNT) {
                impact_cnt = 0;
                still_cnt = 0;
                freefall_armed = false;
                return true;
            }
        } else {
            /* movimento cancela */
            impact_cnt = 0;
            still_cnt = 0;
            freefall_armed = false;
	    freefall_reported = false;
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


    while (1) {
        mpu6050_data d;

        if (!mpu6050_read(&d)) {
            printf("Erro de leitura MPU6050");
            continue;
        }

        if (fall_detect(&d)) {
            printf(">>> QUEDA DETECTADA! <<<");

   
        }
        busy_wait_us(20000); // 20 ms
    }

    return 0;
}

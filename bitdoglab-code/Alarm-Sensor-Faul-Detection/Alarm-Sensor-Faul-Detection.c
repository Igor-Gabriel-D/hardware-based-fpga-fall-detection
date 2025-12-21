#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "rfm96.h"


#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  20
#define PIN_DIO0 8   

#define LORA_FREQUENCY 915E6
#define SEND_INTERVAL_MS 10000

#define SDA_PIN 14
#define SCL_PIN 15
#define I2C_PORT i2c1
#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64



struct repeating_timer timer;
int cont = 0;
bool start = true;






// ----------------------------------------------------------

// Callback para timer repetitivo mostrando "Aguardando..."

// ----------------------------------------------------------



// ----------------------------------------------------------

void iniciar() {
    stdio_init_all();


    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);





    rfm96_config_t lora_cfg = {
        .spi_instance = spi0,
        .pin_miso = PIN_MISO,
        .pin_cs   = PIN_CS,
        .pin_sck  = PIN_SCK,
        .pin_mosi = PIN_MOSI,
        .pin_rst  = PIN_RST,
        .pin_dio0 = PIN_DIO0,
        .frequency = LORA_FREQUENCY
    };

    if (!lora_init(lora_cfg)) {
        while (1);
    }


    sleep_ms(1000);

    // Limpa display e inicia recepção LoRa contínua

    lora_start_rx_continuous();
}


// ----------------------------------------------------------



// ----------------------------------------------------------

int main() {
    int recebido;

    iniciar();


    while (true) {
        int len = lora_receive_bytes((uint8_t *)&recebido, sizeof(recebido));
        if (len == sizeof(recebido)) {
            if (start) {
                cancel_repeating_timer(&timer);

                start = false;
            }
            printf("Valor recebido: %d\n", recebido);
        }

        //printf("Aguardando dados...\n");
    }

    return 0;
}
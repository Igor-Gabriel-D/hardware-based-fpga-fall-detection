#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/spi.h"

// --- Bibliotecas de Terceiros ---
// Nota: Você precisará incluir a biblioteca LoRa no seu projeto CMake
#include "lora.h" 

#define BUZZER_PIN 21
#define FREQ_QUEDA 1000   
#define FREQ_IMPACTO 2800  

// Configuração SPI LoRa
#define LORA_SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  20
#define PIN_DIO0 26

int modo_atual = 0; 
absolute_time_t last_heartbeat;
absolute_time_t proximo_evento;
bool buzzer_ligado = false;

void pwm_init_buzzer(uint pin, uint freq) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (freq * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

void processar_comando(const char* comando) {
    if (strstr(comando, "queda")) {
        modo_atual = 1;
        pwm_init_buzzer(BUZZER_PIN, FREQ_QUEDA);
        proximo_evento = get_absolute_time();
        printf(">> LORA: MODO QUEDA ATIVADO\n");
    } 
    else if (strstr(comando, "impacto")) {
        modo_atual = 2;
        pwm_init_buzzer(BUZZER_PIN, FREQ_IMPACTO);
        printf(">> LORA: MODO IMPACTO ATIVADO\n");
    } 
    else if (strstr(comando, "parar")) {
        modo_atual = 0;
        pwm_set_gpio_level(BUZZER_PIN, 0);
        printf(">> LORA: SILENCIAR\n");
    }
}

int main() {
    stdio_init_all();
    pwm_init_buzzer(BUZZER_PIN, FREQ_QUEDA);

    // --- Inicialização LoRa ---
    lora_struct_t lora;
    lora.spi = LORA_SPI_PORT;
    lora.rst = PIN_RST;
    lora.cs = PIN_CS;
    lora.dio0 = PIN_DIO0;
    lora.frequency = 915E6; // Frequência Brasil

    if (lora_init(&lora) != 0) {
        printf("Erro ao iniciar LoRa!\n");
    } else {
        printf("LoRa Online e Escutando...\n");
    }

    last_heartbeat = get_absolute_time();
    proximo_evento = get_absolute_time();

    char buffer_serial[40];
    int idx_serial = 0;

    while (true) {
        // 1. Heartbeat
        if (absolute_time_diff_us(last_heartbeat, get_absolute_time()) > 5000000) {
            printf("[STATUS] Aguardando sinal LoRa... Modo atual: %d\n", modo_atual);
            last_heartbeat = get_absolute_time();
        }

        // 2. RECEBIMENTO VIA LORA
        if (lora_received(&lora)) {
            char lora_msg[64];
            int len = lora_receive_packet(&lora, (uint8_t*)lora_msg, sizeof(lora_msg)-1);
            if (len > 0) {
                lora_msg[len] = '\0';
                printf("Recebido via LoRa: %s\n", lora_msg);
                processar_comando(lora_msg);
            }
        }

        // 3. BACKUP VIA SERIAL (Ainda útil para testes)
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                if (idx_serial > 0) {
                    buffer_serial[idx_serial] = '\0';
                    processar_comando(buffer_serial);
                    idx_serial = 0;
                }
            } else if (idx_serial < 39 && c >= 32) {
                buffer_serial[idx_serial++] = (char)c;
            }
        }

        // 4. Lógica de Som Não-Bloqueante
        absolute_time_t agora = get_absolute_time();
        if (modo_atual == 1) {
            if (absolute_time_diff_us(proximo_evento, agora) > 0) {
                buzzer_ligado = !buzzer_ligado;
                pwm_set_gpio_level(BUZZER_PIN, buzzer_ligado ? 2048 : 0);
                proximo_evento = delayed_by_ms(agora, 500);
            }
        } 
        else if (modo_atual == 2) {
            pwm_set_gpio_level(BUZZER_PIN, 2048);
        } 
        else {
            pwm_set_gpio_level(BUZZER_PIN, 0);
        }

        sleep_us(100);
    }
}
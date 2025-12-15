#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <irq.h>
#include <uart.h>
#include <console.h>
#include <generated/csr.h>
#include <generated/soc.h>
#include <system.h>

#include "lib/ssd1306.h"

/* -------------------------------------------------------------------------- */
/* Console helpers                                                             */
/* -------------------------------------------------------------------------- */

static char *readstr(void)
{
    char c[2];
    static char s[64];
    static int ptr = 0;

    if (readchar_nonblock()) {
        c[0] = readchar();
        c[1] = 0;

        switch (c[0]) {
        case 0x7f:
        case 0x08:
            if (ptr > 0) {
                ptr--;
                putsnonl("\x08 \x08");
            }
            break;

        case 0x07:
            break;

        case '\r':
        case '\n':
            s[ptr] = 0x00;
            putsnonl("\n");
            ptr = 0;
            return s;

        default:
            if (ptr >= (sizeof(s) - 1))
                break;
            putsnonl(c);
            s[ptr++] = c[0];
            break;
        }
    }
    return NULL;
}

static char *get_token(char **str)
{
    char *c, *d;

    c = strchr(*str, ' ');
    if (c == NULL) {
        d = *str;
        *str = *str + strlen(*str);
        return d;
    }

    *c = 0;
    d = *str;
    *str = c + 1;
    return d;
}

static void prompt(void)
{
    printf("RUNTIME>");
}

static void reboot(void)
{
    ctrl_reset_write(1);
}

static void toggle_led(void)
{
    int v = leds_out_read();
    leds_out_write(!v);
    printf("LED invertido\n");
}

/* -------------------------------------------------------------------------- */
/* Teste SSD1306                                                               */
/* -------------------------------------------------------------------------- */

static void test_display(void)
{
    ssd1306_clear();

    // Desenha algumas linhas horizontais e verticais
    /*for (int x = 0; x < SSD1306_WIDTH; x += 4)
        for (int y = 0; y < SSD1306_HEIGHT; y += 8)
            ssd1306_draw_pixel(x, y, true);
*/

    // Desenha uma string de teste
    ssd1306_draw_string(25, 25, "Hello FPGA! sejfnseijn", true);
    // Atualiza o display
    ssd1306_update();

    printf("Desenho de teste enviado para o display\n");
}

/* -------------------------------------------------------------------------- */
/* Console commands                                                            */
/* -------------------------------------------------------------------------- */

static void help(void)
{
    puts("Available commands:");
    puts("help       - this command");
    puts("reboot     - reboot CPU");
    puts("led        - toggle led");
    puts("draw      - draw test pattern on SSD1306");
}

static void console_service(void)
{
    char *str;
    char *token;

    str = readstr();
    if (!str)
        return;

    token = get_token(&str);

    if (!strcmp(token, "help"))
        help();
    else if (!strcmp(token, "reboot"))
        reboot();
    else if (!strcmp(token, "led"))
        toggle_led();
    else if (!strcmp(token, "draw"))
        test_display();
    else
        printf("Comando desconhecido\n");

    prompt();
}

/* -------------------------------------------------------------------------- */
/* MAIN                                                                        */
/* -------------------------------------------------------------------------- */

int main(void)
{
#ifdef CONFIG_CPU_HAS_INTERRUPT
    irq_setmask(0);
    irq_setie(1);
#endif

    uart_init();
    i2c_init();

#ifdef CSR_TIMER0_BASE
    timer0_en_write(0);
    timer0_reload_write(0);
    timer0_load_write(CONFIG_CLOCK_FREQUENCY / 1000000);
    timer0_en_write(1);
    timer0_update_value_write(1);
#endif

    busy_wait_us(500000);

    printf("\n===============================================\n");
    printf("       SSD1306 Test (FPGA runtime)\n");
    printf("===============================================\n");

    if (!ssd1306_init()) {
        printf("Falha ao inicializar SSD1306\n");
        while (1);
    }

    help();
    prompt();

    while (1)
        console_service();

    return 0;
}


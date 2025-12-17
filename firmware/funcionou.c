#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <irq.h>
#include <uart.h>
#include <console.h>
#include <generated/csr.h>

/* ========= Infra básica de console (IGUAL AO SEU) ========= */

static char *readstr(void)
{
    char c[2];
    static char s[64];
    static int ptr = 0;

    if(readchar_nonblock()) {
        c[0] = readchar();
        c[1] = 0;
        switch(c[0]) {
            case 0x7f:
            case 0x08:
                if(ptr > 0) {
                    ptr--;
                    putsnonl("\x08 \x08");
                }
                break;
            case '\r':
            case '\n':
                s[ptr] = 0x00;
                putsnonl("\n");
                ptr = 0;
                return s;
            default:
                if(ptr >= (sizeof(s) - 1))
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
    if(c == NULL) {
        d = *str;
        *str += strlen(*str);
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

static void help(void)
{
    puts("Available commands:");
    puts("help   - this command");
    puts("reboot - reboot CPU");
    puts("led    - led test");
    puts("mag    - test magnitude squared (AX^2+AY^2+AZ^2)");
}

static void reboot(void)
{
    ctrl_reset_write(1);
}

static void toggle_led(void)
{
    int i;
    printf("invertendo led...\n");
    i = leds_out_read();
    leds_out_write(!i);
}

/* ========= TESTE DO FALL_DETECT_MAG_SQ ========= */

static void test_mag_sq(void)
{
    char *input;
    int32_t ax, ay, az;
    uint32_t mag_sq;

    printf("\n--- Teste MAG_SQ ---\n");

    printf("AX: ");
    while ((input = readstr()) == NULL);
    ax = strtol(input, NULL, 10);

    printf("AY: ");
    while ((input = readstr()) == NULL);
    ay = strtol(input, NULL, 10);

    printf("AZ: ");
    while ((input = readstr()) == NULL);
    az = strtol(input, NULL, 10);

    /* escreve entradas */
    fall_detect_ax_write(ax);
    fall_detect_ay_write(ay);
    fall_detect_az_write(az);

    /* pulso de data_valid (IGUAL start do dot) */
    fall_detect_data_valid_write(1);
    fall_detect_data_valid_write(0);

    /* leitura direta (sem esperar nada) */
    mag_sq = fall_detect_mag_sq_read();

    printf("\n==== Resultado ====\n");
    printf("AX = %ld\n", (long)ax);
    printf("AY = %ld\n", (long)ay);
    printf("AZ = %ld\n", (long)az);
    printf("MAG_SQ = %lu\n", (unsigned long)mag_sq);
}

/* ========= CONSOLE ========= */

static void console_service(void)
{
    char *str, *token;
    str = readstr();
    if(str == NULL) return;

    token = get_token(&str);
    if(strcmp(token, "help") == 0)
        help();
    else if(strcmp(token, "reboot") == 0)
        reboot();
    else if(strcmp(token, "led") == 0)
        toggle_led();
    else if(strcmp(token, "mag") == 0)
        test_mag_sq();
    else
        puts("Comando desconhecido. Digite 'help'.");

    prompt();
}

/* ========= MAIN ========= */

int main(void)
{
#ifdef CONFIG_CPU_HAS_INTERRUPT
    irq_setmask(0);
    irq_setie(1);
#endif
    uart_init();

    printf("--============= Liftoff! ===============--\n");
    help();
    prompt();

    while(1)
        console_service();

    return 0;
}


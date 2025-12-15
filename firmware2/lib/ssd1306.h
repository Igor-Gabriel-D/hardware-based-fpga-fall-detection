#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

/* ---- Configurações do display ---- */
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

/* ---- Inicialização e controle ---- */
bool ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_update(void);

/* ---- Desenho de pixels ---- */
void ssd1306_draw_pixel(int x, int y, bool color);

/* ---- Desenho de caracteres e strings ---- */
void ssd1306_draw_char(int x, int y, char c, bool color);
void ssd1306_draw_string(int x, int y, const char* str, bool color);

/* ---- I2C (exposta para main.c) ---- */
void i2c_init(void);

#endif


#ifndef RFM95_H_
#define RFM95_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> 

#define TX_TIMEOUT_MS 5000

#define SPI_MODE_MANUAL (1 << 16)
#define SPI_CS_MASK     0x0001    

#define REG_FIFO                 0x00
#define REG_OP_MODE              0x01
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08
#define REG_PA_CONFIG            0x09
#define REG_LNA                  0x0C
#define REG_FIFO_ADDR_PTR        0x0D
#define REG_FIFO_TX_BASE_ADDR    0x0E
#define REG_FIFO_RX_BASE_ADDR    0x0F
#define REG_IRQ_FLAGS_MASK       0x11
#define REG_IRQ_FLAGS            0x12
#define REG_MODEM_CONFIG_1       0x1D
#define REG_MODEM_CONFIG_2       0x1E
#define REG_PREAMBLE_MSB         0x20
#define REG_PREAMBLE_LSB         0x21
#define REG_PAYLOAD_LENGTH       0x22
#define REG_MODEM_CONFIG_3       0x26
#define REG_SYNC_WORD            0x39
#define REG_DIO_MAPPING_1        0x40
#define REG_VERSION              0x42
#define REG_PA_DAC               0x4D
#define REG_OCP                  0x0B

#define MODE_SLEEP               0x00
#define MODE_STDBY               0x01
#define MODE_TX                  0x03
#define MODE_RX_CONTINUOUS       0x05 

#define IRQ_TX_DONE_MASK         0x08


bool rfm96_init(void);
bool rfm96_send(const uint8_t *data, size_t len);
void rfm96_set_mode(uint8_t mode);
uint8_t rfm96_read_reg(uint8_t reg);
void rfm96_write_reg(uint8_t reg, uint8_t value);


#endif

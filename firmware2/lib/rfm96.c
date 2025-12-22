#include <stdio.h> 
#include <string.h> 
#include <generated/csr.h> 
#include <system.h>
#include "rfm96.h"

static inline void spi_select(void) {
    spi_cs_write(SPI_MODE_MANUAL | SPI_CS_MASK);
    busy_wait_us(2); 
}

static inline void spi_deselect(void) {
    spi_cs_write(SPI_MODE_MANUAL | 0x0000);
    busy_wait_us(2); 
}

static inline uint8_t spi_txrx(uint8_t tx_byte) {
    uint32_t rx_byte;

    spi_mosi_write((uint32_t)tx_byte);
    spi_control_write((1 << CSR_SPI_CONTROL_START_OFFSET) | (8 << CSR_SPI_CONTROL_LENGTH_OFFSET));
    
    while( (spi_status_read() & (1 << CSR_SPI_STATUS_DONE_OFFSET)) == 0 ) {}
    rx_byte = spi_miso_read();
    return (uint8_t)(rx_byte & 0xFF);
}

// ===============================================================
static void rfm96_write_fifo(const uint8_t *data, uint8_t len) {
    spi_select();
    
    spi_txrx(REG_FIFO | 0x80); 
    for (uint8_t i = 0; i < len; i++) {
        spi_txrx(data[i]);
    }
    spi_deselect();
}

uint8_t rfm96_read_reg(uint8_t reg) {
    uint8_t val;
    spi_select();
    spi_txrx(reg & 0x7F); 
    val = spi_txrx(0x00); 
    spi_deselect();
    return val;
}

void rfm96_write_reg(uint8_t reg, uint8_t value) {
    spi_select();
    spi_txrx(reg | 0x80);
    spi_txrx(value);
    spi_deselect();
}

bool rfm96_init(void) {
    uint8_t rx;
    
    // inicializa o SPI
    spi_cs_write(SPI_MODE_MANUAL | 0x0000);
    spi_loopback_write(0);
    busy_wait_us(1000);
    
    lora_reset_out_write(0); 
    busy_wait_us(5000);
    lora_reset_out_write(1); 
    busy_wait_us(10000);
    
    rx = rfm96_read_reg(REG_VERSION);
    if (rx != 0x12) {
        printf("Versão do módulo não correspondente. Falha na comunicação.\n");
        return false; 
    }

    
    rfm96_write_reg(REG_OP_MODE, (0x80 | MODE_SLEEP));

    uint64_t frf = ((uint64_t)915000000 << 19) / 32000000; 
    rfm96_write_reg(REG_FRF_MSB, (uint8_t)(frf >> 16));
    rfm96_write_reg(REG_FRF_MID, (uint8_t)(frf >> 8));
    rfm96_write_reg(REG_FRF_LSB, (uint8_t)(frf >> 0));
    rfm96_write_reg(REG_PA_CONFIG, 0xFF); 
    rfm96_write_reg(REG_PA_DAC, 0x87);    
    rfm96_write_reg(REG_MODEM_CONFIG_1, 0x78); 
    rfm96_write_reg(REG_MODEM_CONFIG_2, 0xC4); 
    rfm96_write_reg(REG_MODEM_CONFIG_3, 0x0C); 
    rfm96_write_reg(REG_PREAMBLE_MSB, 0x00);
    rfm96_write_reg(REG_PREAMBLE_LSB, 0x0C); 
    rfm96_write_reg(REG_SYNC_WORD, 0x12);  
    rfm96_write_reg(REG_OCP, 0x37);
    rfm96_write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    rfm96_write_reg(REG_FIFO_RX_BASE_ADDR, 0x00);
    rfm96_write_reg(REG_LNA, 0x23);
    rfm96_write_reg(REG_IRQ_FLAGS_MASK, 0x00);
    rfm96_write_reg(REG_IRQ_FLAGS, 0xFF); 

    rfm96_write_reg(REG_OP_MODE, (0x80 | MODE_STDBY));

    busy_wait_us(10000);

    printf("Inicialização concluida com sucesso\n");

    return true;
}

bool rfm96_send(const uint8_t *data, size_t len) {
    

    rfm96_write_reg(REG_OP_MODE, (0x80 | MODE_STDBY));

    
    rfm96_write_reg(REG_FIFO_ADDR_PTR, 0x00);
    rfm96_write_fifo(data, (uint8_t)len);
    rfm96_write_reg(REG_PAYLOAD_LENGTH, (uint8_t)len);

    rfm96_write_reg(REG_IRQ_FLAGS, 0xFF);
    rfm96_write_reg(REG_DIO_MAPPING_1, 0x40); 

    printf("Envio de %d bytes\n", (int)len);

    rfm96_write_reg(REG_OP_MODE, (0x80 | MODE_TX));

    
    int timeout = TX_TIMEOUT_MS;
    while (timeout > 0) {
        
        if (rfm96_read_reg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) {
            rfm96_write_reg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK); 

            rfm96_write_reg(REG_OP_MODE, (0x80 | MODE_STDBY));

            printf("Transmissão realizada\n");
            return true;  
        }
        busy_wait_us(1000); 
        timeout--;
    }

    
    printf("lora configurado para Standby.\n");
    rfm96_write_reg(REG_OP_MODE, (0x80 | MODE_STDBY));

    return false; 
}

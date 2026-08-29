#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "fpga_config.h" 

// SPI & FPGA PIN DEFINITIONS 
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5
#define FPGA_RESET_PIN 4 

// W25Q32 COMMANDS 
#define FLASH_CMD_WREN  0x06 
#define FLASH_CMD_RDSR  0x05 
#define FLASH_CMD_SE    0x20 
#define FLASH_CMD_PP    0x02 
#define FLASH_CMD_CE    0xC7 

// HELPER FUNCTIONS
void flash_send_cmd(spi_device_handle_t spi, uint8_t cmd) {
    spi_transaction_t t = {};
    t.length = 8;                     
    t.tx_buffer = &cmd;               
    spi_device_transmit(spi, &t);
}

void flash_wait_ready(spi_device_handle_t spi) {
    // Create 2-byte arrays to safely hold 16 bits of data
    // TX: Send Command (0x05) followed by a dummy byte (0x00) to clock in the response
    uint8_t tx_data[2] = {FLASH_CMD_RDSR, 0x00}; 
    
    // RX: First byte is empty, second byte will hold the actual status register
    uint8_t rx_data[2] = {0x00, 0x00};

    spi_transaction_t t = {};
    t.length = 16;  // 16 bits = 2 bytes                  
    t.tx_buffer = tx_data;               
    t.rx_buffer = rx_data;            

    do {
        spi_device_transmit(spi, &t);
        vTaskDelay(pdMS_TO_TICKS(5)); 
    } while (rx_data[1] & 0x01); // Check the BUSY bit (Bit 0) of the second byte
}

// MAIN CONFIGURATION FUNCTION 
void configure_fpga_from_littlefs(void) {
    printf("Starting FPGA Configuration Process...\n");

    // 1. Isolate the FPGA
    gpio_set_direction(FPGA_RESET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(FPGA_RESET_PIN, 0); 
    vTaskDelay(pdMS_TO_TICKS(50));     

    // 2. Initialize ESP32 SPI Master
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 5 * 1000 * 1000, 
        .mode = 0,                         
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7
    };

    spi_device_handle_t spi;
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);

    // 3. Open File to determine size
    FILE* f = fopen("/data/fpga_bitstream.bit", "rb");
    if (f == NULL) {
        printf("Failed to open bitstream file!\n");
        // Clean up SPI before returning
        spi_bus_remove_device(spi);
        spi_bus_free(SPI2_HOST);
        return; 
    }

    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // 4. Erase the necessary Sectors (4KB each)
    size_t num_sectors = (file_size + 4095) / 4096;
    printf("Erasing %zu Flash Sectors...\n", num_sectors);
    
    for (size_t i = 0; i < num_sectors; i++) {
        flash_send_cmd(spi, FLASH_CMD_WREN); 
        uint32_t addr = i * 4096;
        uint8_t erase_tx[4] = {
            FLASH_CMD_SE, 
            (addr >> 16) & 0xFF, 
            (addr >> 8) & 0xFF, 
            addr & 0xFF
        }; 
        spi_transaction_t t_erase = {
            .length = 32,
            .tx_buffer = erase_tx
        };
        spi_device_transmit(spi, &t_erase);
        flash_wait_ready(spi); 
    }

    // 5. Read File and Program Flash 
    uint8_t file_buf[256];
    uint8_t spi_tx_buf[260]; 
    uint32_t current_addr = 0x000000;
    size_t bytes_read;

    printf("Writing Bitstream to Flash...\n");
    while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), f)) > 0) {
        flash_send_cmd(spi, FLASH_CMD_WREN); 

        spi_tx_buf[0] = FLASH_CMD_PP;        
        spi_tx_buf[1] = (current_addr >> 16) & 0xFF; 
        spi_tx_buf[2] = (current_addr >> 8) & 0xFF;  
        spi_tx_buf[3] = current_addr & 0xFF;         
        memcpy(&spi_tx_buf[4], file_buf, bytes_read); 

        spi_transaction_t t_write = {
            .length = 8 * (4 + bytes_read),
            .tx_buffer = spi_tx_buf
        };
        spi_device_transmit(spi, &t_write);
        flash_wait_ready(spi); 

        current_addr += 256;
    }
    fclose(f);
    printf("Write Complete! %u bytes written.\n", (unsigned int)current_addr);

    // 6. Clean Up and Boot FPGA 
    spi_bus_remove_device(spi);
    spi_bus_free(SPI2_HOST);
    
    gpio_set_direction(PIN_NUM_MOSI, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_NUM_MISO, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_INPUT);

    printf("Releasing FPGA Reset to Boot...\n");
    gpio_set_level(FPGA_RESET_PIN, 1); 
}
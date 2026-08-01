
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/spi_slave.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#define MOSI_PIN  23
#define MISO_PIN  19
#define CS_PIN    5
#define CLK_PIN   18
#define TRANSFER_SIZE 8 // Aligned to 4 bytes for DMA stability

void app_main(void) {
    spi_bus_config_t bus_config = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num = MISO_PIN,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1, // Must be -1 to prevent mapping to GPIO 0
        .quadhd_io_num = -1  // Must be -1 to prevent mapping to GPIO 0
    };

    spi_slave_interface_config_t slave_config = {
        .spics_io_num = CS_PIN,
        .mode = 0,
        .queue_size = 1,
    };

    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &bus_config, &slave_config, SPI_DMA_CH_AUTO);

    if (ret != ESP_OK) {
        printf("Slave init failed\n");
        return;
    }
    printf("SPI SLAVE IS WORKING - Waiting for Master...\n");

    char *tx_data = heap_caps_malloc(TRANSFER_SIZE, MALLOC_CAP_DMA);
    char *rx_data = heap_caps_malloc(TRANSFER_SIZE, MALLOC_CAP_DMA);

    if (tx_data == NULL || rx_data == NULL) {
        printf("DMA allocation failed\n");
        return;
    }
    printf("SPI SLAVE IS WORKING - Waiting for Master...\n");

    while (1) {
    memset(tx_data, 0, TRANSFER_SIZE);
    strcpy(tx_data, "TIWARI");
    memset(rx_data, 0, TRANSFER_SIZE);

    spi_slave_transaction_t transac = {
        .length = TRANSFER_SIZE * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    // This will block until the Master sends the clock signals
    ret = spi_slave_transmit(SPI2_HOST, &transac, portMAX_DELAY);

    if (ret == ESP_OK) {
        printf("Received NAME : %s\n", rx_data);
    } else {
        printf("NOT RECEIVED\n");
    }
    }
    free(tx_data);
    free(rx_data);
}
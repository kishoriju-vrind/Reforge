#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#define MOSI_PIN  23
#define MISO_PIN  19
#define CS_PIN    5
#define CLK_PIN   18
#define TRANSFER_SIZE 8

void app_main(void) {
    spi_bus_config_t bus_config = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num = MISO_PIN,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1, // Must be -1
        .quadhd_io_num = -1  // Must be -1
    };

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 100000, // 100 kHz
        .spics_io_num = CS_PIN,
        .mode = 0,
        .queue_size = 1,
    };

    spi_device_handle_t dev;
    esp_err_t ret;

    ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        printf("Bus init failed\n");
        return;
    }

    ret = spi_bus_add_device(SPI2_HOST, &dev_config, &dev);
    if (ret != ESP_OK) {
        printf("Device add failed\n");
        return;
    }
   
    char *tx_data = heap_caps_malloc(TRANSFER_SIZE, MALLOC_CAP_DMA);
    char *rx_data = heap_caps_malloc(TRANSFER_SIZE, MALLOC_CAP_DMA);

    if (tx_data == NULL || rx_data == NULL) {
        printf("DMA allocation failed\n");
        return;
    }

    printf("Master initialized. Starting continuous transmission...\n");

    while (1) {
    memset(tx_data, 0, TRANSFER_SIZE);
    strcpy(tx_data, "AMRIT");
    memset(rx_data, 0, TRANSFER_SIZE);
    
    spi_transaction_t transac = {
        .length = TRANSFER_SIZE * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    printf("Master initialized. Giving Slave 30 seconds to boot up...\n");
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay to allow slave to setup transaction

    printf("Starting transmission...\n");
    ret = spi_device_transmit(dev, &transac);

    if (ret == ESP_OK) {
        printf("Transmission Successful\n");
        printf("SURNAME IS  %s\n", rx_data);
    } else {
        printf("Transmission Failed\n");
    }
    }
    
    free(tx_data);
    free(rx_data);
}
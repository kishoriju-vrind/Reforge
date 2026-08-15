#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Defining ESP32E GPIO pins connected to the ICE40 FPGA/W25Q32 here
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

static const char *TAG = "W25Q32_SPI";

void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t spi;

    // 1. Configure the SPI Bus
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    // 2. Configure the SPI Device (Flash Memory)
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1000000,           // 1 MHz clock speed for reliable initialization
        .mode = 0,                           // SPI bus operation Mode 0 (0,0) is supported[span_3](start_span)[span_3](end_span)
        .spics_io_num = PIN_NUM_CS,          // Chip Select pin
        .queue_size = 1,                     // We only need 1 transaction in the queue
    };

    // Initialize the SPI bus on SPI2_HOST
    ESP_LOGI(TAG, "Initializing SPI bus...");
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    // Attach the Flash device to the SPI bus
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    // 3. Preparing the JEDEC ID Transaction
    // The JEDEC ID command is 9Fh[span_4](start_span)[span_4](end_span)
    // We send 1 byte (0x9F) and push 3 dummy bytes (0x00) to clock in the 3 response bytes.
    uint8_t tx_data[4] = {0x9F, 0x00, 0x00, 0x00};
    uint8_t rx_data[4] = {0};

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8 * 4;                 // Total length in bits (4 bytes = 32 bits)
    t.tx_buffer = tx_data;            // Data to send
    t.rx_buffer = rx_data;            // Buffer to receive data

    // 4. Execute the SPI Transaction
    ESP_LOGI(TAG, "Sending JEDEC ID command (0x9F)...");
    ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);

    // 5. Parse and verify the response
    uint8_t mfg_id = rx_data[1];
    uint16_t dev_id = (rx_data[2] << 8) | rx_data[3];

    ESP_LOGI(TAG, "Manufacturer ID: 0x%02X", mfg_id);
    ESP_LOGI(TAG, "Device ID: 0x%04X", dev_id);

    if (mfg_id == 0xEF && dev_id == 0x7016) {
        ESP_LOGI(TAG, "W25Q32JV-IM Flash Memory successfully verified!");
    } else {
        ESP_LOGW(TAG, "ID mismatch. Please check your wiring and connections.");
    }
}


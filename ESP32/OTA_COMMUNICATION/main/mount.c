#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_littlefs.h"

static const char *TAG = "LittleFS";

esp_err_t mount_littlefs(void) {
    ESP_LOGI(TAG, "Initializing LittleFS...");

    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/data",
        .partition_label = "storage",
        .format_if_mount_failed = true, // Formats blank flash on first boot; preserves files thereafter
        .dont_mount = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition with label '%s'", conf.partition_label);
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %u bytes, used: %u bytes, free: %u bytes", 
                 (unsigned int)total, (unsigned int)used, (unsigned int)(total - used));
    }

    return ESP_OK;
}
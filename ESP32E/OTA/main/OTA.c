#include <stdio.h>
#include "fpga_config.h"
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_http_server.h"

static const char *TAG = "OTA_SERVER";

// This tells OTA.c that the mount function exists in the other file
esp_err_t mount_littlefs(void);
//1. GET HANDLER (Shows the Upload Webpage)
void configure_fpga_from_littlefs(void); // This tells OTA.c that the configure function exists in the other file
static esp_err_t index_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving the upload webpage...");
    
    const char* html_page = 
        "<!DOCTYPE html><html><body style='font-family: Arial; text-align: center; margin-top: 50px;'>"
        "<h2>FPGA Bitstream OTA Updater</h2>"
        "<input type='file' id='fileInput' style='margin-bottom: 20px;'><br>"
        "<button onclick='uploadFile()' style='padding: 10px 20px; font-size: 16px;'>Upload to ESP32</button>"
        "<h3 id='status' style='color: blue; margin-top: 20px;'></h3>"
        "<script>"
        "function uploadFile() {"
        "  var file = document.getElementById('fileInput').files[0];"
        "  if (!file) { alert('Select a file first!'); return; }"
        "  document.getElementById('status').innerText = 'Uploading... Please wait.';"
        "  fetch('/upload', { method: 'POST', body: file })"
        "  .then(response => response.text())"
        "  .then(text => document.getElementById('status').innerText = text)"
        "  .catch(err => document.getElementById('status').innerText = 'Upload failed!');"
        "}"
        "</script>"
        "</body></html>";

    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
//2. POST HANDLER (Receives the file and saves it)
static esp_err_t upload_post_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Incoming OTA bitstream detected!");

    FILE *fd = fopen("/data/fpga_bitstream.bit", "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to open file in LittleFS");
        //configure_fpga_from_littlefs(); // Attempt to configure our FPGA even if file open fails
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Storage Error");
        return ESP_FAIL;
    }

    char buf[1024]; 
    int received;
    int remaining = req->content_len;

    while (remaining > 0) {
        if ((received = httpd_req_recv(req, buf, (remaining < sizeof(buf) ? remaining : sizeof(buf)))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) continue;
            fclose(fd);
            ESP_LOGE(TAG, "File reception failed!");
            return ESP_FAIL;
        }
        fwrite(buf, 1, received, fd); 
        remaining -= received;
        ESP_LOGI(TAG, "Received %d bytes. Remaining: %d bytes", received, remaining);
    }

    fclose(fd);
    ESP_LOGI(TAG, "Success! Saved to /data/fpga_bitstream.bit");
    configure_fpga_from_littlefs(); // Calling  FPGA configuration function after successful upload
    httpd_resp_sendstr(req, "Bitstream upload successful! Saved in LittleFS.");
    return ESP_OK;
}
//  3. START THE SERVER

static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_get_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t upload_uri = { .uri = "/upload", .method = HTTP_POST, .handler = upload_post_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &upload_uri);
        return server;
    }
    return NULL;
}
// 4. MAIN APPLICATION

void app_main(void) {
    ESP_LOGI(TAG, "=== Starting FPGA OTA Project ===");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect()); // Connect to the Local Wi-Fi
    ESP_ERROR_CHECK(mount_littlefs());  // Turn on storage of esp 
    
    start_webserver();                  // Start listening for files
    
    ESP_LOGI(TAG, "System Ready! Check IP address above.");
}

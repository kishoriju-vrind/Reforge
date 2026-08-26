#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_http_server.h"

static const char *TAG = "OTA_SERVER";

// Mount function declared in mount.c
esp_err_t mount_littlefs(void);

// Stored in Flash (.rodata) to prevent overflowing the FreeRTOS task stack
static const char html_page[] =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>FPGA OTA Update</title>"
    "<style>"
    "body {"
    "  margin: 0;"
    "  padding: 40px 20px;"
    "  font-family: Arial, sans-serif;"
    "  background: #f5f5f5;"
    "  color: #222;"
    "}"
    ".container {"
    "  max-width: 500px;"
    "  margin: auto;"
    "  background: white;"
    "  padding: 30px;"
    "  border: 1px solid #ddd;"
    "  border-radius: 8px;"
    "}"
    "h1 {"
    "  margin-top: 0;"
    "  font-size: 24px;"
    "  font-weight: 500;"
    "}"
    ".subtitle {"
    "  color: #666;"
    "  font-size: 14px;"
    "  margin-bottom: 25px;"
    "}"
    ".upload-area {"
    "  border: 1px dashed #aaa;"
    "  padding: 30px 20px;"
    "  text-align: center;"
    "  cursor: pointer;"
    "  border-radius: 6px;"
    "  display: block;"
    "}"
    ".upload-area:hover {"
    "  border-color: #333;"
    "  background: #fafafa;"
    "}"
    "#fileInput {"
    "  display: none;"
    "}"
    ".file-info {"
    "  display: none;"
    "  margin-top: 15px;"
    "  padding: 12px;"
    "  background: #f5f5f5;"
    "  border-radius: 5px;"
    "  font-size: 14px;"
    "}"
    ".file-name {"
    "  font-weight: bold;"
    "  word-break: break-all;"
    "}"
    ".file-size {"
    "  margin-top: 5px;"
    "  color: #666;"
    "}"
    ".button {"
    "  width: 100%;"
    "  margin-top: 20px;"
    "  padding: 12px;"
    "  border: none;"
    "  border-radius: 5px;"
    "  background: #333;"
    "  color: white;"
    "  font-size: 15px;"
    "  cursor: pointer;"
    "}"
    ".button:hover {"
    "  background: #222;"
    "}"
    ".button:disabled {"
    "  background: #aaa;"
    "  cursor: not-allowed;"
    "}"
    ".progress-container {"
    "  display: none;"
    "  margin-top: 20px;"
    "}"
    ".progress-bar {"
    "  width: 100%;"
    "  height: 8px;"
    "  background: #ddd;"
    "  border-radius: 5px;"
    "  overflow: hidden;"
    "}"
    ".progress {"
    "  width: 0%;"
    "  height: 100%;"
    "  background: #333;"
    "}"
    "#progressText {"
    "  display: block;"
    "  margin-bottom: 7px;"
    "  font-size: 13px;"
    "  color: #666;"
    "}"
    "#status {"
    "  margin-top: 18px;"
    "  text-align: center;"
    "  font-size: 14px;"
    "}"
    ".footer {"
    "  margin-top: 25px;"
    "  text-align: center;"
    "  font-size: 12px;"
    "  color: #999;"
    "}"
    "</style>"
    "</head>"
    "<body>"
    "<div class='container'>"
    "<h1>FPGA OTA Update</h1>"
    "<div class='subtitle'>Upload a new FPGA bitstream to the controller.</div>"
    "<label class='upload-area' id='dropArea' for='fileInput'>Choose .bin file</label>"
    "<input type='file' id='fileInput' accept='.bin'>"
    "<div class='file-info' id='fileInfo'>"
    "<div class='file-name' id='fileName'></div>"
    "<div class='file-size' id='fileSize'></div>"
    "</div>"
    "<button class='button' id='uploadButton' onclick='uploadFile()' disabled>Upload</button>"
    "<div class='progress-container' id='progressContainer'>"
    "<span id='progressText'>0%</span>"
    "<div class='progress-bar'>"
    "<div class='progress' id='progress'></div>"
    "</div>"
    "</div>"
    "<div id='status'></div>"
    "<div class='footer'>ESP32 FPGA OTA Controller</div>"
    "</div>"
    "<script>"
    "var selectedFile = null;"
    "var fileInput = document.getElementById('fileInput');"
    "var dropArea = document.getElementById('dropArea');"
    "var fileInfo = document.getElementById('fileInfo');"
    "var fileName = document.getElementById('fileName');"
    "var fileSize = document.getElementById('fileSize');"
    "var uploadButton = document.getElementById('uploadButton');"
    "var statusMsg = document.getElementById('status');"
    "fileInput.addEventListener('change', function() {"
    "  handleFile(this.files[0]);"
    "});"
    "dropArea.addEventListener('dragover', function(e) {"
    "  e.preventDefault();"
    "});"
    "dropArea.addEventListener('drop', function(e) {"
    "  e.preventDefault();"
    "  handleFile(e.dataTransfer.files[0]);"
    "});"
    "function handleFile(file) {"
    "  if (!file) return;"
    "  if (!file.name.toLowerCase().endsWith('.bin')) {"
    "    statusMsg.innerText = 'Please select a .bin file.';"
    "    statusMsg.style.color = '#c00';"
    "    return;"
    "  }"
    "  selectedFile = file;"
    "  fileName.innerText = file.name;"
    "  fileSize.innerText = formatSize(file.size);"
    "  fileInfo.style.display = 'block';"
    "  uploadButton.disabled = false;"
    "  statusMsg.innerText = '';"
    "}"
    "function formatSize(bytes) {"
    "  if (bytes < 1024) return bytes + ' B';"
    "  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';"
    "  return (bytes / (1024 * 1024)).toFixed(2) + ' MB';"
    "}"
    "function uploadFile() {"
    "  if (!selectedFile) return;"
    "  uploadButton.disabled = true;"
    "  statusMsg.innerText = 'Uploading...';"
    "  statusMsg.style.color = '#333';"
    "  document.getElementById('progressContainer').style.display = 'block';"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open('POST', '/upload', true);"
    "  xhr.upload.onprogress = function(event) {"
    "    if (event.lengthComputable) {"
    "      var percent = Math.round((event.loaded / event.total) * 100);"
    "      document.getElementById('progress').style.width = percent + '%';"
    "      document.getElementById('progressText').innerText = percent + '%';"
    "    }"
    "  };"
    "  xhr.onload = function() {"
    "    uploadButton.disabled = false;"
    "    if (xhr.status >= 200 && xhr.status < 300) {"
    "      document.getElementById('progress').style.width = '100%';"
    "      document.getElementById('progressText').innerText = '100%';"
    "      statusMsg.innerText = xhr.responseText;"
    "      statusMsg.style.color = '#080';"
    "    } else {"
    "      statusMsg.innerText = 'Upload failed. HTTP ' + xhr.status;"
    "      statusMsg.style.color = '#c00';"
    "    }"
    "  };"
    "  xhr.onerror = function() {"
    "    uploadButton.disabled = false;"
    "    statusMsg.innerText = 'Upload failed. Check the ESP32 connection.';"
    "    statusMsg.style.color = '#c00';"
    "  };"
    "  xhr.send(selectedFile);"
    "}"
    "</script>"
    "</body>"
    "</html>";

// 1. GET HANDLER (Shows the Upload Webpage)
static esp_err_t index_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving the upload webpage...");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// 2. POST HANDLER (Receives the bitstream file and writes directly to LittleFS)
static esp_err_t upload_post_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Incoming OTA bitstream detected! Total size: %d bytes", req->content_len);

    FILE *fd = fopen("/data/fpga_bitstream.bin", "wb");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to open /data/fpga_bitstream.bin for writing");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Storage Write Error");
        return ESP_FAIL;
    }

    char buf[4096]; 
    int received;
    int remaining = req->content_len;

    while (remaining > 0) {
        size_t to_recv = (remaining < sizeof(buf)) ? remaining : sizeof(buf);
        if ((received = httpd_req_recv(req, buf, to_recv)) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            fclose(fd);
            ESP_LOGE(TAG, "File reception failed or socket disconnected!");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Transfer Aborted");
            return ESP_FAIL;
        }

        size_t written = fwrite(buf, 1, received, fd);
        if (written != received) {
            fclose(fd);
            ESP_LOGE(TAG, "LittleFS write error (disk full or hardware fault)");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Disk Full / Write Error");
            return ESP_FAIL;
        }

        remaining -= received;
        ESP_LOGD(TAG, "Received %d bytes. Remaining: %d bytes", received, remaining);
    }

    fclose(fd);
    ESP_LOGI(TAG, "Success! Saved bitstream to /data/fpga_bitstream.bin");
    httpd_resp_sendstr(req, "Bitstream upload successful! Saved in LittleFS.");
    return ESP_OK;
}

// 3. START THE SERVER
static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192; // Ensure sufficient stack for file I/O operations
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t upload_uri = {
            .uri = "/upload",
            .method = HTTP_POST,
            .handler = upload_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &upload_uri);
        return server;
    }
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
}

// 4. MAIN APPLICATION
void app_main(void) {
    ESP_LOGI(TAG, "=== Starting FPGA OTA Project ===");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect()); // Connects using menuconfig Wi-Fi credentials
    ESP_ERROR_CHECK(mount_littlefs());  // Mounts the LittleFS partition

    start_webserver();

    ESP_LOGI(TAG, "System Ready! Webserver listening for incoming bitstreams.");
}
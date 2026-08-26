idf_component_register(
    SRCS "OTA.c" "mount.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_netif esp_http_server littlefs protocol_examples_common
)
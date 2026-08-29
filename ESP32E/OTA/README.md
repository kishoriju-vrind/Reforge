# ESP32 OTA Firmware

This folder contains the ESP32 code for OTA (Over-The-Air) firmware updates.

With OTA, we can update the firmware on the ESP32 over the network without connecting it to the computer every time.

## Build and Flash

Make sure ESP-IDF is installed and configured.

Open this folder in VS Code or the ESP-IDF terminal and run:

```bash
idf.py build flash monitor

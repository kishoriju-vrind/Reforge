ESP32 OTA Firmware

This folder contains the ESP32-side code for OTA (Over-The-Air) firmware updates.

OTA allows us to update the ESP32 firmware over the network without flashing it through USB every time.

Build and Flash

Make sure ESP-IDF is installed and set up.

Open this folder in VS Code or the ESP-IDF terminal and run:

idf.py build flash monitor

This builds the code, flashes it to the ESP32, and opens the serial monitor.

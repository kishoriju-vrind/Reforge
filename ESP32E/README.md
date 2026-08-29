📡 ESP32 Firmware & OTA Module

This folder contains the ESP32 code used in the Reforge project. It includes the code and files needed to run the ESP32 and handle OTA updates.

---

📂 Folder Structure

- "OTA/" – Main ESP32 project folder.
  - "main/" – Contains the main source files such as "OTA.c", "fpga_config.c", and "mount.c".
  - "partitions.csv" – Contains the partition settings used for storing the firmware.
  - "sdkconfig" – Contains the ESP-IDF configuration settings for the project.
  - "CMakeLists.txt" – Used to configure and build the project.

---

🛠️ Build and Flash

This project uses the ESP-IDF framework to build and upload the code to the ESP32.

Steps:

1. Open the "OTA" folder in VS Code with the ESP-IDF Extension installed.
2. Connect the ESP32 board to your computer using a USB cable.
3. Open the ESP-IDF terminal.
4. Run the following command:

idf.py build flash monitor

This command will:

- Build the project.
- Flash the firmware to the ESP32.
- Open the serial monitor to see the ESP32 output.

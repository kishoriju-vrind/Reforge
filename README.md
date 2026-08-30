<div align="center">

# Reforge

**Custom FPGA Development Board & ESP32 Host System**

</div>

## Table of Contents
* [About the Project](#about-the-project)
  * [Aim](#aim)
  * [Description](#description)
  * [Tech-Stack](#tech-stack)
* [File Structure](#file-structure)
* [Getting Started](#getting-started)
* [Usage](#usage)
  * [1. iCE40 FPGA Graphics Demo](#1-ice40-fpga-graphics-demo)
  * [2. ESP32 Host Firmware](#2-esp32-host-firmware)

---

## About the Project

### Aim
Designing a custom PCB development board that combines an ESP32 microcontroller and an iCE40 FPGA to understand how microcontrollers and FPGAs interact in real embedded systems.

### Description
Instead of relying on fixed hardware, FPGAs allow for custom hardware configurations. In this project, the **ESP32** acts as the host system. It is responsible for storing FPGA designs in its filesystem and loading them directly onto the **iCE40 FPGA** when needed. The two chips communicate via SPI, allowing the ESP32 to send data and control the FPGA dynamically. 

The current hardware prototype includes a custom VGA output implementation, where the FPGA acts as a hardwired video card rendering horizontal RGB color bands in real-time.

### Tech-Stack
* **Languages:** Embedded C (ESP32), Verilog HDL (FPGA)
* **Tools:** ESP-IDF, OSS CAD Suite (Yosys, NextPNR), KiCAD
* **Protocols:** SPI (Inter-chip communication), VGA (Analog Video),OTA 

---

## File Structure

```text
Reforge
├── ESP32E                  # ESP32 source code and firmware
│   └── OTA                 # Host system and file management
│       ├── main            # Core C files (SPI communication, FPGA config)
│       ├── CMakeLists.txt  # Build configuration
│       └── partitions.csv  # Custom memory partitions for bitstream storage
        └── README.md  
├── ICE40                   # FPGA hardware designs in Verilog
│   ├── blink               # Basic LED blink and SPI test
        ├── blink.v           # Verilog logic for horizontal RGB bands
│       ├── pins.pcf        # FPGA pinout mapping
│       ├── build.bat       # Automated synthesis script
│       └── flash.bat       # Automated flashing script
│   └── VGA                 # Custom hardware VGA generator
│       ├── vga.v           # Verilog logic for horizontal RGB bands
│       ├── pins.pcf        # FPGA pinout mapping
│       ├── build.bat       # Automated synthesis script
│       └── flash.bat       # Automated flashing script
├── KICAD                   # Schematics for the custom development board
│   ├── Reforge.kicad_sch
│   └── Reforge.kicad_pcb
└── README.md

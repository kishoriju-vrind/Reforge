# iCE40 FPGA Hardware Designs

This folder contains the Verilog code for the UPduino iCE40 FPGA. It includes basic FPGA tests and a VGA project.

## Sub-Projects

- `blink/` - LED blink test with SPI slave communication.
- `VGA/` - VGA display project that generates RGB lines at 640x480 resolution.

## How to Build and Flash

Each project has its own batch files for building and flashing.

1. Open the `blink` or `VGA` folder.
2. Run `build.bat` to build the Verilog design.
3. Run `flash.bat` to flash the bitstream to the FPGA.

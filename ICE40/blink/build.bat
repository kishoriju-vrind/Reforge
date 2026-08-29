@echo off
echo --- Step 1: Synthesizing Verilog ---
yosys -p "synth_ice40 -top blink -json blink.json" blink.v

echo --- Step 2: Place and Route ---
nextpnr-ice40 --up5k --package sg48 --json blink.json --pcf blink.pcf --asc blink.asc

echo --- Step 3: Generating Bitstream ---
icepack blink.asc blink.bin

echo --- Success! Created blink.bin ---
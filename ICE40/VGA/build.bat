yosys -p "synth_ice40 -top vga_top -json vga.json" vga.v
nextpnr-ice40 --up5k --package sg48 --json vga.json --pcf pins.pcf --asc vga.asc
icepack vga.asc vga.bin
pause

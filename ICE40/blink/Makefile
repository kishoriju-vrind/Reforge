DEVICE  = up5k
PACKAGE = sg48
TOP     = blink

all: $(TOP).bin

$(TOP).json: $(TOP).v
	yosys -p "synth_ice40 -top $(TOP) -json $@" $<

$(TOP).asc: $(TOP).json $(TOP).pcf
	nextpnr-ice40 --$(DEVICE) --package $(PACKAGE) --json $< --pcf $(TOP).pcf --asc $@

$(TOP).bin: $(TOP).asc
	icepack $< $@

flash: $(TOP).bin
	iceprog $<

clean:
	rm -f *.json *.asc *.bin

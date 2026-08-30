module vga_top (
    output o_Red,
    output o_Green,
    output o_Blue,
    output o_HSync,
    output o_VSync
);

    // 1. Generate a ~24 MHz clock using the iCE40 internal oscillator
    // (24 MHz is close enough to the standard 25.175 MHz for 99% of monitors to work perfectly)
    wire clk_48mhz;
    SB_HFOSC inthosc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(clk_48mhz)
    );
    
    reg clk_24mhz = 0;
    always @(posedge clk_48mhz) begin
        clk_24mhz <= ~clk_24mhz;
    end

    // 2. Pixel Counters (The "Scanner" tracking X and Y coordinates)
    reg [9:0] x_counter = 0; // Counts 0 to 799 horizontally
    reg [9:0] y_counter = 0; // Counts 0 to 524 vertically

    always @(posedge clk_24mhz) begin
        if (x_counter == 799) begin
            x_counter <= 0;
            if (y_counter == 524)
                y_counter <= 0;
            else
                y_counter <= y_counter + 1;
        end else begin
            x_counter <= x_counter + 1;
        end
    end

    // 3. Synchronization Signals (Telling the monitor when to wrap around)
    // HSync goes LOW during the standard horizontal sync pulse window
    assign o_HSync = (x_counter >= 656 && x_counter < 752) ? 0 : 1;
    // VSync goes LOW during the standard vertical sync pulse window
    assign o_VSync = (y_counter >= 490 && y_counter < 492) ? 0 : 1;

    // 4. Color Output (Drawing the Three Horizontal Bands)
    // Only draw when the scanner is in the visible 640x480 area
    wire video_active = (x_counter < 640) && (y_counter < 480);
    
    // Top third of the screen (Lines 0 to 159)
    assign o_Red   = video_active && (y_counter < 160);
    // Middle third of the screen (Lines 160 to 319)
    assign o_Green = video_active && (y_counter >= 160 && y_counter < 320);
    // Bottom third of the screen (Lines 320 to 479)
    assign o_Blue  = video_active && (y_counter >= 320);

endmodule

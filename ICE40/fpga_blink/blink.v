module top (
    output wire led_blue,
    output wire led_green,
    output wire led_red
);

    // UPduino LEDs are Active LOW: 1 = OFF, 0 = ON
    assign led_green = 1'b1; // Force GREEN OFF
    assign led_blue  = 1'b0; // Force BLUE ON
    assign led_red   = 1'b0; // Force RED ON

endmodule
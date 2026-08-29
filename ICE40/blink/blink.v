module blink (
    output wire led_blue,
    output wire led_green,
    output wire led_red
);

    // Active LOW: 0 = ON, 1 = OFF
    assign led_green = 1'b1; // GREEN OFF
    assign led_blue  = 1'b1; // BLUE OFF
    assign led_red   = 1'b0; // RED ON

endmodule
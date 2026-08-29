module spi_slave_receiver (
    input wire sclk,          // SPI Clock from ESP32
    input wire cs,            // Chip Select (Active Low from ESP32)
    input wire mosi,          // Master Out, Slave In (Data from ESP32)
    output reg [7:0] saved_data // The 8-bit value saved on the FPGA
);

    reg [2:0] bit_cnt;
    reg [7:0] shift_reg;

    always @(posedge sclk or posedge cs) begin
        if (cs) begin
            // When Chip Select goes HIGH, reset the bit counter
            bit_cnt <= 3'd0;
        end else begin
            // Shift in incoming bits from the ESP32 one by one
            shift_reg <= {shift_reg[6:0], mosi};
            bit_cnt <= bit_cnt + 3'd1;
            
            // Once all 8 bits are captured, "save" them to the output register
            if (bit_cnt == 3'd7) begin
                saved_data <= {shift_reg[6:0], mosi};
            end
        end
    end

endmodule
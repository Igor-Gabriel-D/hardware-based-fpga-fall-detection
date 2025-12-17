module fall_detect_mag_sq (
    input  logic        clk,
    input  logic        rst_n,

    input  logic        data_valid,   // pulso quando AX/AY/AZ são válidos
    input  logic signed [15:0] ax,
    input  logic signed [15:0] ay,
    input  logic signed [15:0] az,

    output logic [31:0] mag_sq,       // ax² + ay² + az²
    output logic        mag_valid
);

    logic signed [31:0] ax_s, ay_s, az_s;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            ax_s      <= 0;
            ay_s      <= 0;
            az_s      <= 0;
            mag_sq    <= 0;
            mag_valid <= 1'b0;
        end else if (data_valid) begin
            ax_s      <= ax * ax;
            ay_s      <= ay * ay;
            az_s      <= az * az;
            mag_sq    <= (ax * ax) + (ay * ay) + (az * az);
            mag_valid <= 1'b1;
        end else begin
            mag_valid <= 1'b0;
        end
    end

endmodule


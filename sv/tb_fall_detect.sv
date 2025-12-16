`timescale 1ns/1ps

module tb_fall_detect;

    localparam CLK_FREQ_HZ   = 50_000_000;
    localparam STILL_TIME_MS = 10; // pequeno para simulação
    localparam SAMPLE_RATE_HZ = 50; // equivalente ao busy_wait_us(20000)

    logic clk = 0;
    always #10 clk = ~clk; // 50 MHz

    logic rst_n;
    logic data_valid;
    logic signed [15:0] ax, ay, az;

    logic [31:0] impact_thresh_sq;
    logic [31:0] still_thresh_sq;

    logic fall_detected;

    fall_detect #(
        .SAMPLE_RATE_HZ(50),
        .STILL_TIME_MS(STILL_TIME_MS)
    ) dut (
        .clk(clk),
        .rst_n(rst_n),
        .data_valid(data_valid),
        .ax(ax),
        .ay(ay),
        .az(az),
        .impact_thresh_sq(impact_thresh_sq),
        .still_thresh_sq(still_thresh_sq),
        .fall_detected(fall_detected)
    );


    // Envia uma amostra (equivalente a mpu6050_read)
    task send_sample(input signed [15:0] x,
                     input signed [15:0] y,
                     input signed [15:0] z);
    begin
        data_valid <= 1'b1;
        ax <= x; ay <= y; az <= z;
        @(posedge clk);
        data_valid <= 1'b0;
        @(posedge clk);
    end
    endtask

    initial begin
        $dumpfile("fall_detect.vcd");
        $dumpvars(0, tb_fall_detect);

        rst_n = 0;
        data_valid = 0;
        ax = 0; ay = 0; az = 0;

        impact_thresh_sq = 20000 * 20000;
        still_thresh_sq  = 3000  * 3000;

        repeat (5) @(posedge clk);
        rst_n = 1;

        $display("=== INÍCIO DA SIMULAÇÃO ===");

        // 1) Movimento normal
        repeat (10)
            send_sample(2000, 1500, 1000);

        // 2) Impacto
        send_sample(25000, 0, 0);

        // 3) Imóvel — CONTINUA LENDO O SENSOR
        // (equivalente ao while(1) do C)
        repeat ((STILL_TIME_MS * SAMPLE_RATE_HZ) / 1000 + 2) begin
            send_sample(300, 200, 100); // abaixo do still threshold
        end

        // 4) Verificação
        if (fall_detected)
            $display(">>> QUEDA DETECTADA COM SUCESSO <<<");
        else
            $display("!!! ERRO: QUEDA NÃO DETECTADA !!!");

        #200;
        $finish;
    end

endmodule


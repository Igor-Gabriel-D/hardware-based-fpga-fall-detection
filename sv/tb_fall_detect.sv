`timescale 1ns/1ps

module tb_fall_detect_mag_sq;

    // ----------------------------
    // Sinais do DUT
    // ----------------------------
    logic        clk;
    logic        rst_n;
    logic        data_valid;
    logic signed [15:0] ax, ay, az;
    logic [31:0] mag_sq;
    logic        mag_valid;

    // ----------------------------
    // Instância do DUT
    // ----------------------------
    fall_detect_mag_sq dut (
        .clk(clk),
        .rst_n(rst_n),
        .data_valid(data_valid),
        .ax(ax),
        .ay(ay),
        .az(az),
        .mag_sq(mag_sq),
        .mag_valid(mag_valid)
    );

    // ----------------------------
    // Clock: 10 ns (100 MHz)
    // ----------------------------
    initial clk = 0;
    always #5 clk = ~clk;

    // ----------------------------
    // Task para aplicar uma amostra
    // ----------------------------
    task send_sample(
        input signed [15:0] tax,
        input signed [15:0] tay,
        input signed [15:0] taz
    );
    begin
        @(posedge clk);
        ax         <= tax;
        ay         <= tay;
        az         <= taz;
        data_valid <= 1'b1;

        @(posedge clk);
        data_valid <= 1'b0;
    end
    endtask

    // ----------------------------
    // Teste principal
    // ----------------------------
    initial begin
        // Valores iniciais
        ax = 0;
        ay = 0;
        az = 0;
        data_valid = 0;
        rst_n = 0;

        // Reset
        repeat (3) @(posedge clk);
        rst_n = 1;

        // ----------------------------
        // Casos de teste
        // ----------------------------

        // Caso 1: tudo zero
        send_sample(0, 0, 0);

        // Caso 2: vetor simples
        send_sample(1000, 0, 0);        // mag_sq = 1_000_000

        // Caso 3: eixo Y
        send_sample(0, -2000, 0);       // mag_sq = 4_000_000

        // Caso 4: vetor 3D
        send_sample(1000, 2000, 3000);  // mag_sq = 1e6 + 4e6 + 9e6 = 14e6

        // Caso 5: valores negativos grandes
        send_sample(-15000, -15000, -15000);

        // Aguarda um pouco e encerra
        repeat (5) @(posedge clk);
        $finish;
    end

    // ----------------------------
    // Monitor
    // ----------------------------
    always @(posedge clk) begin
        if (mag_valid) begin
            $display(
                "[%0t] AX=%0d AY=%0d AZ=%0d | mag_sq=%0d",
                $time, ax, ay, az, mag_sq
            );
        end
    end

endmodule


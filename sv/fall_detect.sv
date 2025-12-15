module fall_detect #(
    parameter CLK_FREQ_HZ = 50_000_000, // Frequência do clock da FPGA (Ex: 50MHz ou 27MHz)
    parameter STILL_TIME_MS = 2000      // Tempo que deve ficar imóvel (2 segundos)
)(
    input  logic        clk,
    input  logic        rst_n,          // Reset ativo em nível baixo
    
    // Entradas do Acelerômetro (Valores brutos 16-bit signed)
    input  logic        data_valid,     // Pulso de 1 ciclo quando novos dados chegam
    input  logic signed [15:0] ax,
    input  logic signed [15:0] ay,
    input  logic signed [15:0] az,

    // Configuração de Limiares (Podem ser registradores configuráveis via SPI/I2C ou fixos)
    // Nota: Devem ser o valor ao QUADRADO para evitar raiz quadrada
    input  logic [31:0] impact_thresh_sq, 
    input  logic [31:0] still_thresh_sq,

    output logic        fall_detected   // Pulso de saída quando queda confirmada
);

    // --- 1. Cálculo da Magnitude (Pipeline) ---
    // Usamos 32 bits porque (2^15)^2 * 3 cabe em 32 bits unsigned.
    logic [31:0] ax_sq, ay_sq, az_sq;
    logic [31:0] mag_sq;
    logic        mag_valid;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            ax_sq <= 0; ay_sq <= 0; az_sq <= 0;
            mag_sq <= 0;
            mag_valid <= 0;
        end else if (data_valid) begin
            // Estágio 1: Elevar ao quadrado
            ax_sq <= ax * ax;
            ay_sq <= ay * ay;
            az_sq <= az * az;
            mag_valid <= 1'b1; // Sinaliza que o pipeline avançou
        end else begin
            mag_valid <= 1'b0; // Só calculamos quando há dado novo
        end
    end

    // Estágio 2: Soma (combinacional ou registrado, aqui registrado no processamento da FSM)
    wire [31:0] current_mag_sq = ax_sq + ay_sq + az_sq;


    // --- 2. Temporizador ---
    localparam TIME_CYCLES = (CLK_FREQ_HZ / 1000) * STILL_TIME_MS;
    logic [$clog2(TIME_CYCLES)-1:0] timer_cnt;
    logic timer_en, timer_rst;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) timer_cnt <= 0;
        else if (timer_rst) timer_cnt <= 0;
        else if (timer_en && mag_valid) timer_cnt <= timer_cnt + 1; // Incrementa a cada amostra
        // Nota: Se data_valid for muito rápido, o contador deve ser baseado em clk real
        // Se data_valid for lento (ex: 100Hz), ajustamos a constante TIME_CYCLES.
    end

    // --- 3. Máquina de Estados (FSM) ---
    typedef enum logic [1:0] {
        IDLE,           // Esperando Impacto
        WAIT_STILL,     // Impacto detectado, verificando imobilidade
        ALARM           // Queda confirmada
    } state_t;

    state_t state, next_state;

    // Lógica Sequencial da FSM
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) state <= IDLE;
        else if (mag_valid) state <= next_state; // Só muda de estado quando chega dado novo
    end

    // Lógica Combinacional da FSM
    always_comb begin
        next_state = state;
        fall_detected = 1'b0;
        timer_en = 1'b0;
        timer_rst = 1'b0;

        case (state)
            IDLE: begin
                timer_rst = 1'b1;
                // Se magnitude > impacto (Lembra: comparamos quadrados!)
                if (current_mag_sq > impact_thresh_sq) begin
                    next_state = WAIT_STILL;
                end
            end

            WAIT_STILL: begin
                timer_en = 1'b1;
                
                // Se houve movimento brusco, reseta (conforme seu código C: else impact_time = 0)
                if (current_mag_sq >= still_thresh_sq) begin
                    next_state = IDLE;
                end
                // Se o tempo passou (conversão de millisegundos para ciclos/amostras)
                // Assumindo aqui que timer conta amostras. Se contar clk, a lógica muda levemente.
                else if (timer_cnt >= TIME_CYCLES) begin
                    next_state = ALARM;
                end
            end

            ALARM: begin
                fall_detected = 1'b1;
                next_state = IDLE; // Retorna ao início ou fica travado até reset externo
            end
        endcase
    end

endmodule
module fall_detect #(
    // Taxa de amostragem do acelerômetro (equivale ao busy_wait_us do C)
    parameter SAMPLE_RATE_HZ = 50,      // 50 Hz → 20 ms
    parameter STILL_TIME_MS  = 300       // Igual ao seu C
)(
    input  logic        clk,
    input  logic        rst_n,

    // Entradas do acelerômetro
    input  logic        data_valid,      // 1 pulso por amostra
    input  logic signed [15:0] ax,
    input  logic signed [15:0] ay,
    input  logic signed [15:0] az,

    // Limiares AO QUADRADO
    input  logic [31:0] impact_thresh_sq,
    input  logic [31:0] still_thresh_sq,

    output logic        fall_detected    // Pulso de 1 amostra
);

    // ============================================================
    // 1. Cálculo da magnitude ao quadrado (sem raiz)
    // ============================================================
    logic [31:0] ax_sq, ay_sq, az_sq;
    logic [31:0] mag_sq;
    logic        mag_valid;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            ax_sq <= 0;
            ay_sq <= 0;
            az_sq <= 0;
            mag_sq <= 0;
            mag_valid <= 1'b0;
        end else if (data_valid) begin
            ax_sq <= ax * ax;
            ay_sq <= ay * ay;
            az_sq <= az * az;
            mag_sq <= (ax * ax) + (ay * ay) + (az * az);
            mag_valid <= 1'b1;
        end else begin
            mag_valid <= 1'b0;
        end
    end

    // ============================================================
    // 2. Temporizador baseado em AMOSTRAS
    // ============================================================
    localparam int TIME_SAMPLES =
        (STILL_TIME_MS * SAMPLE_RATE_HZ) / 1000;

    logic [$clog2(TIME_SAMPLES+1)-1:0] timer_cnt;

    // ============================================================
    // 3. Máquina de Estados (FSM)
    // ============================================================
    typedef enum logic [1:0] {
        IDLE,
        WAIT_STILL,
        ALARM
    } state_t;

    state_t state, next_state;

    // FSM sequencial
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            state <= IDLE;
        else if (mag_valid)
            state <= next_state;
    end

    // Contador de tempo (amostras)
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            timer_cnt <= 0;
        else if (state == IDLE)
            timer_cnt <= 0;
        else if (state == WAIT_STILL && mag_valid)
            timer_cnt <= timer_cnt + 1;
    end

    // FSM combinacional
    always_comb begin
        next_state    = state;
        fall_detected = 1'b0;

        case (state)
            // ----------------------------------------------------
            IDLE: begin
                if (mag_sq > impact_thresh_sq)
                    next_state = WAIT_STILL;
            end

            // ----------------------------------------------------
            WAIT_STILL: begin
                // Se voltou a se mexer → cancela
                if (mag_sq >= still_thresh_sq)
                    next_state = IDLE;
                // Se ficou imóvel tempo suficiente → queda
                else if (timer_cnt >= TIME_SAMPLES)
                    next_state = ALARM;
            end

            // ----------------------------------------------------
            ALARM: begin
                fall_detected = 1'b1; // pulso único
                next_state = IDLE;
            end
        endcase
    end

endmodule


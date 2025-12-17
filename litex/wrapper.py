from migen import *
from litex.soc.interconnect.csr import *

class FallDetectWrapper(Module, AutoCSR):
    def __init__(self, platform, clk_freq):
        # ---------------------------------------------------------
        # 1. Registradores para o C ESCREVER (Inputs do Hardware)
        # ---------------------------------------------------------
        # O processador vai escrever os valores brutos aqui
        self._ax = CSRStorage(16, description="Acelerometro X")
        self._ay = CSRStorage(16, description="Acelerometro Y")
        self._az = CSRStorage(16, description="Acelerometro Z")
        
        # Um bit para avisar o hardware: "Ei, dados novos chegaram!"
        self._valid = CSRStorage(1, description="Data Valid Pulse")

        # Configurações (Thresholds) - O C configura isso no boot
        self._impact_thresh = CSRStorage(32, description="Impact Threshold Sq", reset=1073741824)
        self._still_thresh  = CSRStorage(32, description="Still Threshold Sq", reset=20000000)

        # ---------------------------------------------------------
        # 2. Registradores para o C LER (Outputs do Hardware)
        # ---------------------------------------------------------
        # O hardware escreve aqui se detectou queda, o C lê
        self._detected = CSRStatus(1, description="Fall Detected Status")

        # ---------------------------------------------------------
        # 3. Conexão com o SystemVerilog
        # ---------------------------------------------------------
        platform.add_source("sv/fall_detect.sv") # O caminho da sua imagem

        self.specials += Instance("fall_detect",
            p_CLK_FREQ_HZ = clk_freq,
            p_STILL_TIME_MS = 2000,

            i_clk   = ClockSignal(),
            i_rst_n = ~ResetSignal(),

            # Conectando os CSRs (software) às entradas do módulo (hardware)
            # .storage acessa o valor físico que o C escreveu
            i_ax = self._ax.storage,
            i_ay = self._ay.storage,
            i_az = self._az.storage,
            i_data_valid = self._valid.storage, 
            
            i_impact_thresh_sq = self._impact_thresh.storage,
            i_still_thresh_sq  = self._still_thresh.storage,

            # A saída do módulo vai para o Status Register
            o_fall_detected = self._detected.status
        )
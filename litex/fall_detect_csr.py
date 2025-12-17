from migen import *
from litex.soc.interconnect.csr import AutoCSR, CSRStorage, CSRStatus

class FallDetectCSR(Module, AutoCSR):
    def __init__(self):
        # ---------------- CSRs ----------------
        self.ax = CSRStorage(16, name="ax", description="Accel X")
        self.ay = CSRStorage(16, name="ay", description="Accel Y")
        self.az = CSRStorage(16, name="az", description="Accel Z")

        self.data_valid = CSRStorage(1, name="data_valid",
            description="Pulso: AX/AY/AZ válidos")

        self.mag_sq    = CSRStatus(32, name="mag_sq",
            description="Magnitude ao quadrado")
        self.mag_valid = CSRStatus(1, name="mag_valid",
            description="Resultado válido")

        # ---------------- sinais internos ----------------
        ax_sig        = Signal(16)
        ay_sig        = Signal(16)
        az_sig        = Signal(16)
        data_valid_sig = Signal()

        mag_sq_sig    = Signal(32)
        mag_valid_sig = Signal()

        # ---------------- instância Verilog ----------------
        self.specials += Instance(
            "fall_detect_mag_sq",
            i_clk       = ClockSignal(),
            i_rst_n     = ~ResetSignal(),

            i_data_valid = data_valid_sig,
            i_ax         = ax_sig,
            i_ay         = ay_sig,
            i_az         = az_sig,

            o_mag_sq     = mag_sq_sig,
            o_mag_valid  = mag_valid_sig
        )

        # ---------------- mapeamento CSR <-> sinais ----------------
        self.comb += [
            ax_sig.eq(self.ax.storage),
            ay_sig.eq(self.ay.storage),
            az_sig.eq(self.az.storage),

            data_valid_sig.eq(self.data_valid.storage),

            self.mag_sq.status.eq(mag_sq_sig),
            self.mag_valid.status.eq(mag_valid_sig)
        ]


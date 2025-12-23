# Projeto Final – Detecção de Queda Baseada em Hardware com FPGA

Projeto final da disciplina Embarcatech – FPGA

## Descrição do Projeto

Este projeto implementa um sistema de detecção de quedas baseado em hardware, utilizando uma FPGA ColorLight i9 com um SoC customizado gerado via LiteX.

O sistema realiza o processamento de dados de aceleração diretamente em hardware, permitindo a identificação rápida de eventos de queda, reduzindo latência e dependência de processamento em software.

A arquitetura combina:

- Lógica dedicada em hardware (SystemVerilog) para detecção de padrões de queda
- SoC LiteX com processador softcore
- Firmware bare-metal em C para inicialização e controle
- Estrutura modular, facilitando testes e expansões futuras

O foco do projeto é demonstrar o uso de FPGAs para detecção de eventos críticos em tempo real, explorando paralelismo, determinismo temporal e baixa latência.

---

## Estrutura do Projeto

hardware-based-fpga-fall-detection/

```
├── build/
│   └── colorlight_i5/
│       └── gateware e bitstream gerados
│
├── bitdog/
|   ├── .gitignore
|   ├── pico_sdk_import.cmake
│   ├── bitdog.c
│   └── CMakeLists.txt
|
├── firmware/ (Feito pra usar o módulo em SystemVerilog)
│   ├── main.c
|   ├── lib/
│   └── Makefile
|
├── firmware2/ (Feito para executar o algorítimo de detecção de queda em firmware)
│   ├── main.c
|   ├── lib/
│   └── Makefile
│
├── litex/
|   ├── fall_detect_csr.py
│   └── colorlight_i5.py
│
├── sv/
|   ├── tb_fall_detect.sv
│   └── fall_detect.sv
│
└── README.txt
```
---

## Funcionamento do Sistema

### FPGA (ColorLight i9)

Função:
Plataforma de processamento e detecção de quedas.

Componentes principais:

- SoC LiteX
  - Processador softcore (PicoRV32 ou VexRiscv)
  - Barramento Wishbone
- Módulo de Detecção de Queda em Hardware
  - Implementado em SystemVerilog
  - Processamento paralelo dos dados de aceleração
- Firmware bare-metal
  - Inicialização do sistema
  - Comunicação com o módulo de hardware
  - Exibição de resultados via terminal serial

---

### Fluxo de Operação

1. Inicialização do SoC e periféricos.
2. Entrada de dados de aceleração (reais ou simulados).
3. O módulo de hardware analisa:
   - Variações abruptas de aceleração
   - Limiares (thresholds) configurados
4. Um evento de queda é detectado em hardware.
5. O resultado é disponibilizado ao firmware.
6. O firmware registra ou exibe o evento detectado.

---

## Módulo de Detecção de Queda (Hardware)

O módulo de detecção foi projetado para:

- Operação determinística
- Baixa latência
- Execução independente do processador
- Fácil integração com o SoC LiteX

Principais características:

- Processamento em paralelo
- Lógica combinacional e sequencial
- Interface simples com o firmware
- Possibilidade de ajuste de parâmetros

---

## Tecnologias Utilizadas

### Hardware / HDL
- FPGA ColorLight i9
- SystemVerilog

### SoC e Ferramentas
- LiteX
- OSS CAD Suite
- openFPGALoader

### Software
- C (bare-metal)
- Python (descrição do SoC)

---

## Instruções de Compilação e Execução

### Preparação do Ambiente

source caminho/para/oss-cad-suite/environment

---

### Geração do SoC com LiteX

python3 litex/colorlight_i5.py --board i9 --revision 7.2 --build --cpu-type=vexriscv --ecppack-compress

---

### Compilação do Firmware

cd firmware
make
cd ..

---

### Gravação do Bitstream na FPGA

openFPGALoader -b colorlight-i5 build/colorlight_i5/gateware/colorlight_i5.bit

---

### Execução do Firmware

litex_term /dev/ttyACM0 --speed 115200 --kernel firmware/main.bin

Após iniciar o terminal:
- Pressione Enter
- Digite reboot

---

## Objetivos Alcançados

- Implementação de detecção de queda em hardware
- Integração de SystemVerilog ao SoC LiteX
- Firmware bare-metal funcional
- Arquitetura modular
- Redução de latência em comparação a software

---

## Melhorias Futuras e Perspectivas

- Atuação Física: Implementação de um sistema de airbag acoplado para proteção imediata.
- Conectividade: Otimização da taxa de transmissão de dados via tecnologia LoRa.
- Portabilidade: Redução das dimensões do dispositivo (compactação) e implementação de alimentação via bateria dedicada, eliminando a dependência atual do cabo USB.

---

## Autores do Projeto

João Emanuel Cândido Gonçalves da Silva | 202521511890004

Igor Gabriel Dantas Rocha | 202521511890009

PACKAGES=libc libcompiler_rt libbase libfatfs liblitespi liblitedram libliteeth liblitesdcard liblitesata bios
PACKAGE_DIRS=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libc /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libcompiler_rt /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libbase /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libfatfs /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitespi /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitedram /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libliteeth /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitesdcard /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitesata /home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/bios
LIBS=libc libcompiler_rt libbase libfatfs liblitespi liblitedram libliteeth liblitesdcard liblitesata
TRIPLE=riscv-none-elf
CPU=vexriscv
CPUFAMILY=riscv
CPUFLAGS=-march=rv32i2p0_m     -mabi=ilp32 -D__vexriscv__
CPUENDIANNESS=little
CLANG=0
CPU_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/cores/cpu/vexriscv
SOC_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc
PICOLIBC_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/pythondata-software-picolibc/pythondata_software_picolibc/data
PICOLIBC_FORMAT=integer
COMPILER_RT_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/pythondata-software-compiler_rt/pythondata_software_compiler_rt/data
export BUILDINC_DIRECTORY
BUILDINC_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/fall_detection/build/colorlight_i5/software/include
LIBC_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libc
LIBCOMPILER_RT_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libcompiler_rt
LIBBASE_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libbase
LIBFATFS_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libfatfs
LIBLITESPI_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitespi
LIBLITEDRAM_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitedram
LIBLITEETH_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/libliteeth
LIBLITESDCARD_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitesdcard
LIBLITESATA_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/liblitesata
BIOS_DIRECTORY=/home/igor/igor/prog/embarcatech/fpga/litex/litex/soc/software/bios
LTO=0
BIOS_CONSOLE_FULL=1
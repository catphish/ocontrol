set -e
rm -f *.o
REPOROOT="/home/charlie/STM32Cube/Repository/STM32Cube_FW_L0_V1.10.0"
CCOPTS="-Wall -mcpu=cortex-m0 -mthumb -I$REPOROOT/Drivers/CMSIS/Device/ST/STM32L0xx/Include -I$REPOROOT/Drivers/CMSIS/Include -DSTM32L0xx -O2 -ffast-math"
arm-none-eabi-gcc $CCOPTS -c startup_stm32l031xx.s -o startup_stm32l031xx.o
arm-none-eabi-gcc $CCOPTS -c system.c -o system.o
arm-none-eabi-gcc $CCOPTS -c main.c -o main.o
#arm-none-eabi-gcc $CCOPTS -S -c main.c -o main.s
arm-none-eabi-gcc $CCOPTS -T STM32L031K6Tx_FLASH.ld -Wl,--gc-sections *.o -o main.elf -lm
arm-none-eabi-objcopy -O binary main.elf main.bin
#st-flash write main.bin 0x8000000

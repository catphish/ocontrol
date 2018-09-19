set -e
REPOROOT="/home/charlie/STM32Cube/Repository/STM32Cube_FW_L0_V1.10.0"
CCOPTS="-Wall -mcpu=cortex-m0 -mthumb -I$REPOROOT/Drivers/CMSIS/Device/ST/STM32L0xx/Include -I$REPOROOT/Drivers/CMSIS/Include -DSTM32L0xx -O3 -ffast-math"
arm-none-eabi-gcc $CCOPTS -T STM32L031K6Tx_FLASH.ld -Wl,--gc-sections *.c *.s -o main.elf -lm
arm-none-eabi-objcopy -O binary main.elf main.bin
#st-flash write main.bin 0x8000000

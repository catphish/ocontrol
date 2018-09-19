#include <stm32l031xx.h>

// At 2MHz, each systick is 0.5us, we can time up to about 8 seconds
void usleep(unsigned int delay) {
  SysTick->LOAD = 0x00FFFFFF;
  SysTick->CTRL = 5;
  SysTick->VAL = 0;
  while(0x00FFFFFF - SysTick->VAL < delay * 2);
}

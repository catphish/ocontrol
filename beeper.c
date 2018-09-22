#include "gpio.h"
#include <stm32l031xx.h>

void beeper_init() {
  set_port_af(GPIOB, 6, 5);  // AF5 for timer output on B6
  set_port_mode(GPIOB, 6,  PORT_MODE_AF); // TIM21_CH1
  TIM21->CR1 = (1<<7) | 1;
  TIM21->CCMR1 = (6<<4)|(1<<3); // OC1M_PWM1
  TIM21->CCER = 0; // CC1E
  TIM21->CCR1 = 250;
  TIM21->ARR = 500;
}

void beep_on() {
  TIM21->CCER = 1;
}

void beep_off() {
  TIM21->CCER = 0;
}

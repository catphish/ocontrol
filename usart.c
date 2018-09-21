#include "gpio.h"
#include <stm32l031xx.h>

void usart_init() {
  set_port_af(GPIOA, 9, 4);  // AF4 for usart
  set_port_af(GPIOA, 10, 4); // AF4 for usart
  set_port_mode(GPIOA, 10, PORT_MODE_AF); // USART2
  set_port_mode(GPIOA, 9,  PORT_MODE_AF); // USART2
  USART2->BRR = 218;
  USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void usart_write_char(unsigned char c) {
  while(!(USART2->ISR & (1<<7)));
  USART2->TDR = c;
}

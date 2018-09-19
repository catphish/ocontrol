#include "gpio.h"
#include <stm32l031xx.h>

void gps_init() {
  // LPUART + GPS
  gpio_out(GPIOA, 1, 1); // Idle high to disable GPS
  set_port_af(GPIOA, 2, 6);  // AF6 for lpuart
  set_port_af(GPIOA, 3, 6);  // AF6 for lpuart
  set_port_mode(GPIOA, 3,  PORT_MODE_AF); // LPUART1
  set_port_mode(GPIOA, 2,  PORT_MODE_AF); // LPUART1
  set_port_mode(GPIOA, 1,  PORT_MODE_OUTPUT); // GPS enable GPIO out
  LPUART1->CR1 = (1<<3) | (1<<2) | (1<<0);
  LPUART1->BRR = 0x369;
}
#include "gpio.h"
#include <stm32l031xx.h>

void gpio_init() {
}

void set_port_mode(GPIO_TypeDef * port, unsigned int pin, unsigned int mode) {
  unsigned int val = port->MODER;
  val &= ~(3 << (pin*2));
  val |= (mode << (pin * 2));
  port->MODER = val;
}

void set_port_af(GPIO_TypeDef * port, unsigned int pin, unsigned int af) {
  if(pin > 7) {
    pin -= 8;
    unsigned int val = port->AFR[1];
    val &= ~(0xf<<(pin*4));
    val |= (af << (pin*4));
    port->AFR[1] = val;
  } else {
    unsigned int val = port->AFR[0];
    val &= ~(0xf<<(pin*4));
    val |= (af << (pin*4));
    port->AFR[0] = val;
  }
}

void gpio_out(GPIO_TypeDef * port, unsigned int pin, unsigned int value) {
  if(value) {
    port->BSRR = (1<<pin);
  } else {
    port->BRR  = (1<<pin);
  }
}

void led_init(){
  set_port_mode(GPIOB, 5,  PORT_MODE_OUTPUT); // LED output
}

void led_on() {
  gpio_out(GPIOB, 5, 1);
}

void led_off() {
  gpio_out(GPIOB, 5, 0);
}

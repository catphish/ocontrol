#include "gpio.h"
#include "usart.h"
#include <string.h>
#include <stm32l031xx.h>

char buffer[128];
unsigned char buffer_ptr;
unsigned char buffer_valid;

void gps_init() {
  // LPUART + GPS
  gpio_out(GPIOA, 1, 1); // Idle high to disable GPS
  set_port_af(GPIOA, 2, 6);  // AF6 for lpuart
  set_port_af(GPIOA, 3, 6);  // AF6 for lpuart
  set_port_mode(GPIOA, 3,  PORT_MODE_AF); // LPUART1
  set_port_mode(GPIOA, 2,  PORT_MODE_AF); // LPUART1
  set_port_mode(GPIOA, 1,  PORT_MODE_OUTPUT); // GPS enable GPIO out
  LPUART1->CR1 = (1<<5) | (1<<2) | (1<<0); // RX interrupt, RX enable, lpuart enable
  LPUART1->BRR = 0x369; // 9600 baud
  NVIC->ISER[0] = (1 << LPUART1_IRQn);
}

void gps_on() {
  GPIOA->BRR = (1<<1); // A1 low, GPS on
}

void gps_off() {
  GPIOA->BSRR = (1<<1); // A1 high, GPS off
}

void LPUART1_IRQHandler() {
  LPUART1->ICR = 1 << 3;
  char rxchar = LPUART1->RDR;
  if(rxchar == '$') {
    buffer_valid = 1;
    buffer_ptr = 0;
    memset(buffer, 0, 128);
  } else if((rxchar == '\n') && buffer_valid) {
    // We have a line
    if(!strncmp(buffer, "GPRMC,", 6)) {
      usart_write_char(0x2);
      // this is the line we want
      if(buffer[17] == 'A') {
        // Data is valid
        RTC->ISR |= (1<<7);
        while(!(RTC->ISR & (1<<6)));
        RTC->TR = (buffer[6]  - '0') << 20 |
                  (buffer[7]  - '0') << 16 |
                  (buffer[8]  - '0') << 12 |
                  (buffer[9]  - '0') << 8  |
                  (buffer[10] - '0') << 4  |
                  (buffer[11] - '0') << 0;
        RTC->ISR &= ~(1<<7);
        gps_off();
      }
      buffer_valid = 0;
    }
  } else if(buffer_valid && buffer_ptr < 128) {
    buffer[buffer_ptr] = rxchar;
    buffer_ptr++;
  }
}
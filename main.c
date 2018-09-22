//#include <stdint.h>
#include <stm32l031xx.h>
#include "util.h"
#include "gpio.h"
#include "usart.h"
#include "st95hf.h"
#include "gps.h"
#include "beeper.h"

void SystemInitError(uint8_t error_source) {
  while(1);
}

void SystemInit() {
  // Power everything up, configure clocks, general pre-initialization
  RCC->APB1ENR |= (1<<28); // Enable access to power registers
  PWR->CR      |= (1<<8);  // Enable access to RTC registers
  RTC->WPR      = 0xCA;    // I think this is also needed
  RTC->WPR      = 0x53;    // I think this is also needed
  RCC->IOPENR  |= 3;       // Enable PORT A+B
  RCC->APB1ENR |= (1<<17); // Enable USART2
  RCC->APB1ENR |= (1<<18); // Enable LPUART1
  RCC->APB2ENR |= (1<<12); // Enable SPI1
  RCC->APB2ENR |= (1<<2);  // Enable TIM21
  RCC->CSR     |= (1<<8);  // LSE ON
  RCC->CSR     |= (1<<18); // RTC ON
  RCC->CSR     |= (1<<16); // RTC using LSE
  RCC->CCIPR   |= (3<<10); // LSE for LPUART
  usleep(1000);
}

int main() {
  unsigned char buffer[256];

  // Zero the RTC
  // RTC->ISR |= (1<<7);
  // while(!(RTC->ISR & (1<<6)));
  // RTC->TR = 0;
  // RTC->ISR &= ~(1<<7);

  // Configure all peripherals
  beeper_init();
  led_init();
  gps_init();
  st95hf_init();
  usart_init();

  gps_on();

  wakeup_pulse();

  // Set mode
  spi_tx_string((char[]){0,2,2,2,0}, 5);
  read_response(buffer);

  // Set params
  spi_tx_string((char[]){0,9,4,0x3a,0,0x58,4}, 7);
  read_response(buffer);
  spi_tx_string((char[]){0,9,4,0x68,0,1,0xd1}, 7);
  read_response(buffer);

  while(1) {
    spi_tx_string((char[]){0,4,2,0x26,7}, 5);
    unsigned char response = read_response(buffer);
 
    if(response == 0x80 && buffer[0] == 5 && buffer[1] == 0x44 && buffer[2] == 0) {
      //wait_a_bit();
      spi_tx_string((char[]){0,4,3,0x30,0,0x28}, 6);
      unsigned char response = read_response(buffer);
   
      usart_write_char(buffer[0]);
      usart_write_char(buffer[1]);
      usart_write_char(buffer[2]);
      usart_write_char(buffer[3]);
      usart_write_char(buffer[4]);
      usart_write_char(buffer[5]);
      usart_write_char(buffer[6]);
      usart_write_char(buffer[7]);
      usart_write_char(0xff);
      if(response == 0x80 && buffer[0] == 0x15) {
        led_on();
        //TIM21->CCER = 1; // Beep on
        usleep(50000); // 50ms on
        led_off(); // Blink an LED
        //TIM21->CCER = 0; // Beep off
        usleep(100000); // 100ms off
      }
    }
  }


}

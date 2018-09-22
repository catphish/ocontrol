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
  usleep(100000);

  // Set mode
  //wait_a_bit();
  ss_low();
  spi_tx(0);
  spi_tx(2);
  spi_tx(2);
  spi_tx(2);
  spi_tx(0);
  ss_high();
  read_response(buffer);

  // Set params
  //wait_a_bit();
  ss_low();
  spi_tx(0);
  spi_tx(9);
  spi_tx(4);
  spi_tx(0x3a);
  spi_tx(0);
  spi_tx(0x58);
  spi_tx(4);
  ss_high();
  read_response(buffer);

  //wait_a_bit();
  ss_low();
  spi_tx(0);
  spi_tx(9);
  spi_tx(4);
  spi_tx(0x68);
  spi_tx(1);
  spi_tx(1);
  spi_tx(0xd1);
  ss_high();
  read_response(buffer);

  while(1) {
    //wait_a_bit();
    ss_low();
    spi_tx(0);
    spi_tx(4);
    spi_tx(2);
    spi_tx(0x26);
    spi_tx(0x07);
    ss_high();
    unsigned char response = read_response(buffer);
 
    if(response == 0x80 && buffer[0] == 5 && buffer[1] == 0x44 && buffer[2] == 0) {
      //wait_a_bit();
      ss_low();
      spi_tx(0);
      spi_tx(4);
      spi_tx(3);
      spi_tx(0x30);
      spi_tx(0x0);
      spi_tx(0x28);
      ss_high();
      unsigned char response = read_response(buffer);
   
      usart_write_char(buffer[1]);
      usart_write_char(buffer[2]);
      usart_write_char(buffer[3]);
      usart_write_char(buffer[5]);
      usart_write_char(buffer[6]);
      usart_write_char(buffer[7]);
      usart_write_char(buffer[8]);
      usart_write_char(0xff);
      if(response == 0x80 && buffer[0] == 0x15) {
        GPIOB->ODR = 0b100000; // Blink an LED
        //TIM21->CCER = 1; // CC1E

        usleep(100000);
        GPIOB->ODR = 0b000000; // Blink an LED
        TIM21->CCER = 0; // CC1E
        usleep(100000);
      }
    }
  }


}

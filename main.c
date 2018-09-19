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
  // Power everything up
  RCC->APB1ENR |= (1<<28); // Enable access to power registers
  PWR->CR      |= (1<<8);  // Enable access to RTC registers
  // RTC->WPR      = 0xCA;
  // RTC->WPR      = 0x53;
  RCC->IOPENR  |= 3;       // Enable PORT A+B
  RCC->APB2ENR |= (1<<12); // Enable SPI1
  RCC->APB2ENR |= (1<<2);  // Enable TIM21
  RCC->CSR     |= (1<<8);  // LSE ON
  RCC->CCIPR   |= (3<<10); // LSE for LPUART

  // Wait for everything to wake up
  usleep(1000);

  // Configure all peripherals
  beeper_init();
  led_init();
  gps_init();
  st95hf_init();
  usart_init();

}

int main() {
  // while(1) {
  //   usleep(500000);
  //   GPIOB->BSRR = (1<<5); // LED on
  //   usleep(500000);
  //   GPIOB->BRR = (1<<5);  // LED off
  // }

  GPIOA->BRR = (1<<1); // A1 low, GPS on
  while(1) {
    while(!(LPUART1->ISR & (1<<5)));
    while(!(USART2->ISR & (1<<7)));
    USART2->TDR = LPUART1->RDR;
  }

  // unsigned char buffer[256];

  // // Try to wake the ST95HF
  // irq_pulse();

  // // Set mode
  // //wait_a_bit();
  // ss_low();
  // spi_tx(0);
  // spi_tx(2);
  // spi_tx(2);
  // spi_tx(2);
  // spi_tx(0);
  // ss_high();
  // read_response(buffer);

  // // Set params
  // //wait_a_bit();
  // ss_low();
  // spi_tx(0);
  // spi_tx(9);
  // spi_tx(4);
  // spi_tx(0x3a);
  // spi_tx(0);
  // spi_tx(0x58);
  // spi_tx(4);
  // ss_high();
  // read_response(buffer);

  // //wait_a_bit();
  // ss_low();
  // spi_tx(0);
  // spi_tx(9);
  // spi_tx(4);
  // spi_tx(0x68);
  // spi_tx(1);
  // spi_tx(1);
  // spi_tx(0xd1);
  // ss_high();
  // read_response(buffer);

  // while(1) {
  //   //wait_a_bit();
  //   ss_low();
  //   spi_tx(0);
  //   spi_tx(4);
  //   spi_tx(2);
  //   spi_tx(0x26);
  //   spi_tx(0x07);
  //   ss_high();
  //   unsigned char response = read_response(buffer);
 
  //   if(response == 0x80 && buffer[0] == 5 && buffer[1] == 0x44 && buffer[2] == 0) {
  //     //wait_a_bit();
  //     ss_low();
  //     spi_tx(0);
  //     spi_tx(4);
  //     spi_tx(3);
  //     spi_tx(0x30);
  //     spi_tx(0x0);
  //     spi_tx(0x28);
  //     ss_high();
  //     unsigned char response = read_response(buffer);
   
  //     serial_write_char(buffer[1]);
  //     serial_write_char(buffer[2]);
  //     serial_write_char(buffer[3]);
  //     serial_write_char(buffer[5]);
  //     serial_write_char(buffer[6]);
  //     serial_write_char(buffer[7]);
  //     serial_write_char(buffer[8]);
  //     serial_write_char(0xff);

  //     if(response == 0x80 && buffer[0] == 0x15) {
  //       GPIOB->ODR = 0b100000; // Blink an LED
  //       //TIM21->CCER = 1; // CC1E
  //       wait_a_bit();
  //       GPIOB->ODR = 0b000000; // Blink an LED
  //       //TIM21->CCER = 0; // CC1E
  //       wait_a_bit();
  //     }
  //   }
  // }

}

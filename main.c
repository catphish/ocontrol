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

  // Configure all peripherals
  beeper_init();
  led_init();
  gps_init();
  st95hf_init();
  usart_init();

  gps_on();

  wakeup_pulse();

  calibrate();

  // Set params
  // spi_tx_string((char[]){0,9,4,0x3a,0,0x58,4}, 7);
  // read_response(buffer);
  // spi_tx_string((char[]){0,9,4,0x68,0,1,0xd3}, 7);
  // read_response(buffer);

  while(1) {
    detect_tag();

    // Set mode
    spi_tx_string((char[]){0,2,2,2,0}, 5);
    read_response(buffer);

    usleep(1000);

    // Wake the tag
    spi_tx_string((char[]){0,4,2, 0x26,7}, 5);
    unsigned char response = read_response(buffer);

    // Check the tag type
    if(response == 0x80 && buffer[0] == 5 && buffer[1] == 0x44 && buffer[2] == 0) {

      int ht = (RTC->TR & (0x3<<20)) >> 20;
      int hu = (RTC->TR & (0xf<<16)) >> 16;
      int h = ht*10+hu;
      int mt = (RTC->TR & (0x7<<12)) >> 12;
      int mu = (RTC->TR & (0xf<<8 )) >> 8;
      int m = mt*10+mu;
      int st = (RTC->TR & (0x7<<4 )) >> 4;
      int su = (RTC->TR & (0xf<<0 )) >> 0;
      int s = st*10+su;
      
      int control = 1;

      // Read block 0, the tag documentation says to do so
      spi_tx_string((char[]){0,4,3, 0x30,0,0x28}, 6);
      response = read_response(buffer);

      // Write to block 4
      spi_tx_string((char[]){0,4,7, 0xa2,4, control,h,m,s, 0x28}, 10);
      response = read_response(buffer);
      usleep(10000); // Wait for the write to finish

      // Read block 4 to verify the write
      spi_tx_string((char[]){0,4,3, 0x30,4,0x28}, 6);
      response = read_response(buffer);
 
      // Power off field
      spi_tx_string((char[]){0,2,2,0,0}, 5);
      read_response(buffer);

      if(buffer[1] == control && buffer[2] == h && buffer[3] == m && buffer[4] == s) {
        led_on();
        //beep_on();
        usleep(50000); // 50ms on

        led_off();
        //beep_off();
        usleep(1000000); // 2s off
      }
    }
  }

}

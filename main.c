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
  RCC->APB2ENR |= (1<<0);  // Enable SYSCFG
  RCC->CSR     |= (1<<8);  // LSE ON
  RCC->CSR     |= (1<<18); // RTC ON
  RCC->CSR     |= (1<<16); // RTC using LSE
  RCC->CCIPR   |= (3<<10); // LSE for LPUART
  usleep(1000);
}

int main() {
  // Configure all peripherals
  beeper_init();
  led_init();
  gps_init();
  st95hf_init();
  usart_init();

  // Blink LED to indicate succssful boot
  led_on();
  usleep(50000);
  led_off();

  gps_on();

  // Wakeup
  wakeup_pulse();
  // Reset
  spi_tx_string((char[]){1}, 1);
  usleep(10000);
  // Wakeup again
  wakeup_pulse();

  calibrate();

  detect_tag(); // This runs forever using interrupts

  // Low power main loop
  while(1) {
    asm("wfi");
  }

}

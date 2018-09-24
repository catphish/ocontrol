#include "gpio.h"
#include <stm32l031xx.h>
#include "util.h"
#include "usart.h"

void st95hf_init() {
  gpio_out(GPIOA, 4, 1); // NFC wakeup idles high
  gpio_out(GPIOB, 0, 1); // SPI SS should idle high
  set_port_mode(GPIOA, 4,  PORT_MODE_OUTPUT); // NFC wakeup GPIO output
  set_port_mode(GPIOB, 1,  PORT_MODE_INPUT);  // NFC IRQ GPIO input
  set_port_mode(GPIOA, 7,  PORT_MODE_AF);     // SPI1
  set_port_mode(GPIOA, 6,  PORT_MODE_AF);     // SPI1
  set_port_mode(GPIOA, 5,  PORT_MODE_AF);     // SPI1
  set_port_mode(GPIOB, 0,  PORT_MODE_OUTPUT); // SPI SS using GPIO out

  SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | (1<<3); // Baud rate 2MHz/?
  SPI1->CR1 |= SPI_CR1_SPE;

  SYSCFG->EXTICR[0] |= (1<<4);
  EXTI->FTSR |= 2;
  NVIC->ISER[0] = (1 << EXTI0_1_IRQn);
}

void ss_low() {
  gpio_out(GPIOB, 0, 0);
}

void ss_high() {
  gpio_out(GPIOB, 0, 1);
}

void wakeup_pulse(){
  gpio_out(GPIOA, 4, 0);
  usleep(20000);
  gpio_out(GPIOA, 4, 1);
  usleep(20000);
}

unsigned char spi_tx(unsigned char tx) {
  while(!(SPI1->SR & 2));
  SPI1->DR = tx;
  while(!(SPI1->SR & 1));
  return(SPI1->DR);
}

void spi_tx_string(char* string, int length) {
  ss_low();
  for(int n=0;n<length;n++) {
    spi_tx(string[n]);
  }
  ss_high();
}

unsigned char read_response(unsigned char* buffer) {
  // Wait for interrupt line
  while(GPIOB->IDR & (1<<1));

  //  Read data
  ss_low();
  spi_tx(2);
  unsigned char response = spi_tx(0);
  buffer[0] = spi_tx(0);
  for(int n=1; n<=buffer[0];n++)
    buffer[n] = spi_tx(0);
  ss_high();

  return response;
}

int calibration;
void calibrate() {
  unsigned char buffer[16];
  for(calibration=0x50;calibration<0xfc;calibration++) {
    spi_tx_string((char[]){0,7,14, 11, 0xa2,0, 0xf8,1, 0x18,0, 0x10, 0x60,0x60, 0,calibration, 0x3f, 1}, 17);
    read_response(buffer);
    if (buffer[1] == 1) {
      // Blink LED to indicate successful calibration
      led_on();
      usleep(50000);
      led_off();
      usleep(100000);
      led_on();
      usleep(50000);
      led_off();
      usleep(100000);
      led_on();
      usleep(50000);
      led_off();
      return;
    }
  }
}

void detect_tag() {
  // Enable interrupt
  EXTI->IMR |= 2;
  // Enter tag detect sleep
  spi_tx_string((char[]){0,7,14, 10, 0x21,0, 0x79,1, 0x18,0, 0x10, 0x60,0x60, calibration-1,calibration+1, 0x3f, 0}, 17);
}

void EXTI0_1_IRQHandler() {
  // Disable interrupt
  EXTI->IMR &= ~(2);
  // Acknowledge interrupt
  EXTI->PR = 2;

  usart_write_char(0x1);

  // Read response
  unsigned char buffer[16];
  read_response(buffer);

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
    
    int id = 1;

    // Read block 0, the tag documentation says to do so
    spi_tx_string((char[]){0,4,3, 0x30,0,0x28}, 6);
    response = read_response(buffer);

    // Write to block 4
    spi_tx_string((char[]){0,4,7, 0xa2,4, id,h,m,s, 0x28}, 10);
    response = read_response(buffer);
    usleep(10000); // Wait for the write to finish

    // Read block 4 to verify the write
    spi_tx_string((char[]){0,4,3, 0x30,4,0x28}, 6);
    response = read_response(buffer);

    // Power off field
    spi_tx_string((char[]){0,2,2,0,0}, 5);
    read_response(buffer);

    if(buffer[1] == id && buffer[2] == h && buffer[3] == m && buffer[4] == s) {
      led_on();
      //beep_on();
      usleep(50000); // 50ms on

      led_off();
      //beep_off();
      usleep(1000000); // 2s off
    }
  }
  detect_tag();
}
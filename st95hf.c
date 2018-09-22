#include "gpio.h"
#include <stm32l031xx.h>
#include "util.h"

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

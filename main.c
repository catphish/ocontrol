#include <stdint.h>
#include <stm32l031xx.h>

#define nop()  __asm__ __volatile__ ("nop" ::)

void wait_a_bit() {
  for(int n=0; n<100000; n++) nop();
}

void ss_low() {
  GPIOA->BRR = (1<<4); // A4 low
}

void ss_high() {
  GPIOA->BSRR = (1<<4);  // A4 high
}

void irq_pulse(){
  wait_a_bit();
  GPIOA->BRR = (1<<3); // A3 low
  wait_a_bit();
  GPIOA->BSRR = (1<<3);  // A3 high
  wait_a_bit();
}

unsigned char spi_tx(unsigned char tx) {
  while(!(SPI1->SR & 2));
  SPI1->DR = tx;
  while(!(SPI1->SR & 1));
  return(SPI1->DR);
}

int main() {
  while(1) {
    // Try to wake the ST95HF
    irq_pulse();

    // Request hardware info
    ss_low();
    wait_a_bit();
    spi_tx(0);
    spi_tx(1);
    spi_tx(0);
    wait_a_bit();
    ss_high();

    wait_a_bit();

    // Read flags
    ss_low();
    wait_a_bit();
    USART2->TDR = GPIOA->IDR; // This shows the MISO IRQ line remains high
    USART2->TDR = spi_tx(3); // Always 0
    USART2->TDR = spi_tx(3); // Always 0
    USART2->TDR = spi_tx(3); // Always 0
    ss_high();

    GPIOB->ODR = 0b100000; // Blink an LED
    wait_a_bit();

    USART2->TDR = '\n'; // Seems to help with buffering

    GPIOB->ODR = 0b000000; // Blink an LED
    wait_a_bit();
  }
}

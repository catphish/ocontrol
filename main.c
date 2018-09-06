#include <stdint.h>
#include <stm32l031xx.h>

#define nop()  __asm__ __volatile__ ("nop" ::)

void serial_write_char(unsigned char c) {
  while(!(USART2->ISR & (1<<7)));
  USART2->TDR = c;
}

void wait_a_bit() {
  for(int n=0; n<50000; n++) nop();
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

unsigned char read_response(unsigned char* buffer) {
  // Read flags
  //wait_a_bit();
  ss_low();
  while(!(spi_tx(3) & 8));
  ss_high();

  //wait_a_bit();

  //Read data
  ss_low();
  spi_tx(2);
  unsigned char response = spi_tx(0);
  buffer[0] = spi_tx(0);
  for(int n=1; n<=buffer[0];n++)
    buffer[n] = spi_tx(0);
  ss_high();

  return response;
}

int main() {
  unsigned char buffer[256];

  // Try to wake the ST95HF
  irq_pulse();

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
   
      serial_write_char(buffer[1]);
      serial_write_char(buffer[2]);
      serial_write_char(buffer[3]);
      serial_write_char(buffer[5]);
      serial_write_char(buffer[6]);
      serial_write_char(buffer[7]);
      serial_write_char(buffer[8]);
      serial_write_char(0xff);

      if(response == 0x80 && buffer[0] == 0x15) {
        GPIOB->ODR = 0b100000; // Blink an LED
        TIM21->CCER = 1; // CC1E
        wait_a_bit();
        GPIOB->ODR = 0b000000; // Blink an LED
        TIM21->CCER = 0; // CC1E
        wait_a_bit();
      }
    }
  }
}

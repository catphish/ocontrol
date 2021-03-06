#include "gpio.h"
#include <stm32l031xx.h>
#include "util.h"
#include "usart.h"
#include "beeper.h"

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
  unsigned char buffer[32];
  for(calibration=0x10;calibration<0xfc;calibration++) {
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
  spi_tx_string((char[]){0,7,14, 10, 0x21,0, 0x79,1, 0x18,0, 0x10, 0x60,0x60, calibration-8,calibration+8, 0x3f, 0}, 17);
}

void usart_write_buffer(unsigned char * buffer) {
  for(int nn=0; nn<=buffer[0]; nn++){
    usart_write_char(buffer[nn]);
  }
  usart_write_char(0xff);
}

int anticollision(int level) {
  unsigned char buffer[32];
  unsigned char response;
  unsigned char command_id;
  if(level == 1)      command_id = 0x93;
  else if(level == 2) command_id = 0x95;
  else if(level == 3) command_id = 0x97;
  else return -1;

  // Send an ANTICOLLISION
  spi_tx_string((char[]){0,4,3, command_id,0x20, 8}, 6);
  response = read_response(buffer);

  if(response == 0x80 && buffer[0] == 8 && buffer[6] == 0x28) {
    // Received UID with no collision, send SELECT
    spi_tx_string((char[]){0,4,8, command_id, 0x70, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], 0x28}, 11);
    response = read_response(buffer);
    if(response == 0x80) {
      // SELECT was successful, lets look at the SAK
      if(buffer[1] & 0x4) {
        // There's more serial nuber to negotiate, proceed with next level ANTICOLLISION
        return anticollision(level + 1);
      } else {
        if(buffer[1] & 0x20) return 2; // IEC 14443-4 device
        else return 1; // Probably just a memory card
      }
    } else return -1;
  } else return -1;
}

void fail() {
  // Acknowledge
  beeper_on();
  usleep(100000);
  beeper_off();
  usleep(100000);
  beeper_set_time(260);
  beeper_on();
  usleep(100000);
  beeper_off();
  beeper_set_time(250);
}

void write_timestamp() {
  unsigned char buffer[32];
  unsigned char response;

  // Tag is a regular tag. Write the time and control number.
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

  // Write to block 4
  spi_tx_string((char[]){0,4,7, 0xa2,4, id,h,m,s, 0x28}, 10);
  response = read_response(buffer);

  if(response == 0x90 && buffer[1] == 0xa) {
    // Read block 4 to verify the write
    spi_tx_string((char[]){0,4,3, 0x30,4,0x28}, 6);
    response = read_response(buffer);

    if(buffer[1] == id && buffer[2] == h && buffer[3] == m && buffer[4] == s) {
      // Power off field
      spi_tx_string((char[]){0,2,2,0,0}, 5);
      read_response(buffer);

      led_on();
      beeper_on();
      usleep(100000); // 100ms on

      led_off();
      beeper_off();
      usleep(1000000); // 2s off
    } else fail();
  } else fail();
}

void EXTI0_1_IRQHandler() {
  unsigned char buffer[32];
  unsigned char response;

  // Disable interrupt
  EXTI->IMR &= ~(2);
  // Acknowledge interrupt
  EXTI->PR = 2;

  // Read and discard response (from sleep command)
  read_response(buffer);

  // Set mode and timeout (power on RF)
  spi_tx_string((char[]){0,2,4,2,0,1,0x80}, 7);
  read_response(buffer);

  // Wait 10ms for tag to power up
  usleep(10000);

  // Send a REQA begin the handshake
  spi_tx_string((char[]){0,4,2, 0x26,7}, 5);
  response = read_response(buffer);

  if(response == 0x80) {
    // Received ATQA, proceed with anticollision
    response = anticollision(1);
    if (response == 1) {
      // We're talking to a dumb tag
      write_timestamp();
    } else if (response == 2) {
      // We're talking to a smart device
      // Send RATS
      spi_tx_string((char[]){0,4,3, 0xe0, 0x20, 0x28}, 6);
      response = read_response(buffer);
      usart_write_char(response);
      usart_write_buffer(buffer);

      // Send SELECT
      spi_tx_string((char[]){0,4,13, 2, 0,0xa4,4,0, 5, 0xF0,0x2A,0x00,0x67,0xA0, 0, 0x28}, 16);
      response = read_response(buffer);
      usart_write_char(response);
      usart_write_buffer(buffer);

      if(response == 0x80) {
        // Set the clock
        RTC->ISR |= (1<<7);
        while(!(RTC->ISR & (1<<6)));
        int yt = buffer[10] / 10;
        int yu = buffer[10] % 10;
        int mt = buffer[11] / 10;
        int mu = buffer[11] % 10;
        int dt = buffer[12] / 10;
        int du = buffer[12] % 10;

        int wd = buffer[13] / 10;

        int hht = buffer[14] / 10;
        int hhu = buffer[14] % 10;
        int mmt = buffer[15] / 10;
        int mmu = buffer[15] % 10;
        int sst = buffer[16] / 10;
        int ssu = buffer[16] % 10;

        RTC->TR = (hht << 20) | (hhu << 16) | (mmt << 12) | (mmu << 8) | (sst << 4) | (ssu << 0);
        RTC->DR = (yt << 20) | (yu << 16) | (wd << 13) | (mt << 12) | (mu << 8) | (dt << 4) | (du << 0);
        RTC->ISR &= ~(1<<7);

        // Power off field
        spi_tx_string((char[]){0,2,2,0,0}, 5);
        read_response(buffer);

        // Acknowledge
        beeper_on();
        usleep(50000);
        beeper_set_time(240);
        usleep(50000);
        beeper_set_time(250);
        usleep(50000);
        beeper_set_time(240);
        usleep(50000);
        beeper_set_time(250);
        usleep(50000);
        beeper_off();
      } else fail();

    } else fail();
  } else fail();
  // Power off field
  spi_tx_string((char[]){0,2,2,0,0}, 5);
  read_response(buffer);

  // Go back to low power card detection mode
  detect_tag();
}
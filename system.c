#include <stm32l031xx.h>

#define nop()  __asm__ __volatile__ ("nop" ::)

void SystemInitError(uint8_t error_source) {
  while(1);
}

extern volatile uint32_t adc_data[];

void SystemInit() {
  RCC->IOPENR |= 3; // Enable PORT A+B
  RCC->APB1ENR |= (1<<17); // Enable USART2
  RCC->APB2ENR |= (1<<12); // Enable SPI1

  for(int n=0; n<100000; n++) nop();

  // These pin should default high
  GPIOA->BSRR = (1<<3) | (1<<4);

  // Port functions
  GPIOA->OSPEEDR=0xFFFFFFFF; // Very high speed, power is free for now
  GPIOA->MODER = 0xEBEBA940; // Some AF, some input, some output
  GPIOB->MODER = 0xFFFFF7FF;
  GPIOA->AFR[1] = (4<<4) | (4<<8); // AF4 for serial

  // SPI
  SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | (7<<3); // Baud rate 2MHz/256
  SPI1->CR1 |= SPI_CR1_SPE;

  // Serial
  USART2->BRR = 218;
  USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

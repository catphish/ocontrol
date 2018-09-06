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
  RCC->APB2ENR |= (1<<2);  // Enable TIM21

  for(int n=0; n<100000; n++) nop();

  // These pin should default high
  GPIOA->BSRR = (1<<3) | (1<<4);

  // Port functions
  GPIOA->OSPEEDR=0xFFFFFFFF; // Very high speed, power is free for now
  GPIOB->OSPEEDR=0xFFFFFFFF; // Very high speed, power is free for now

  GPIOA->MODER = 0xEBEBA940; // Some AF, some input, some output
  GPIOB->MODER = 0xFFFFE7FF;

  GPIOA->AFR[1] = (4<<4) | (4<<8); // AF4 for serial
  GPIOB->AFR[0] = (5<<24); // AF5 for timer output on B6

  // SPI
  SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | (1<<3); // Baud rate 2MHz/2
  SPI1->CR1 |= SPI_CR1_SPE;

  // Serial
  USART2->BRR = 218;
  USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

  // TIM21
  TIM21->CR1 = (1<<7) | 1;
  TIM21->CCMR1 = (6<<4)|(1<<3); // OC1M_PWM1
  TIM21->CCER = 0; // CC1E
  TIM21->CCR1 = 250;
  TIM21->ARR = 500;
}

#include <stm32l031xx.h>

#define nop()  __asm__ __volatile__ ("nop" ::)

void SystemInitError(uint8_t error_source) {
  while(1);
}

extern volatile uint32_t adc_data[];

void SystemInit() {
  RCC->APB1ENR |= (1<<28); // Enable access to power registers
  PWR->CR      |= (1<<8);  // Enable access to RTC registers
  // RTC->WPR      = 0xCA;
  // RTC->WPR      = 0x53;
  RCC->IOPENR  |= 3;       // Enable PORT A+B
  RCC->APB1ENR |= (1<<17); // Enable USART2
  RCC->APB1ENR |= (1<<18); // Enable LPUART1
  RCC->APB2ENR |= (1<<12); // Enable SPI1
  RCC->APB2ENR |= (1<<2);  // Enable TIM21
  RCC->CSR     |= (1<<8);  // LSE ON
  RCC->CCIPR   |= (3<<10); // LSE for LPUART
  for(int n=0; n<100000; n++) nop();

  // These pin should default high
  GPIOB->BSRR = (1<<0);  // B0 high
  GPIOA->BSRR = (1<<4);  // A4 high

  // Port functions
  GPIOA->OSPEEDR=0xFFFFFFFF; // Very high speed, power is free for now
  GPIOB->OSPEEDR=0xFFFFFFFF; // Very high speed, power is free for now

  GPIOA->MODER = 0xEBEBA9A7; // Some AF, some input, some output
  GPIOB->MODER = 0xFFFFE7F1;

  GPIOA->AFR[0] = (6<<8) | (6<<12); // AF6 for lpuart
  GPIOA->AFR[1] = (4<<4) | (4<<8);  // AF4 for usart
  GPIOB->AFR[0] = (5<<24); // AF5 for timer output on B6

  // SPI
  SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | (1<<3); // Baud rate 2MHz/?
  SPI1->CR1 |= SPI_CR1_SPE;

  // Serial
  USART2->BRR = 218;
  USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

  // LPUART
  LPUART1->CR1 = (1<<3) | (1<<2) | (1<<0);
  LPUART1->BRR = 0x369;

  // TIM21
  TIM21->CR1 = (1<<7) | 1;
  TIM21->CCMR1 = (6<<4)|(1<<3); // OC1M_PWM1
  TIM21->CCER = 0; // CC1E
  TIM21->CCR1 = 250;
  TIM21->ARR = 500;
}

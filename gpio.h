#include <stm32l031xx.h>

#define PORT_MODE_INPUT 0
#define PORT_MODE_OUTPUT 1
#define PORT_MODE_AF 2
#define PORT_MODE_ANALOG 3

void set_port_mode(GPIO_TypeDef * port, unsigned int pin, unsigned int mode);
void set_port_af(GPIO_TypeDef * port, unsigned int pin, unsigned int af);
void gpio_out(GPIO_TypeDef * port, unsigned int pin, unsigned int value);
void led_init();
void led_on();
void led_off();

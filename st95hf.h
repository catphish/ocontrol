void st95hf_init();
void wakeup_pulse();
void spi_tx_string(char* string, int length);
unsigned char read_response(unsigned char* buffer);

void ss_low();
void ss_high();
unsigned char spi_tx(unsigned char tx);
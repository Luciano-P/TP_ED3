

#ifndef _RUTINAS_TP
#define _RUTINAS_TP

void set_tim0_g(void);
void set_tim0_r(void);
void set_tim1(int ms);
void set_ADC(void);
int send_UART_12b(uint16_t);
int send_muestras(uint16_t muestras[], int length);
void enable_UART();
void disable_UART();
void cargar_muestras(uint16_t muestras[]);

#endif

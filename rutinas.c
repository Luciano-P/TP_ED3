

#include "LPC17xx.h"
#include "lpc17xx_uart.h"

/*
 * Toma la conversion de 12 bits del ADC y la divide en 2 bytes para luego enviarla mediante UART0
 */
void send_UART_12b(uint16_t palabra)
{

	uint8_t aux;

	aux = palabra & 0xFF;
	UART_SendByte(LPC_UART0, aux);
	aux = (palabra>>8) & 0xF;
	UART_SendByte(LPC_UART0, aux);

}

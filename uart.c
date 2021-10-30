

#include "LPC17xx.h"
#include "lpc17xx_uart.h"

void conf_UART(void)
{

	UART_CFG_Type uartconf;

	uartconf.Baud_rate = 9600;
	uartconf.Parity = UART_PARITY_ODD;
	uartconf.Databits = UART_DATABIT_8;
	uartconf.Stopbits = UART_STOPBIT_1;

	UART_Init(LPC_UART0, &uartconf);

}

void send_UART(uint8_t x)
{

	UART_SendByte(LPC_UART0, x);

}

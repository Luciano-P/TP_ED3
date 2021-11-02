

#include "LPC17xx.h"
#include "lpc17xx_uart.h"

void conf_UART(void)
{

    LPC_PINCON->PINSEL0  |= 0x1<<4;		//P0.2 = TXD0
	LPC_PINCON->PINSEL0  |= 0x1<<6;		//P0.3 = RXD0
	LPC_PINCON->PINMODE0 |= 0X2<<4;		//No pull-up, no pull-down
	LPC_PINCON->PINMODE0 |= 0X2<<6;		//No pull-up, no pull-down

	UART_CFG_Type UARTconf;
	UARTconf.Baud_rate = 9600;
	UARTconf.Parity    = UART_PARITY_ODD;
	UARTconf.Databits  = UART_DATABIT_8;
	UARTconf.Stopbits  = UART_STOPBIT_1;

	UART_FIFO_CFG_Type UARTFIFOconf;
	UARTFIFOconf.FIFO_ResetRxBuf = DISABLE;
	UARTFIFOconf.FIFO_ResetTxBuf = ENABLE;
	UARTFIFOconf.FIFO_DMAMode    = DISABLE;
	UARTFIFOconf.FIFO_Level      = UART_FIFO_TRGLEV0;
	
	UART_Init(LPC_UART0, &UARTconf);
	UART_FIFOConfigStructInit(&UARTFIFOconf);
    
	UART_FIFOConfig(LPC_UART0, &UARTFIFOconf);

	UART_TxCmd(LPC_UART0, ENABLE);
}

void send_UART(uint8_t x)
{

	UART_SendByte(LPC_UART0, x);

}

void send_UART_12b(uint16_t palabra)
{

	uint8_t aux;

	aux = palabra & 0xFF;
	UART_SendByte(LPC_UART0, aux);
	aux = (palabra>>8) & 0xF;
	UART_SendByte(LPC_UART0, aux);

}

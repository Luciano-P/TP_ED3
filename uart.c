

#include "LPC17xx.h"
#include "lpc17xx_uart.h"

void conf_UART(void)
{

    LPC_PINCON->PINSEL0  |= 0x1<<4;		//P0.2 = TXD0
	LPC_PINCON->PINSEL0  |= 0x1<<6;		//P0.3 = RXD0
	LPC_PINCON->PINMODE0 |= 0X2<<4;		//No pull-up, no pull-down
	LPC_PINCON->PINMODE0 |= 0X2<<6;		//No pull-up, no pull-down

	UART_CFG_Type UARTconf;				
	UARTconf.Baud_rate = 9600;				//Baud_rate = 9600 bits/seg
	UARTconf.Parity    = UART_PARITY_ODD;	//Configurado con bit de paridad par
	UARTconf.Databits  = UART_DATABIT_8;	//Envia de a byte
	UARTconf.Stopbits  = UART_STOPBIT_1;    //Se configura con un bit de stop

	UART_FIFO_CFG_Type UARTFIFOconf;
	UARTFIFOconf.FIFO_ResetRxBuf = DISABLE;				//No se usa el RX no hace falta resetear su FIFO
	UARTFIFOconf.FIFO_ResetTxBuf = ENABLE;				//Reseteamos FIFO de TX
	UARTFIFOconf.FIFO_DMAMode    = DISABLE;             //Se deshabilita el modo DMA
	UARTFIFOconf.FIFO_Level      = UART_FIFO_TRGLEV0;	//No importa ya que solo afecta al RX
	
	UART_Init(LPC_UART0, &UARTconf);					//Inicializa el modulo UART0
	UART_FIFOConfigStructInit(&UARTFIFOconf);			
    
	UART_FIFOConfig(LPC_UART0, &UARTFIFOconf);			//Inicializa FIFO del modulo UART0

	UART_TxCmd(LPC_UART0, ENABLE);						//Habilito transmision
}

void send_UART(uint8_t x)
{

	UART_SendByte(LPC_UART0, x);						//Envio Byte

}

void send_UART_12b(uint16_t palabra)					//Creo que esto deberiamos hacerlo directamente
{														//en el passtrough osea en el Arduino

	uint8_t aux;

	aux = palabra & 0xFF;
	UART_SendByte(LPC_UART0, aux);
	aux = (palabra>>8) & 0xF;
	UART_SendByte(LPC_UART0, aux);

}

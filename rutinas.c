

#include "LPC17xx.h"
#include "lpc17xx_uart.h"

#include "rutinas.h"

#define TIEMPO_MAXIMO (0xFFFFFFFFUL)

/*
 * Toma la conversion de 12 bits del ADC y la divide en 2 bytes para luego enviarla mediante UART0
 * Se bloquea hasta lograr el envio, al menos que supere el tiempo limite
 * Si supero el tiempo se cancela el envio y retorna 1, sino, fue exitoso y retorna 0
 */
int send_UART_12b(uint16_t palabra)
{

	uint8_t aux;
	int timeout;

	aux = palabra & 0xFF;					//Tomo los primeros 8 bits para enviarlos
	timeout = TIEMPO_MAXIMO;				//Espero que haya lugar en el buffer
	while (!(LPC_UART0->LSR & (0x1<<5))) {
		if (timeout == 0) break;
		timeout--;
	}
	if(timeout == 0) return 1;				//Si hubo exceso de tiempo salgo y retorno 1
	UART_SendByte(LPC_UART0, aux);			//Envio el primer byte


	aux = (palabra>>8) & 0xF;				//Tomo los 4 bits restantes para enviarlos
	timeout = TIEMPO_MAXIMO;				//Espero que haya lugar en el buffer
	while (!(LPC_UART0->LSR & (0x1<<5))) {
			if (timeout == 0) break;
			timeout--;
		}
	if(timeout == 0) return 1;				//Si hubo exceso de tiempo salgo y retorno 1
	UART_SendByte(LPC_UART0, aux);			//Envio el segundo byte

	return 0;

}


/*
 * Recorre el array muestras para enviar cada dato mediante UART
 */
void send_muestras(uint16_t muestras[], int length){

	for(int i=0; i<length; i++){
		if(send_UART_12b(muestras[i])){
			//Que hago si no se envio algun dato
		}
	}
}





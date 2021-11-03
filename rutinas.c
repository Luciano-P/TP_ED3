

#include "LPC17xx.h"
#include "lpc17xx_uart.h"

#include "rutinas.h"

#define TIEMPO_MAXIMO_UART (0xFFFFFFFFUL)


/*
 * Este set es para el TIMER0 al momento de grabar
 * Configuro el tim0 para match1 a 8 KHz, lo que me genera un rising edge a 4 KHz por toggleo
 */
void set_tim0_g(void)
{

		LPC_TIM0   -> EMR       |=  (3<<6);      //Controla que pasa con el pin asociado al MATCH, en este caso togglea P1.28 (Salida de MATCH TIMER no GPIO) Pag 509
	    LPC_TIM0   -> MR1        =   62;         //Valor cargado al Match Register 1 del TMR0
	    LPC_TIM0   -> MCR       &= ~(1<<3);      //Deshabilito interrupciones por MR1 PAG 507
	    LPC_TIM0   -> TCR        =   3;          //Habilita al contador y lo pone en Reset PAG 505
	    LPC_TIM0   -> TCR       &= ~(1<<1);      //Saco el TMR0 de Reset

		NVIC_DisableIRQ(TIMER0_IRQn);
}


/*
 * Este set es para el TIMER0 al momento de reproducir
 * Configuro el tim0 para match1 a 4 KHz e interrupciones.
 */
void set_tim0_r(void)
{

	LPC_TIM0   -> EMR       &=  ~(3<<6);     //Desahbilito la salida externa de MR1
	LPC_TIM0   -> MR1        =   124;        //MR1 cargado para generar 4 KHz
	LPC_TIM0   -> MCR        =   (1<<3);     //Genera interrupcion por MR1
	LPC_TIM0   -> IR         =  (1<<1);      //Borro Flag de Interrupcion
	LPC_TIM0   -> TCR        =   3;          //Habilita al contador y lo pone en Reset
	LPC_TIM0   -> TCR       &= ~(1<<1);      //Saco el TMR0 de Reset


	NVIC_EnableIRQ(TIMER0_IRQn);
}


/*
 *
 */
void set_tim1(int ms)
{

	LPC_TIM1	->IR  		|=    (1<<0);			//Bajo el flag de interrupcion
	LPC_TIM1	->MR0  	     =    (ms-1);			//Match a la cantidad de ms pasado por parametro

	LPC_TIM1	->TCR 		|=       0x3;			//Inicio y reseteo del timer
	LPC_TIM1	->TCR 		&= 		~0x2;			//Saco el TMR1 del Reset
}


/*
 * Enciendo el ADC y habilito sus interrupciones
 */
void set_ADC(void)
{

	LPC_ADC -> ADCR    |= (0x1<<21);
	NVIC_EnableIRQ(ADC_IRQn);

}

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
	timeout = TIEMPO_MAXIMO_UART;				//Espero que haya lugar en el buffer
	while (!(LPC_UART0->LSR & (0x1<<5))) {
		if (timeout == 0) break;
		timeout--;
	}
	if(timeout == 0) return 1;				//Si hubo exceso de tiempo salgo y retorno 1
	UART_SendByte(LPC_UART0, aux);			//Envio el primer byte


	aux = (palabra>>8) & 0xF;				//Tomo los 4 bits restantes para enviarlos
	timeout = TIEMPO_MAXIMO_UART;				//Espero que haya lugar en el buffer
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
 * Retorna 1 sel envio fallo y se interrumpio, 0 sino
 */
int send_muestras(uint16_t muestras[], int length){

	for(int i=0; i<length; i++){
		if(send_UART_12b(muestras[i])){
			return 1;
		}
	}
	return 0;
}

void enable_UART()
{
	UART_TxCmd(LPC_UART0, ENABLE);						//Habilito transmision
}

void disable_UART()
{
	UART_TxCmd(LPC_UART0, DISABLE);						//Deshabilito transmision
}





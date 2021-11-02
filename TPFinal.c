
/*
 * TP Final: Placa de audio
 * Integrantes: Luciano Peschiutta, Teo Argañaraz
 */


/*
 * Configuracion de pines de la placa:
 *
 */

#include "LPC17xx.h"

#include "configuraciones.h"

#define MAX_MUESTRAS 16000

uint16_t muestras[MAX_MUESTRAS];		//Espacio de memoria donde se guardaran las muestras del ADC
int nr_muestra;							//Variable encargada de recorrer el array de muestras
int grabando;							//Flag que indica si se esta grabando
int reproduciendo;						//Flag que indica si se esta reproduciendo



int main(void)
{

	nr_muestra = 0;			//Pongo a 0 la variable de conteo de muestras
	conf_gpio();			//Configuro GPIO

	while(1){


	}

    return 0 ;
}


void EINT3_IRQHandler(void)
{
	if(LPC_GPIOINT->IO0IntStatR & 0x1){		//Chequeo si se pulso boton grabar

		if((!grabando) && (!reproduciendo)){

			grabando = 1;					//Levanto el flag de grabando
			LPC_GPIO0->FIOSET = (0x1<<15);	//Enciendo el led que indica grabacion
			nr_muestra = 0;					//Reseteo el contador de muestras
			conf_ADC();						//Configuro ADC
			conf_tim1(4000);				//Configuro Tim1
			conf_tim0_g();					//Configuro Tim0 para grabacion

		}

	LPC_GPIOINT->IO0IntClr |= 0x1;	//Bajo flag de interrupcion

	}
	else if(LPC_GPIOINT->IO0IntStatR & 0x2){	//Chequeo si se pulso boton reproducir

		if((!grabando) && (!reproduciendo)){

			reproduciendo = 1;				//Levanto el flag de reproduccion
			LPC_GPIO0->FIOSET = (0x1<<16);	//Enciendo el led que indica reproduccion
			nr_muestra = 0;					//Reseteo el contador de muestras
			conf_DAC();						//Configuro DAC
			conf_tim1(4000);				//Configuro Tim1
			conf_tim0_r();					//Configuro Tim0 para reproduccion


		}

	LPC_GPIOINT->IO0IntClr |= 0x2;			//Bajo flag de interrupcion

	}
}


void TIMER1_IRQHandler(void)
{
	if(grabando == 1){

		grabando = 0;
		LPC_GPIO0->FIOCLR = (0x1<<15);	//Apago el led que indica grabacion

		LPC_ADC->ADCR &= ~(1<<21);		//Apago el ADC
		LPC_SC->PCONP &= ~(1<<12);		//Quito potencia al ADC

		NVIC_DisableIRQ(ADC_IRQn);		//Deshabilito interrupcion por ADC

		LPC_TIM0->TCR &= ~(0x1);		//Desactivo el Timer0
		LPC_TIM1->TCR &= ~(0x1);		//Desactivo el Timer1

	}else{

		reproduciendo = 0;
		LPC_GPIO0->FIOCLR = (0x1<<16);	//Apago el led que indica reproduccion

		LPC_TIM0->TCR &= ~(0x1);		//Desactivo el Timer0
		LPC_TIM1->TCR &= ~(0x1);		//Desactivo el Timer1

	}


	LPC_TIM1->IR |= 0x1;				//Bajo el flag de interrupcion

}


void TIMER0_IRQHandler(void)
{
	if(nr_muestra < MAX_MUESTRAS){

		LPC_DAC->DACR = (muestras[nr_muestra]<<4) & 0xFFC0;		//Le paso el valor de la muestra
		nr_muestra ++;											//Incremento el contador de muestras
	}

	LPC_TIM0->IR |= (0x1<<1);									//Bajo el flag de interrupcion
}


void ADC_IRQHandler(void)
{

	if(nr_muestra < MAX_MUESTRAS){

		muestras[nr_muestra] = (LPC_ADC->ADDR0>>4) & 0xFFF ;	//Guardo la muestra
		nr_muestra ++;											//Incremento el contador de muestras

	}

}


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






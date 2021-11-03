

/*
 * TP Final: Placa de audio
 * Integrantes: Luciano Peschiutta, Teo Argañaraz
 */


/*
 * Configuracion de pines de la placa:
 *	P0.0 = Entrada grabar
 *	P0.1 = Entrada reproducir
 *	P0.2 = TXD0, salida comunicacion serial
 *	P0.6 = Salida grabando
 *	P0.7 = Salida reproduciendo
 *	P0.8 = Salida enviando
 *	P0.18 = Entrada enviar
 *	P0.23 = ADC0, entrada ADC
 *	P0.26 = AOUT, salida DAC
 */

#include "LPC17xx.h"


#include "configuraciones.h"
#include "rutinas.h"

#define MAX_MUESTRAS 16000

uint16_t muestras[MAX_MUESTRAS];		//Espacio de memoria donde se guardaran las muestras del ADC
int nr_muestra;							//Variable encargada de recorrer el array de muestras
int grabando;							//Flag que indica si se esta grabando
int reproduciendo;						//Flag que indica si se esta reproduciendo
int enviando;							//Flag que indica si se estan enviando datos por UART
int grabacion_disponible;				//Flag que indica si hay disponible una grabacion
int overrun;							//Flag que indica si hubo overrun del ADC
int uart_timeout;						//Flag que indica si hubo timeout en el uart



int main(void)
{

	grabacion_disponible = 0;		//Inicializo los flags
	overrun = 0;
	uart_timeout = 0;
	grabando = 0;
	reproduciendo = 0;
	enviando = 0;

	conf_gpio();					//Configuro GPIO

	while(1){

		if(enviando == 1){

			LPC_GPIO0->FIOSET = (0x1<<8);	//Enciendo el led que indica reproduccion
			conf_UART();
			if(send_muestras(muestras, MAX_MUESTRAS)){	//Realizo el envio de las muestras mientras verifico
				uart_timeout = 1;						//que no falle.
			}
			if(uart_timeout){
				LPC_GPIO0->FIOSET = (0x1<<6);	//Enciendo ademas el led de grabacion en conjunto
			}else{
				LPC_GPIO0->FIOCLR = (0x1<<8);	//Apago el led que indica reproduccion
			}

			enviando = 0;

		}

	}

    return 0 ;
}


void EINT3_IRQHandler(void)
{
	if(LPC_GPIOINT->IO0IntStatR & 0x1){		//Chequeo si se pulso boton grabar

		if(overrun || uart_timeout){
			overrun = 0;
			uart_timeout = 0;
			LPC_GPIO0->FIOCLR = (0x1<<6) | (0x1<<7) | (0x1<<8);	//Apago los 3 leds

		}

		if((!grabando) && (!reproduciendo) && (!enviando)){

			grabando = 1;					//Levanto el flag de grabando
			LPC_GPIO0->FIOSET = (0x1<<6);	//Enciendo el led que indica grabacion
			nr_muestra = 0;					//Reseteo el contador de muestras
			conf_ADC();						//Configuro ADC
			conf_tim1(4000);				//Configuro Tim1
			conf_tim0_g();					//Configuro Tim0 para grabacion

		}

	LPC_GPIOINT->IO0IntClr |= (0x1<<0);	//Bajo flag de interrupcion

	}
	else if(LPC_GPIOINT->IO0IntStatR & 0x2){	//Chequeo si se pulso boton reproducir

		if((!grabando) && (!reproduciendo) && (!enviando) && (!overrun) && (!uart_timeout) && grabacion_disponible){

			reproduciendo = 1;				//Levanto el flag de reproduccion
			LPC_GPIO0->FIOSET = (0x1<<7);	//Enciendo el led que indica reproduccion
			nr_muestra = 0;					//Reseteo el contador de muestras
			conf_DAC();						//Configuro DAC
			conf_tim1(4000);				//Configuro Tim1
			conf_tim0_r();					//Configuro Tim0 para reproduccion


		}

		LPC_GPIOINT->IO0IntClr |= (0x1<<1);	//Bajo flag de interrupcion

	}
	else if(LPC_GPIOINT->IO0IntStatR & (0x1<<18)){	//Chequeo si se pulso boton enviar

		if((!grabando) && (!reproduciendo) && (!enviando) && (!overrun) && (!uart_timeout) && grabacion_disponible){

			enviando = 1;					//Levanto el flag de reproduccion

		}

		LPC_GPIOINT->IO0IntClr |= (0x1<<18);	//Bajo flag de interrupcion
	}
}


void TIMER1_IRQHandler(void)
{
	if(grabando == 1){

		grabando = 0;
		grabacion_disponible = 1;
		LPC_GPIO0->FIOCLR = (0x1<<6);	//Apago el led que indica grabacion

		NVIC_DisableIRQ(ADC_IRQn);		//Deshabilito interrupcion por ADC

		LPC_ADC->ADCR &= ~(1<<21);		//Apago el ADC
		LPC_SC->PCONP &= ~(1<<12);		//Quito potencia al ADC

		LPC_TIM0->TCR &= ~(0x1);		//Desactivo el Timer0
		LPC_TIM1->TCR &= ~(0x1);		//Desactivo el Timer1

	}
	else
	{

		reproduciendo = 0;
		LPC_GPIO0->FIOCLR = (0x1<<7);	//Apago el led que indica reproduccion

		LPC_TIM0->TCR &= ~(0x1);		//Desactivo el Timer0
		LPC_TIM1->TCR &= ~(0x1);		//Desactivo el Timer1

	}


	LPC_TIM1->IR |= 0x1;				//Bajo el flag de interrupcion

}


void TIMER0_IRQHandler(void)
{
	if(nr_muestra < MAX_MUESTRAS){

		uint32_t aux;
		aux = LPC_DAC->DACR & (0x1<<16);
		aux |= (muestras[nr_muestra]<<4);
		LPC_DAC->DACR = aux & 0x1FFC0;		//Le paso el valor de la muestra
		nr_muestra ++;						//Incremento el contador de muestras

	}

	LPC_TIM0->IR |= (0x1<<1);									//Bajo el flag de interrupcion
}


void ADC_IRQHandler(void)
{

	if(LPC_ADC->ADDR0 & (0x1<<30)){

		overrun = 1;
		grabando = 0;
		grabacion_disponible = 0;

		NVIC_DisableIRQ(ADC_IRQn);		//Deshabilito interrupcion por ADC

		LPC_ADC->ADCR &= ~(1<<21);		//Apago el ADC
		LPC_SC->PCONP &= ~(1<<12);		//Quito potencia al ADC

		LPC_TIM0->TCR &= ~(0x1);		//Desactivo el Timer0
		LPC_TIM1->TCR &= ~(0x1);		//Desactivo el Timer1

		LPC_GPIO0->FIOSET = (0x1<<6) | (0x1<<7) | (0x1<<8); //Enciendo los 3 leds


	}
	else if(nr_muestra < MAX_MUESTRAS){

		muestras[nr_muestra] = (LPC_ADC->ADDR0>>4) & 0xFFF ;	//Guardo la muestra
		nr_muestra ++;											//Incremento el contador de muestras

	}

}









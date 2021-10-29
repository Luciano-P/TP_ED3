
#include "LPC17xx.h"

void conf_gpio(void);
void conf_tim0_g(void);
void conf_tim1(int ms);
void conf_ADC(void);
void conf_DAC(void);

/*
 * Configuro los puertos P0.0 y P0.1 como entradas para los comandos grabar y reproducir
 * Configuro los puertos P0.15 y P0.16 como salidas para los indicadores grabando y reproduciendo
 */
void conf_gpio(void)
{

	LPC_PINCON->PINMODE0 |= 0xF; 	//Pull-down a P0.0 y P0.1

	LPC_GPIOINT->IO0IntEnR |= 0x3;	//Habilito interrupciones por P0.0 y P0.1
	LPC_GPIOINT->IO0IntClr |= 0x3;	//Limpio las flags

	LPC_GPIO0->FIODIR |= (0x3<<15);	//Seteo los puertos P0.15 y P0.16 como salidas

	NVIC_EnableIRQ(EINT3_IRQn);		//Habilito las interrupciones en el NVIC
}


/*
 * Esta configuracion es para el TIMER0 al momento de grabar
 * Configuro el tim0 para match1 a 20 KHz, lo que me genera un rising edge a 10 KHz por toggleo
 */
void conf_tim0_g(void)
{

	LPC_SC->PCONP |= (0x1<<1); 		//Habilito el modulo tim 0
	LPC_SC->PCLKSEL0 &= ~(0x3<<2);	//PCLK = CCLK/4

	LPC_TIM0->PR = 24;				//Prescaler para una t_res de micros
	LPC_TIM0->MCR |= (0x1<<4);		//Receteo por MR1
	LPC_TIM0->MCR &= ~(0x1<<3);		//Desactivo interrupciones por MR1
	LPC_TIM0->MR1 = (49);			//Match para 20 KHZ
	LPC_TIM0->EMR |= (0x3<<6);		//Toggleo de MR1

	LPC_TIM0->CTCR |= 0x3 ;			//Inicio y receteo del timer
	LPC_TIM0->CTCR &= ~0x2;			//Quito el recet del timer

	NVIC_DisableIRQ(TIMER0_IRQn);

}


/*
 * Esta configuracion es para el TIMER0 al momento de reproducir
 * Configuro el tim0 para match1 a 10 KHz e interrupciones.
 */
void conf_tim0_r(void)
{

	LPC_SC->PCONP |= (0x1<<1); 		//Habilito el modulo tim 0
	LPC_SC->PCLKSEL0 &= ~(0x3<<2);	//PCLK = CCLK/4

	LPC_TIM0->PR = 24;				//Prescaler para una t_res de micros
	LPC_TIM0->MCR |= (0x3<<3);		//Receteo e interrupciones por MR1
	LPC_TIM0->MR1 = (99);			//Match para 10 KHZ
	LPC_TIM0->EMR &= ~(0x3<<6);		//Desactivo toggleo de MR1

	LPC_TIM0->CTCR |= 0x3 ;			//Inicio y receteo del timer
	LPC_TIM0->CTCR &= ~0x2;			//Quito el recet del timer

	NVIC_EnableIRQ(TIMER0_IRQn);

}


/*
 * Configuro el tim1 para contar hasta ms [ms] y generar una interrupcion
 */
void conf_tim1(int ms)
{

	LPC_SC->PCONP |= (0x1<<2); 		//Habilito el modulo tim 1
	LPC_SC->PCLKSEL0 &= ~(0x3<<4);	//PCLK = CCLK/4

	LPC_TIM1->PR = 24999;			//Prescaler para una tres de ms
	LPC_TIM1->MCR |= (0x3<<0);		//Interrrupciones y receteo por MR0
	LPC_TIM1->MR0 = (ms-1);			//Match a la cantidad de ms pasado por parametro

	LPC_TIM1->CTCR |= 0x3 ;			//Inicio y receteo del timer
	LPC_TIM1->CTCR &= ~0x2;			//Quito el recet del timer

	NVIC_EnableIRQ(TIMER1_IRQn);

}


/*
 * Configuro el ADC a PCLK = CCLK/8, con conversion por canal 0, con start por MR0.1 de flanco ascendente
 * e interrupciones por canal 0.
 */
void conf_ADC(void)
{

	LPC_PINCON->PINSEL1 |= (0x1<<14); 	//Habilito P0.23 como entrada AD0.0
	LPC_PINCON->PINMODE1 |= (0x2<<14);	//Sin resistencias pull-up ni pull-douwn

	LPC_SC->PCONP |= (0x1<<12);		//Habilito el modulo ADC
	LPC_SC->PCLKSEL0 != (0x3<<24);	//PCLK = CCLK/8

	LPC_ADC->ADCR |= (0x1<<21) | (0x4<<24);	//Enciendo el adc y configuro start por MR0.1
	LPC_ADC->ADINTEN = 0x1;					//Interrupciones por canal 0

	NVIC_EnableIRQ(ADC_IRQn);

}


/*
 * Configuro el DAC, con BIAS activo porque nuestra frecuencia de salida es mucho menor a 400HKz
 */
void conf_DAC(void)
{

	LPC_PINCON->PINSEL1 |= (0x2<<20);		//Habilito el P0.26 como salida del DAC
	LPC_PINCON->PINMODE1 |= (0x2<<20);		//Sin resistencias pull-up ni pull-down

	LPC_DAC->DACR |= (0x1<<16);				//Activo el BIAS

}





#include "LPC17xx.h"
#include "lpc17xx_uart.h"

#include "configuraciones.h"


/*
 * Configuro los puertos P0.0, P0.1, P0.18 como entradas para los comandos grabar, reproducir y enviar
 * Configuro los puertos P0.6, P0.7, P0.8 como salidas para los indicadores grabando, reproduciendo y enviando
 */
void conf_gpio(void)
{

	LPC_PINCON->PINMODE0 |= 0xF; 		//Pull-down a P0.0 y P0.1
	LPC_PINCON->PINMODE1 |= (0x3<<4);	//Pull-down a P0.18

	LPC_GPIOINT->IO0IntEnR |= 0x3 | (0x1<<18);	//Habilito interrupciones por P0.0, P0.1, P0.18
	LPC_GPIOINT->IO0IntClr |= 0x3 | (0x1<<18);	//Limpio las flags

	LPC_GPIO0->FIODIR |= (0x1<<6) | (0x1<<7) | (0x1<<8);	//Seteo los puertos P0.6, P0.7 y P0.8 como salidas

	NVIC_SetPriority(EINT3_IRQn, 7);
	NVIC_EnableIRQ(EINT3_IRQn);		//Habilito las interrupciones en el NVIC
}


/*
 * Esta configuracion es para el TIMER0 al momento de grabar
 * Configuro el tim0 para match1 a 8 KHz, lo que me genera un rising edge a 4 KHz por toggleo
 */
void conf_tim0_g(void)
{

    LPC_SC     -> PCON      |=  (1<<1);      //Enciendo el Timer PAG 65
    LPC_SC     -> PCLKSEL0  &= ~(3<<2);      //PCLK = CCLK/4

    LPC_TIM0   -> PR 		 =   24;	     //Prescaler para una t_res de micros
    LPC_TIM0   -> EMR       |=  (3<<6);      //Controla que pasa con el pin asociado al MATCH, en este caso togglea P1.28 (Salida de MATCH TIMER no GPIO) Pag 509
    LPC_TIM0   -> MR1        =   62;         //Valor cargado al Match Register 0 del TMR0
    LPC_TIM0   -> MCR        =  (1<<4);      //Reinicia el contador cuando TMR0 = MR1 pero no genera interrupcion PAG 507
    LPC_TIM0   -> TCR        =   3;          //Habilita al contador y lo pone en Reset PAG 505
    LPC_TIM0   -> TCR       &= ~(1<<1);      //Saco el TMR0 de Reset

	NVIC_DisableIRQ(TIMER0_IRQn);

}



/*
 * Esta configuracion es para el TIMER0 al momento de reproducir
 * Configuro el tim0 para match1 a 4 KHz e interrupciones.
 */
void conf_tim0_r(void)
{

    LPC_SC     -> PCON      |=  (1<<1);      //Enciendo el Timer
    LPC_SC     -> PCLKSEL0  &= ~(3<<2);      //PCLK = CCLK/4

    LPC_TIM0   -> PR 		 =   24;		 //Prescaler para una t_res de microsegundos
    LPC_TIM0   -> EMR       &=  ~(3<<6);     //M0.1 deshabilitado
    LPC_TIM0   -> MR1        =   124;        //MR0 cargado para generar 4 KHz
    LPC_TIM0   -> MCR        =   (3<<3);     //Reinicia el contador cuando TMR0 = MR0 y genera interrupcion
    LPC_TIM0   -> IR         =  (1<<1);      //Borro Flag de Interrupcion
    LPC_TIM0   -> TCR        =   3;          //Habilita al contador y lo pone en Reset
    LPC_TIM0   -> TCR       &= ~(1<<1);      //Saco el TMR0 de Reset

    NVIC_SetPriority(TIMER0_IRQn, 6);
	NVIC_EnableIRQ(TIMER0_IRQn);

}


/*
 * Configuro el tim1 para contar hasta ms [ms] y generar una interrupcion
 */
void conf_tim1(int ms)
{
	LPC_SC		->PCONP 	|=  (0x1<<2); 			//Habilito el modulo TIMER1
	LPC_SC		->PCLKSEL0  &= ~(0x3<<4);			//PCLK = CCLK/4

	LPC_TIM1	->PR   		 =     24999;			//Prescaler para una t_res de ms
	LPC_TIM1	->MCR 		|=  (0x3<<0);			//Interrrupciones y reseteo por MR0
	LPC_TIM1	->IR  		|=    (1<<0);
	LPC_TIM1	->MR0  	     =    (ms-1);			//Match a la cantidad de ms pasado por parametro

	LPC_TIM1	->TCR 		|=       0x3;			//Inicio y reseteo del timer
	LPC_TIM1	->TCR 		&= 		~0x2;			//Saco el TMR1 del Reset

	NVIC_SetPriority(TIMER1_IRQn, 5);
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
	LPC_SC->PCLKSEL0 |= (0x3<<24);	//PCLK = CCLK/8

	LPC_ADC -> ADCR    &= ~(1<<16);
	LPC_ADC -> ADCR    |= (0x1<<21) | (0x4<<24);	//Enciendo el adc y configuro start por MR0.1
	LPC_ADC -> ADINTEN  = 0x1;						//Interrupciones por canal 0

	NVIC_SetPriority(ADC_IRQn, 6);
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


/*
 * Configuramos el modulo UART0
 */
void conf_UART(void)
{

    LPC_PINCON->PINSEL0  |= 0x1<<4;		//P0.2 = TXD0
	LPC_PINCON->PINMODE0 |= 0X2<<4;		//No pull-up, no pull-down
	//LPC_PINCON->PINSEL0  |= 0x1<<6;		//P0.3 = RXD0
	//LPC_PINCON->PINMODE0 |= 0X2<<6;		//No pull-up, no pull-down

	UART_CFG_Type UARTconf;
	UARTconf.Baud_rate = 9600;				//Baud_rate = 9600 bits/seg
	UARTconf.Parity    = UART_PARITY_ODD;	//Configurado con bit de paridad impar
	UARTconf.Databits  = UART_DATABIT_8;	//Envia de a byte
	UARTconf.Stopbits  = UART_STOPBIT_1;    //Se configura con un bit de stop

	UART_FIFO_CFG_Type UARTFIFOconf;
	UARTFIFOconf.FIFO_ResetRxBuf = DISABLE;				//No se usa el RX no hace falta resetear su FIFO
	UARTFIFOconf.FIFO_ResetTxBuf = ENABLE;				//Reseteamos FIFO de TX
	UARTFIFOconf.FIFO_DMAMode    = DISABLE;             //Se deshabilita el modo DMA
	UARTFIFOconf.FIFO_Level      = UART_FIFO_TRGLEV0;	//No importa ya que solo afecta al RX

	UART_Init(LPC_UART0, &UARTconf);					//Inicializa el modulo UART0

	UART_FIFOConfig(LPC_UART0, &UARTFIFOconf);			//Inicializa FIFO del modulo UART0

	UART_TxCmd(LPC_UART0, ENABLE);						//Habilito transmision
}





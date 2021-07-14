#include <avr/io.h>

#include <util/delay.h>

#include "adc.h"



void adcinit(){	

	//make PA0 an analog input

	DDRA &= ~(1<<PA0);	  

	//enable ADC module, set prescalar of 128 which gives CLK/128 	

	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

	//set the voltage reference using REFS1 and REFS0 bits and select the ADC channel using the MUX bits

	ADMUX = 0b01000000;      // set REFS1 = 0 |REFS0 = 1 (Vref as AVCC pin) | ADLAR = 0(right adjusted) |  MUX4 to MUX0 is 0000 for ADC0

}


int adcread(char channel)							

{

	/* set input channel to read */

	ADMUX = 0x40 | (channel & 0x07);   // 0100 0000 | (channel & 0000 0100)

	/* Start ADC conversion */

	ADCSRA |= (1<<ADSC);      

    /* Wait until end of conversion by polling ADC interrupt flag */

	while (!(ADCSRA & (1<<ADIF)));    

   /* Clear interrupt flag */

	ADCSRA |= (1<<ADIF);               

	_delay_ms(1);                      /* Wait a little bit */

    /* Return ADC word */

	return ADCW;                      

}
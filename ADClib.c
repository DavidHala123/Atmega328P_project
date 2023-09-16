#include <avr/io.h>

void initalizeADC()
{
	ADMUX = (1<<REFS0) | (0<<ADLAR); // REFS0 - reference, ADLAR - ADMUX to Conversion Logic
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); //ADC enable, Interrupt enable, ADC Prescaler 128 --> 16MHz /128 = 125kHz
}

void ADC_start_conversion()
{
	ADCSRA |= (1<<ADSC); // STARTS CONVERSION ADC (BUTTON)
}
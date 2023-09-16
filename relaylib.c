/*
 * relaylib.c
 *
 * Created: 4/21/2023 5:42:44 PM
 *  Author: 42060
 */ 

#include <avr/io.h>
#include <util/delay.h>

#define RELAY_PIN_IN1 1
#define RELAY_PIN_IN2 5

void init_relay()
{
	DDRB |= (1 << RELAY_PIN_IN1);
	DDRB |= (1 << RELAY_PIN_IN2);
}

void out_relay1()
{
	PORTB &= ~(1 << RELAY_PIN_IN2);
	PORTB ^= (1 << RELAY_PIN_IN1);
}

void switch_relay1()
{
	PORTB &= ~(1 << RELAY_PIN_IN2);
	PORTB ^= (1 << RELAY_PIN_IN1);
}

void out_relay2()
{
	PORTB &= ~(1 << RELAY_PIN_IN1);
	PORTB |= (1 << RELAY_PIN_IN2);
}

void relay1_on()
{
	PORTB &= ~(1 << RELAY_PIN_IN2);
	PORTB |= (1 << RELAY_PIN_IN1);
}

void relay1_off()
{
	PORTB &= ~(1 << RELAY_PIN_IN2);
	PORTB &= ~(1 << RELAY_PIN_IN1);
}
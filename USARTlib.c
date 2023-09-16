#include <avr/io.h>
#include <stdbool.h>


void USART_Init(unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8); //set high baud rate
	UBRR0L = (unsigned char)ubrr; //Set low baud rate
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1 << RXCIE0); //enable Tx and Rx pin, Enable RX interupts
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); //Set frame format: 1stop bit, 8 bit data
}

uint16_t usart_read_string(uint8_t *arr)
{
	uint16_t index = 0;
	if (UCSR0A & (1<<RXC0)) //is bit recieved? (RXC == Recieve complete bit)
	{
		arr[index] = UDR0;
		while(UDR0 != '\r')
		{
			if(index >= 20)
				return false;
			
			else if (UCSR0A & (1<<RXC0))
			{
				index++;
				arr[index] = UDR0;
			}
		}
	}
	return index;
}



void usart_send(char input)
{
	while (!(UCSR0A & (1<<UDRE0))); //while Data buffer is not empty (empty flag --> UDREn)
	UDR0 = input; // Data buffer UDR0 = data --> sends the data back
}

void lcd_to_uart(char lcd_text[17])
{
	for(int i = 0; i < strlen(lcd_text); i++)
	{
		usart_send(lcd_text[i]);
	}
	usart_send('\n');
}
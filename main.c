#include <avr/io.h>
#include <avr/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>


#include "LCD_lib.h"
#include "mmKey.h"
#include "USARTlib.h"
#include "ADClib.h"
#include "I2Clib.h"
#include "relaylib.h"

#define FOSC 16000000
#define BAUD 38400
#define MYUBRR FOSC/16/BAUD-1
#define RTC_ADDR 0x68


//#define SQW (1 << 5) //D
#define SDA (1 << 4) //C
#define SCL (1 << 5) //C
uint8_t wrongPass_ct = 0;
uint8_t usartBuffer[20];
uint8_t usart_string[20];
uint16_t indexUS = 0;
bool usart_new = false;
uint8_t passwO[16];

bool presettingTime = false;
bool presetTimeSet = false;

uint8_t lcd_text[16] = {'5', '2'};
volatile char new_char;
volatile char memory_char;
volatile uint8_t i = 0;
uint8_t button;
bool but_state = false;

uint32_t eeAddr;
uint8_t RTC_data[7];

uint8_t presetTime[7];
bool presetLock = true;

char bufferR[20];
char bufferW[20];
volatile uint8_t err = 0;

//--------------------------------------------INTERUPTS--------------------------------------------------

ISR(ADC_vect) //ADC finished interupt
{
	button = ADC; //complete flag --> Data
}

ISR(USART_RX_vect) //message recieved interupt
{
	if(UDR0 != '\r') //end of message recieved
	{
		usartBuffer[indexUS] = UDR0;
		indexUS++;
	}
	else
	{
		usartBuffer[indexUS] = UDR0;
		indexUS++;
		for(int i = 0; i < sizeof(usart_string)/sizeof(usart_string[0]); i++)
		{
			usart_string[i] = usartBuffer[i];
		}
		usart_new = true;
		indexUS = 0;
		memset(usartBuffer, '\0', sizeof(usartBuffer)/sizeof(usartBuffer[0]));
	}
}


static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
	uart_putchar('\r', stream);
	do{
		
	}while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


//--------------------------------------------BUTTONS--------------------------------------------------
//MOST LEFT BUTTON PRESET TIME TO LOCK/UNLOCK DOORS. PASSWORD IS REQUIRED

void getButton() //Get button press from LCD module (HD44780)
{
	if(button == 0) //Subtract 10second from preset time
	{
		if(presettingTime)
		{
			presetTime[0] = presetTime[0] - 0x10;
			for(int i = 0; i < 2; i++)
			{
				if(presetTime[i] > 0x59)
				{
					presetTime[i] = 0x59;
					presetTime[i + 1] -= 0x01;
				}
			}
			but_state = true;
		}
	}
	else if(button >= 95 && button <= 105) //Add 10seconds to preset time
	{
		if(presettingTime)
		{
			presetTime[0] = presetTime[0] + 0x10;
			for(int i = 0; i < 2; i++)
			{
				if(presetTime[i] >= 0x59)
				{
					presetTime[i] = 0x00;
					presetTime[i + 1] += 0x01;
				}
			}
			but_state = true;
		}
	}
	else if(button >= 120 && button <= 140) //Button to preset time (Sets time when doors are locked/unlocked automatically) - RTC module used
	{
		if(!but_state)
		{
			if(!presettingTime && check_pass_match(&lcd_text))
			{
				read_RTC_time(&presetTime);
				presetTimeSet = false;
				presettingTime = true;
			}
			else
			{
				lcd_clear();
				i = 0;
				presetTimeSet = true;
				presettingTime = false;	
			}
			but_state = true;
		}
	}
	else if(button >= 145 && button <= 160) //Accepts preset time
	{
		if(!but_state)
		{
			presetLock ^= 1;
		}
	}
	else
	{
		but_state = false;
	}
	
	
	if(presettingTime) //Preset time mode
	{
		_delay_ms(30);
		if(presetLock)
		{
			sprintf(&lcd_text, "L: %02x:%02x:%02x", presetTime[2],presetTime[1],presetTime[0]);
		}		
		else
		{
			sprintf(&lcd_text, "U: %02x:%02x:%02x", presetTime[2],presetTime[1],presetTime[0]);
	
		}
		lcd_clear();
		printLCD(&lcd_text, strlen(lcd_text));
	}
	
}

//--------------------------------------------MAIN--------------------------------------------------

int main(void)
{
	stdout = &mystdout;
	DDRC &= ~SDA | ~SCL;
	initalizeADC();
	init_relay();
	twi_init(100000);
	lcd_init();
	mmKeyInit();
	USART_Init(MYUBRR);
	lcd_clear();
	uint8_t err = 0;
	sei();
	init_RTC_time();
	while (1)
	{
		if(usart_new) //User input: AU:12345 --> ADDS USER WITH PASSWORD 12345 TO EEPROM, ST:59381207160423 --> SETS TIME (sec, minute, hour, day in year (wont be printed), day, month, year), DU:12345 --> DELETES USER FROM EEPROM MEMORY, DAU --> DELETES ALL USERS
		{
			for(int i = 0; i < 16; i++)
			{
				passwO[i] = usart_string[i + 4];
			}
			if(usart_string[1] == 'A' && usart_string[2] == 'U')
			{
				lcd_clear();
				if(strlen(passwO) > 4)
				{
					err = eeprom_write_data(&passwO, 16);
					_delay_ms(30);
					if(err == 200)
					{
						printf("LOG: USER ADDED");
					}	
				}

			}
			if(usart_string[1] == 'S' && usart_string[2] == 'T')
			{
				if(strlen(passwO) == 14 && setTime_via_string(&passwO) == 200)
				{
					printf("LOG: TIME CHANGED SUCCESSFULLY\n");
				}
			}
			if(usart_string[1] == 'D' && usart_string[2] == 'U')
			{
				if(eeprom_delete_data_specific(atoi(passwO)) == 200)
				{
					printf("LOG: USER %d DELETED", atoi(passwO));
				}
				else
					printf("LOG: SOMETHING WENT WRONG");
					
				
			}
			if(usart_string[1] == 'D' && usart_string[2] == 'A' && usart_string[3] == 'U')
			{
				if(eeprom_delete_data() == 200)
					printf("LOG: ALL DATA DELETED");
				else
					printf("SOMETHING WENT WRONG");
			}
			usart_new = false;
		}
		
		//---------------------KEYBOARD --> LCD--------------------------

		ADC_start_conversion(); //ADC conversion for LCD buttons
		new_char = updateKeys();
		if (new_char != memory_char) //Membrane keyboard, * - INSERT PASSWORD, # - CLEARS LCD
		{
			switch(new_char)
			{
				case '*':
				lcd_clear();
				lcd_to_uart(lcd_text);
				if(check_pass_match(&lcd_text))
				{
					switch_relay1();
					wrongPass_ct = 0;
					read_RTC_time(&RTC_data);
					printf("%02x:%02x:%02x 20%02x/%02x/%02x LOG: ACCESS GRATNED \n", RTC_data[2],RTC_data[1],RTC_data[0],RTC_data[6],RTC_data[5],RTC_data[4]);
				}
				else
				{
					wrongPass_ct += 1;
					if(wrongPass_ct == 3)
					{
						out_relay2();
						read_RTC_time(&RTC_data);
						printf("%02x:%02x:%02x 20%02x/%02x/%02x LOG: WRONG PASSWORD - ALARM TRIGGERED \n", RTC_data[2],RTC_data[1],RTC_data[0],RTC_data[6],RTC_data[5],RTC_data[4]);
					}
				}
				memset(lcd_text, '\0', sizeof(lcd_text)/sizeof(lcd_text[0]));
				i = 0;
				break;
				
				case '#':
				lcd_clear();
				memset(lcd_text, '\0', sizeof(lcd_text));
				i = 0;
				break;
				
				case '\0':
				memory_char = '\0';
				break;
				
				default:
				if(strlen(lcd_text) <= 16)
				{
					lcd_text[i++] = new_char;
					lcd_data(new_char);
					memory_char = new_char;
				}
				break;
			}
		}
		
		//---------------------BUTTONS--------------------------
		
		getButton();
		
		if(presetTimeSet)
		{
			read_RTC_time(&RTC_data);
			if(memcmp(RTC_data, presetTime, strlen(RTC_data)) == 0)
			{
				if(presetLock)
					relay1_on();
				else
					relay1_off();
					
				presetTimeSet = false;
			}
		}
		
		//---------------------DELAY---------------------------
		
		_delay_ms(3000);  // Debounce delay
	}
}


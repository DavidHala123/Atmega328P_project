#include <avr/io.h>
#include <avr/delay.h>
#define LCD_RS 0
#define LCD_E 1
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define A0_LCD (1<<0)

void lcd_command(unsigned char cmd)
{
	PORTB &= ~(1<<LCD_RS);  // RS low for command
	PORTD = (PORTD & 0x0F) | (cmd & 0xF0);  // high nibble
	PORTB |= (1<<LCD_E);  // enable pulse
	_delay_ms(1);
	PORTB &= ~(1<<LCD_E);  // disable pulse
	_delay_ms(100);
	PORTD = (PORTD & 0x0F) | ((cmd << 4) & 0xF0);  // low nibble
	PORTB |= (1<<LCD_E);  // enable pulse
	_delay_ms(1);
	PORTB &= ~(1<<LCD_E);  // disable pulse
	_delay_ms(100);
}

void lcd_data(unsigned char data)
{
	PORTB |= (1<<LCD_RS);  // RS high for data
	PORTD = (data & 0xF0);  // high nibble
	PORTB |= (1<<LCD_E);  // enable pulse
	_delay_ms(1);
	PORTB &= ~(1<<LCD_E);  // disable pulse
	_delay_ms(100);
	PORTD = ((data << 4) & 0xF0);  // low nibble
	PORTB |= (1<<LCD_E);  // enable pulse
	_delay_ms(1);
	PORTB &= ~(1<<LCD_E);  // disable pulse
	_delay_ms(100);
}

void lcd_init()
{
	DDRB |= (1<<LCD_RS) | (1<<LCD_E); //Created specifically for LCD shield --> We can specify PINs in library
	DDRD |= (1<<LCD_D4) | (1<<LCD_D5) | (1<<LCD_D6) | (1<<LCD_D7);
	DDRC |= (1<<A0_LCD);
	_delay_ms(50);
	lcd_command(0x01);  // clear display
	lcd_command(0x02);  // return home
	lcd_command(0x28);  // 4-bit mode, 2 lines, 5x8 font
	lcd_command(0x0C);  // display on, cursor off, blink off
	lcd_command(0x06);  // entry mode, increment cursor
}

void lcd_shift(uint8_t shift)
{
	for(int i=0;i<=shift;i++)
	{
		lcd_command(0x18); //SHIFTS CURSOOR TO THE LEFT
		_delay_ms(5000);
	}
}

void lcd_clear()
{
	lcd_command(0x01); //Clears LCD
	lcd_command(0x02); //Position 0 - first line
}

void lcd_shiftAddr(uint8_t addr)
{
	uint8_t newAddr = 0x80 + addr;
	lcd_command(newAddr);
}

void printLCD(uint8_t *data, uint8_t len)
{
	for(int i = 0; i < len; i++)
	{
		lcd_data(data[i]);
	}
}
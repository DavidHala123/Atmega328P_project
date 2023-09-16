#include <avr/io.h>
#define ROW1_D (1<<2)
#define ROW2_D (1<<3)
#define ROW3_B (1<<3)
#define ROW4_B (1<<4)
#define COL1 (1<<1)
#define COL2 (1<<2)
#define COL3 (1<<3)


char keys[4][3] = {
	{'1','2','3'},
	{'4','5','6'}, 
	{'7','8','9'}, 
	{'*', '0', '#'}};

void mmKeyInit()
{
	DDRD |= ROW1_D | ROW2_D;
	DDRB |= ROW3_B | ROW4_B;
	DDRC &= ~(COL1);
	DDRC &= ~(COL2);
	DDRC &= ~(COL3);
	PORTC |= COL1 | COL2 | COL3;
}

char updateKeys() {
	for (int row = 0; row < 4; row++) {
		// Drive the row low
		if(row <= 1)
		{
			PORTD &= ~(1 << (row + 2) );
		}
		else
		{
			PORTB &= ~(1 << (row + 1) );
		}
		

		// Check for pressed keys in each column
		for(int col = 0 ;col <= 2; col++)
		{
			if(!(PINC & (1 << (col + 1) )))
			{
				return keys[row][col];
			}
		}

		// Drive the row high (reset)
		if(row <= 1)
		{
			PORTD |= (1 << (row + 2) );
		}
		else
		{
			PORTB |= (1 << (row + 1) );
		}
	}

	return '\0'; // No key press detected
}
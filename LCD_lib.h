#pragma once

void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init();
void lcd_shift(uint8_t shift);
lcd_clear();
void lcd_shiftAddr(uint8_t addr);
void printLCD(uint8_t *data, uint8_t len);
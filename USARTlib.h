#pragma once

void USART_Init(unsigned int ubrr);
void usart_send(char input);
void lcd_to_uart();
uint16_t usart_read_string(uint8_t *arr);
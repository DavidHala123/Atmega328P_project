#pragma once

void twi_init(uint32_t twiClkS);
uint8_t twi_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
uint8_t twi_read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
uint8_t twi_sqw_enable();


uint8_t eeprom_delete_data();
uint8_t eeprom_write_data(uint8_t *inputString, uint8_t len);
uint8_t get_eeprom_orderID();
uint8_t eeprom_read_data_specific(uint8_t orderID, uint8_t *outputBuffer);
uint8_t eeprom_write_data_specific(uint8_t orderID, uint8_t *outputBuffer);
uint8_t EEPROM_write(uint8_t reg, uint8_t *data, uint16_t len);
uint8_t EEPROM_read(uint8_t reg, uint8_t *data, uint16_t len);
uint8_t eeprom_delete_data_specific(uint8_t adr);

uint8_t setTime_via_string(uint8_t* input);
uint8_t init_RTC_time();
uint8_t read_RTC_time(uint8_t *data);
bool check_pass_match();

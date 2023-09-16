#include <avr/delay.h>
#include <stdbool.h>
#define SQW 4 //D
#define SDA 4 //C
#define SCL 5 //C

#define TWI_TIMEOUT 1600

#define TWI_START 0x08
#define TWI_RSTART 0x10
#define TWIT_ADDR_ACK 0x18
#define TWIT_ADDR_NACK 0x20
#define TWIT_DATA_ACK 0x28
#define TWIT_DATA_NACK 0x30

#define TWIR_ADDR_ACK 0x40
#define TWIR_ADDR_NACK 0x48
#define TWIR_DATA_ACK 0x50
#define TWIR_DATA_NACK 0x58

#define TWI_ERROR 0x38
#define TWI_NONE 0xF8

#define RTC_ADDR 0x68

enum{
	TWI_OK = 200,
	TWI_ERROR_START = 201,
	TWI_ERROR_RSTART = 202,
	TWI_NACK = 203,
	TWI_UNKNOWN_ERR = 204,
};

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t status = 0xF8;
uint8_t eeBuffer[16];

//Get status code when interupted
ISR(TWI_vect)
{
	status = (TWSR & 0xF8);
}

uint8_t init_RTC_data[7] = {0x50,0x59,0x23,0x07, 0x29, 0x03, 0x23};
uint8_t timeArray[7] = {0,0,0,0,0,0,0};
uint8_t indArray = 0;


void twi_init(uint32_t twiClkS)
{
	//Set TWI speed
	uint32_t gen_t = (((16000000/twiClkS) - 16) / 2) & 0xFF;
	DDRC &= ~(1 << SDA) | ~(1 << SCL);
	DDRD &= ~(1 << SQW);
	TWBR = gen_t & 0xFF;
	TWCR = (1 << TWEN) | (TWIE);				//Enable TWI interface and interupts
	PORTC |= (1 << SDA) | (1 << SCL);			//Enable pull up resistors on PC4 and PC5
}

static uint8_t twi_start(void)
{
	uint16_t i = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE); //START CONDITION
	while(status != TWI_START)
	{
		i++;
		if(i >= TWI_TIMEOUT)
		{
			return TWI_ERROR_START; //TIMEOUT HANDLING
		}
	}
	return TWI_OK;
}

static void twi_stop()
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN) | (1 << TWIE); //STOP CONDITION
}

static uint8_t twi_restart(void)
{
	uint16_t i = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE); //START CONDITION
	while(status != TWI_RSTART)
	{
		i++;
		if(i >= TWI_TIMEOUT)
		{
			return TWI_ERROR_START; //TIMEOUT HANDLING
		}
	}
	return TWI_OK;
}


static uint8_t twi_addr_write_ack(void)
{
	uint16_t i = 0;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE); //START CONDITION
	while(status != TWIT_ADDR_ACK)
	{
		i++;
		if(i >= TWI_TIMEOUT)
		{
			return TWI_ERROR_START; //TIMEOUT HANDLING
		}
	}
	return TWI_OK;
}

static uint8_t twi_addr_read_ack(void)
{
	uint16_t i = 0;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE); //START CONDITION
	while(status != TWIR_ADDR_ACK)
	{
		i++;
		if(i >= TWI_TIMEOUT)
		{
			return TWI_ERROR_START; //TIMEOUT HANDLING
		}
	}
	return TWI_OK;
}


static uint8_t twi_data_write_ack(void)
{
	uint16_t i = 0;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE); //START CONDITION
	while(status != TWIT_DATA_ACK)
	{
		i++;
		if(i >= TWI_TIMEOUT)
		{
			return TWI_ERROR_START; //TIMEOUT HANDLING
		}
	}
	return TWI_OK;
}


static uint8_t twi_data_read_ack(uint8_t ack)
{
	uint16_t i = 0;
	if(ack != 0)
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA); //START CONDITION
		while(status != TWIR_DATA_ACK)
		{
			i++;
			if(i >= TWI_TIMEOUT)
			{
				return TWI_ERROR_START; //TIMEOUT HANDLING
			}
		}
	}
	else
	{
		uint16_t i = 0;
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE); //START CONDITION
		while(status != TWIR_DATA_NACK)
		{
			i++;
			if(i >= TWI_TIMEOUT)
			{
				return TWI_ERROR_START; //TIMEOUT HANDLING
			}
		}
	}
	return TWI_OK;
}



uint8_t twi_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	err = twi_start();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = (addr << 1); //SLA + W
	
	err = twi_addr_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = reg;
	err = twi_data_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	for(i=0; i < len; i++)
	{
		TWDR = data[i];
		err = twi_data_write_ack();
		if(err != TWI_OK)
		{
			twi_stop();
			return err;
		}
	}
	
	twi_stop();
	return TWI_OK;
}


uint8_t twi_read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	err = twi_start();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = (addr << 1);
	
	err = twi_addr_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = reg;
	err = twi_data_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	err = twi_restart();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = (addr << 1) | 1;
	
	
	err = twi_addr_read_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	for(i=0; i < len - 1; i++)
	{
		err = twi_data_read_ack(1);
		if(err != TWI_OK)
		{
			twi_stop();
			return err;
		}
		data[i] = TWDR;
	}
	//NOT SURE
	err = twi_data_read_ack(0);
	data[i] = TWDR;
	
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	twi_stop();
	
	return TWI_OK;
	
}




uint8_t twi_sqw_enable()
{
	uint16_t data = {0x10};
	return twi_write(0x68, 0x07, &data, 1);
}






//EEPROM FUNCTION

uint8_t EEPROM_write_handler(uint16_t reg, uint8_t *data, uint16_t len)
{
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	err = twi_start();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = (0xA << 4); //SLA + W
	
	err = twi_addr_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	//FIRST AND SECOND WORD ADDRESS
	
	TWDR = (reg>>8);
	err = twi_data_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	TWDR = reg;
	err = twi_data_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	//DATA W
	
	for(i=0; i < len; i++)
	{
		TWDR = data[i];
		err = twi_data_write_ack();
		if(err != TWI_OK)
		{
			twi_stop();
			return err;
		}
	}
	
	twi_stop();
	return TWI_OK;
}


uint8_t EEPROM_read_handler(uint16_t reg, uint8_t *data, uint16_t len)
{
	uint16_t i = 0;
	uint8_t err = TWI_OK;
	err = twi_start();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = (0xA << 4);
	
	err = twi_addr_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	//FIRST AND SECOND WORD ADDRESS
	
	TWDR = (reg >> 8);
	err = twi_data_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	TWDR = reg;
	err = twi_data_write_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	err = twi_restart();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	TWDR = (0xA << 4) | 1;
	
	
	err = twi_addr_read_ack();
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	for(i=0; i < len - 1; i++)
	{
		err = twi_data_read_ack(1);
		if(err != TWI_OK)
		{
			twi_stop();
			return err;
		}
		data[i] = TWDR;
	}
	
	err = twi_data_read_ack(0);
	data[i] = TWDR;
	
	if(err != TWI_OK)
	{
		twi_stop();
		return err;
	}
	
	twi_stop();
	
	return TWI_OK;
	
}


uint8_t get_eeprom_orderID()
{
	uint8_t eeID = 0;
	uint8_t err = TWI_OK;
	err = EEPROM_read_handler(0xFFF, &eeID, 1);
	if(err != TWI_OK)
	{
		return 260;
	}
	return eeID;
}

uint8_t increment_eeprom_counter(uint16_t eeID)
{
	uint8_t err = TWI_OK;
	uint8_t newAddr = eeID + 1;
	err = EEPROM_write_handler(0xFFF, &newAddr, 1);
	return err;
}

uint8_t decrement_eeprom_counter(uint16_t eeID)
{
	uint8_t err = TWI_OK;
	uint8_t newAddr = eeID - 1;
	err = EEPROM_write_handler(0xFFF, &newAddr, 1);
	return err;
}

uint8_t eeprom_delete_data()
{
	uint8_t err = TWI_OK;
	uint32_t initEE = 0;
	err = EEPROM_write_handler(0xFFF, &initEE, 1);
	return err;
}

uint8_t eeprom_write_data(uint8_t *inputString, uint8_t len)
{
	uint8_t eOID = 0;
	uint8_t err = 0;
	eOID = get_eeprom_orderID();
	if(eOID == 260)
	{
		return TWI_UNKNOWN_ERR;
	}
	err = EEPROM_write_handler(eOID * 16, inputString, len);
	if(err == TWI_OK)
	{
		_delay_ms(30);
		err = increment_eeprom_counter(eOID);
	}
	return err;
}

uint8_t eeprom_read_data_specific(uint8_t orderID, uint8_t *outputBuffer)
{
	return EEPROM_read_handler(orderID * 16, outputBuffer, 16);
}

uint8_t eeprom_write_data_specific(uint8_t orderID, uint8_t *outputBuffer)
{
	return EEPROM_write_handler(orderID * 16, outputBuffer, 16);
}

uint8_t desegment_eeprom(uint8_t maxID)
{
	uint8_t eepromData[16];
	uint8_t err = 200;
	for(int i = 0; i < maxID; i++)
	{
		err = eeprom_read_data_specific(i, &eepromData);
		
		if(err == 200 && memcmp(eepromData, "AAAAAAAAAAAAAAAA", 16) == 0)
		{
			err = eeprom_read_data_specific(i+1, &eepromData);
			if(err != 200)
			{
				return err;
			}
			err = eeprom_write_data_specific(i, &eepromData);
			if(err != 200)
			{
				return err;
			}
			_delay_ms(30);
			err = eeprom_write_data_specific(i+1, "AAAAAAAAAAAAAAAA");
			if(err != 200)
			{
				return err;
			}
			_delay_ms(30);
		}
	}
	return err;
}

uint8_t eeprom_delete_data_specific(uint8_t adr)
{
	uint8_t err;
	uint8_t maxID = get_eeprom_orderID();
	if(eeprom_write_data_specific(adr, "AAAAAAAAAAAAAAAA") == 200)
	{
		err = desegment_eeprom(maxID);
		if(err != 200)
		{
			return err;
		}
		err = decrement_eeprom_counter(maxID);
		if(err != 200)
		{
			return err;
		}		
	}
	return err;
}

uint8_t setTime_via_string(uint8_t* input)
{
	sscanf(input, "%2x%2x%2x%2x%2x%2x%2x", &timeArray[0], &timeArray[1], &timeArray[2], &timeArray[3], &timeArray[4], &timeArray[5], &timeArray[6]);
	return twi_write(RTC_ADDR,0x00, &timeArray, sizeof(timeArray));
}

uint8_t init_RTC_time()
{
	return twi_write(RTC_ADDR,0x00, &init_RTC_data, sizeof(init_RTC_data));
}

uint8_t read_RTC_time(uint8_t *data)
{
	return twi_read(RTC_ADDR,0x00, data, sizeof(init_RTC_data));
}

bool check_pass_match(uint8_t *lcd_text)
{
	uint8_t maxID = get_eeprom_orderID();
	for(int i = 0; i < maxID; i++)
	{
		eeprom_read_data_specific(i, &eeBuffer);
		if(memcmp(eeBuffer, lcd_text, 16) == 0)
		{
			return true;
		}
	}
	return false;
}
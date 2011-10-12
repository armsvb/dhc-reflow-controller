/*! \file i2csw.c \brief Software-driven I2C interface using port pins. */
//*****************************************************************************
//
// File Name	: 'i2csw.c'
// Title		: Software-driven I2C interface using port pins
// Author		: Pascal Stang
// Created		: 11/22/2000
// Revised		: 5/2/2002
// Version		: 1.1
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include <avr/io.h>
#include <util/delay.h>

#include "i2csw.h"

// Standard I2C bit rates are:
// 100KHz for slow speed
// 400KHz for high speed

#define QDEL	_delay_us(5)		 // i2c quarter-bit delay
#define HDEL	_delay_us(10)		 // i2c half-bit delay

// i2c quarter-bit delay
//#define QDEL	asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");
// i2c half-bit delay
//#define HDEL	asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");

//#define I2C_SDL_LO      SDAPORT&=~(_BV(SDA))
//#define I2C_SDL_HI      SDAPORT|=_BV(SDA)

//external pull-up, port = 0; DDR = 0 -> Hi out/input, DDR = 1 -> low out
//needed for level translation !!!!!!

#define I2C_SDA_LO      SDADDR|=(_BV(SDA))
#define I2C_SDA_HI      SDADDR&=~(_BV(SDA))

#define I2C_SCL_LO      SCLDDR|=(_BV(SCL))
#define I2C_SCL_HI      SCLDDR&=~(_BV(SCL))

#define I2C_SCL_TOGGLE  HDEL; I2C_SCL_HI; HDEL; I2C_SCL_LO;
#define I2C_START       I2C_SDA_LO; QDEL; I2C_SCL_LO; 
#define I2C_STOP        HDEL; I2C_SCL_HI; QDEL; I2C_SDA_HI; HDEL;

/*
void i2ct(void)
{
	HDEL; I2C_SCL_HI; HDEL; I2C_SCL_LO;
}

void i2cstart(void)
{
	I2C_SDL_LO; QDEL; I2C_SCL_LO; 
}

void i2cstop(void)
{
	HDEL; I2C_SCL_HI; QDEL; I2C_SDL_HI; HDEL;
}


#define I2C_SCL_TOGGLE  i2ct();
#define I2C_START       i2cstart();
#define I2C_STOP        i2cstop();	
*/

uint8_t i2cPutbyte(uint8_t b)
{
	int i;
	
	for (i=7;i>=0;i--)
	{
		if ( b & (1<<i) )
			I2C_SDA_HI;
		else
			I2C_SDA_LO;			// address bit
			I2C_SCL_TOGGLE;		// clock HI, delay, then LO
	}

	I2C_SDA_HI;					// leave SDL HI
	// added    
//	SDADDR &= ~(_BV(SDA));		// change direction to input on SDA line (may not be needed)
	HDEL;
	I2C_SCL_HI;					// clock back up
  	b = SDAPIN & _BV(SDA);		// get the ACK bit

	HDEL;
	I2C_SCL_LO;					// not really ??
//	SDADDR |= _BV(SDA);			// change direction back to output
	HDEL;
	return (b == 0);			// return ACK value
}


uint8_t i2cGetbyte(uint8_t last)
{
	int i;
	uint8_t c,b = 0;
		
	I2C_SDA_HI;					// make sure pullups are ativated
//	SDADDR &= ~(_BV(SDA);		// change direction to input on SDA line (may not be needed)

	for(i=7;i>=0;i--)
	{
		HDEL;
		I2C_SCL_HI;				// clock HI
	  	c = SDAPIN & _BV(SDA);  
		b <<= 1;
		if(c) b |= 1;
		HDEL;
    	I2C_SCL_LO;				// clock LO
	}

//	sbi(SDADDR, SDA);			// change direction to output on SDA line
  
	if (last)
		I2C_SDA_HI;				// set NAK
	else
		I2C_SDA_LO;				// set ACK

	I2C_SCL_TOGGLE;				// clock pulse
	I2C_SDA_HI;					// leave with SDL HI
	return b;					// return received byte
}


//************************
//* I2C public functions *
//************************

//! Initialize I2C communication
void i2c_Init(void)
{
	SDADDR &= ~(_BV(SDA));		// set SDA as input
	SCLDDR &= ~(_BV(SCL));		// set SCL as input
	SDAPORT &= ~(_BV(SDA));	// as input pulled Hi by external pull-up, as output Lo
	SCLPORT &= ~(_BV(SCL));	// as input pulled Hi by external pull-up, as output Lo
}

//! Send a byte sequence on the I2C bus
void i2c_Send(uint8_t device, uint8_t subAddr, uint16_t length, uint8_t *dataxx)
{
	I2C_START;      			// do start transition
	i2cPutbyte(device); 		// send DEVICE address
	i2cPutbyte(subAddr);		// and the subaddress

	// send the data
	while (length--)
		i2cPutbyte(*dataxx++);

	I2C_SDA_LO;					// clear data line and
	I2C_STOP;					// send STOP transition
}

//! Retrieve a byte sequence on the I2C bus
void i2c_Receive(uint8_t device, uint8_t subAddr, uint8_t length, uint8_t *dataxx)
{
	int j = length;
	uint8_t *p = dataxx;

	I2C_START;					// do start transition
	i2cPutbyte(device);			// send DEVICE address
	i2cPutbyte(subAddr);   		// and the subaddress
	HDEL;
	I2C_SCL_HI;      			// do a repeated START
	I2C_START;					// transition

	i2cPutbyte(device | READ);	// resend DEVICE, with READ bit set

	// receive data bytes
	while (j--)
		*p++ = i2cGetbyte(j == 0);

	I2C_SDA_LO;					// clear data line and
	I2C_STOP;					// send STOP transition
}

uint8_t i2c_Receive_Status(uint8_t device)
{

	I2C_START;					// do start transition
	i2cPutbyte(device | READ);	// send DEVICE address and read

	return(i2cGetbyte(1));		// read status byte
}

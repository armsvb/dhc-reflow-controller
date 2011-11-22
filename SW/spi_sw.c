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

#include "spi_sw.h"

// Standard I2C bit rates are:
// 100KHz for slow speed
// 400KHz for high speed


// i2c quarter-bit delay
//#define QDEL	asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");
// i2c half-bit delay
//#define HDEL	asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");

//#define I2C_SDL_LO      SDAPORT&=~(_BV(SDA))
//#define I2C_SDL_HI      SDAPORT|=_BV(SDA)

//external pull-up, port = 0; DDR = 0 -> Hi out/input, DDR = 1 -> low out
//needed for level translation !!!!!!

#define SPI_SDA_LO      SDADDR|=(_BV(SDA))
#define SPI_SDA_HI      SDADDR&=~(_BV(SDA))

#define SPI_SCL_LO      SCLDDR|=(_BV(SCL))
#define SPI_SCL_HI      SCLDDR&=~(_BV(SCL))


//************************
//* SPI_sw public functions *
//************************

//! Initialize SPI communication (one way only)
void Spi_sw_Init(void)
{
	SDADDR &= ~(_BV(SDA));		// set SDA as input
	SCLDDR &= ~(_BV(SCL));		// set SCL as input
	SDAPORT &= ~(_BV(SDA));	// as input pulled Hi by external pull-up, as output Lo
	SCLPORT &= ~(_BV(SCL));	// as input pulled Hi by external pull-up, as output Lo
	SCLDDR |= _BV(SCL);			// set SCL as output (pull low)

}

//! Send a byte sequence on the I2C bus
void Spi_sw_Send8_t(uint8_t data2send)
{
	uint8_t i;
	
	// send the data
	for (i=0;i<8;i++)
	{
		if(data2send & 0x80)
			SPI_SDA_HI;
		else	
			SPI_SDA_LO;
		
		data2send = data2send << 1;
		SPI_SCL_HI;
		_delay_us(0.5);
		SPI_SCL_LO;
		_delay_us(0.5);
	}
	SPI_SDA_LO;					// clear data line and
	return;
}



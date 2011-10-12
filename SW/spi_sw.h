/*! \file i2csw.h \brief software-driven I2C interface using port pins. */
//*****************************************************************************
//
// File Name	: 'i2csw.h'
// Title		: software-driven I2C interface using port pins
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

#ifndef SPI_SW_H
#define SPI_SW_H

#include "hw.h"
#include "eadogs.h"

#ifndef GLUE
	#define GLUE(a, b)     a##b
	#define PORT(x)        GLUE(PORT, x)
	#define PIN(x)         GLUE(PIN, x)
	#define DDR(x)         GLUE(DDR, x)
#endif	//GLUE

// defines and constants
#define READ		0x01	// I2C READ bit

#define SDAPORT				PORT(DISP_P)
#define SCLPORT				PORT(DISP_P)
#define SDADDR				DDR(DISP_P)
#define SCLDDR				DDR(DISP_P)
#define SDA					DISP_SDA
#define SCL					DISP_SCL


// functions

// initialize I2C interface pins
void Spi_sw_Init(void);

// send spi data to port
void Spi_sw_Send8_t(uint8_t data2send);


#endif

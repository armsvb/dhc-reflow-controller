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

#ifndef I2CSW_H
#define I2CSW_H

#include "hw.h"
#include "bo9864.h"

#ifndef GLUE
	#define GLUE(a, b)     a##b
	#define PORT(x)        GLUE(PORT, x)
	#define PIN(x)         GLUE(PIN, x)
	#define DDR(x)         GLUE(DDR, x)
#endif	//GLUE

// defines and constants
#define READ		0x01	// I2C READ bit

#define SDAPORT				PORT(SDA_P)
#define SCLPORT				PORT(SCL_P)
#define SDADDR				DDR(SDA_P)
#define SCLDDR				DDR(SCL_P)
#define SDAPIN				PIN(SDA_P)

// functions

// initialize I2C interface pins
void i2c_Init(void);

// send I2C data to <device> register <sub>
void i2c_Send(uint8_t device, uint8_t sub, uint16_t length, uint8_t *data);

// receive I2C data from <device> register <sub>
void i2c_Receive(uint8_t device, uint8_t sub, uint8_t length, uint8_t *data);

// reseive status uint8_t - non standard i2c comm - pcf8548
uint8_t i2c_Receive_Status(uint8_t device);


#endif

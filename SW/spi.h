/*! \file spi.h \brief SPI interface driver. */
//*****************************************************************************
//
// File Name	: 'spi.h'
// Title		: SPI interface driver
// Author		: Pascal Stang - Copyright (C) 2000-2002
// Created		: 11/22/2000
// Revised		: 06/06/2002
// Version		: 0.6
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
///	\ingroup driver_avr
/// \defgroup spi SPI (Serial Peripheral Interface) Function Library (spi.c)
/// \code #include "spi.h" \endcode
/// \par Overview
///		Provides basic byte and word transmitting and receiving via the AVR
///	SPI interface.  Due to the nature of SPI, every SPI communication operation
/// is both a transmit and simultaneous receive.
///
///	\note Currently, only MASTER mode is supported.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#ifndef SPI_H_
#define SPI_H_

#ifndef GLUE
	#define GLUE(a, b)     a##b
	#define PORT(x)        GLUE(PORT, x)
	#define PIN(x)         GLUE(PIN, x)
	#define DDR(x)         GLUE(DDR, x)
#endif	//GLUE

#define SPI_HIGH_SPEED		SPCR &= ~(_BV(SPR1)|_BV(SPR0)); SPSR |= (_BV(SPI2X))
#define SPI_PORT			PORT(SPI_P)
#define SPI_DDR				DDR(SPI_P)

// function prototypes

// SPI interface initializer
void SPI_Init(void);

void SPI_Send8(uint8_t data);

uint8_t SPI_Receive8(void);

uint8_t SPI_Transfer8(uint8_t data);

uint16_t SPI_Transfer16(uint16_t data);

#endif

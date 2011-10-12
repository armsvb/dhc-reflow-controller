#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "hw.h"
#include "spi.h"

// Comment-out or uncomment this line as necessary
//#define SPI_USEINT

// global variables

#ifdef SPI_USEINT
volatile uint8_t spiTransferComplete;
#endif


// SPI interrupt service handler
#ifdef SPI_USEINT
ISR(SPI_STC_vector)
{
	spiTransferComplete = TRUE;
}
#endif

// access routines

void SPI_Init(void)
{
	uint8_t temp;
	
	SPI_DDR |= (_BV(SPI_SCK)|_BV(SPI_MOSI));	//set outputs
//	SPI_DDR &= ~(_BV(SPI_SS));
	SPI_DDR &= ~(_BV(SPI_MISO));				//set input
	SPI_PORT |= _BV(SPI_SCK);					//set sck Hi
	
/* setup SPI interface :
	SPR0 = 1	
	SPR1 = 0	// fclk/16
	CPHA = 0	// leading edge sample, trailing edge setup
	CPOL = 0	// sck is Lo when idle
	MSTR = 1	// master
	DORD = 0	// MSB first
	SPE  = 1	// SPI enable		*/

	SPCR = (_BV(MSTR)|_BV(SPE)|_BV(SPR0));
	
	temp = SPSR;						// clear status

	#ifdef SPI_USEINT
	spiTransferComplete = TRUE;			// enable SPI interrupt
	SPCR |= _BV(SPIE);
	#endif
}

void SPI_Send8(uint8_t dataout)
{

	SPDR = dataout;

	#ifdef SPI_USEINT
		while(!spiTransferComplete);	// send a byte over SPI and ignore reply
		spiTransferComplete = FALSE;
	#else
		while(!(SPSR & _BV(SPIF)));
	#endif
	dataout = SPDR;
}

uint8_t SPI_Receive8(void)
{

	SPDR = 0xff;

	#ifdef SPI_USEINT
		while(!spiTransferComplete);	// send a byte over SPI and ignore reply
	#else
		while(!(SPSR & _BV(SPIF)));
	#endif
	return(SPDR);
}



uint8_t SPI_Transfer8(uint8_t dataout)
{
	#ifdef SPI_USEINT
	spiTransferComplete = FALSE;		// send the given data
	SPDR = dataout;
	while(!spiTransferComplete);		// wait for transfer to complete
	#else
	SPDR = dataout;						// send the given data
	while(!(SPSR & _BV(SPIF)));		// wait for transfer to complete
	#endif

	return(SPDR);						// return the received data
}

uint16_t SPI_Transfer16(uint16_t dataout)
{
	uint16_t rxData = 0;

	rxData = ((uint16_t)(SPI_Transfer8((uint8_t)(dataout>>8))))<<8;	// send MS byte of given data
	rxData |= (SPI_Transfer8((uint8_t)dataout));			// send LS byte of given data

	return(rxData);											// return the received data
}

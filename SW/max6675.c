
#include <inttypes.h>
#include <avr/io.h>

#include "spi.h"
#include "max6675.h"
#include "hw.h"

void MAX_Init_hw(void)
{
	TEMPDDR |= _BV(MAX0);
	TEMPDDR &= ~(_BV(MAX1));			// acts as HWB - active low - uses external pullup
	TEMPPORT |= _BV(MAX0);
	TEMPPORT &= ~(_BV(MAX1));			// only external pullup 
	SPI_Init();
}

uint16_t MAX_Read_temp(uint8_t sensor)
{
	uint16_t datamax;
	
	
	if(sensor)
		TEMPDDR |= _BV(MAX1);		// set as output with port preset to low
	else
		TEMPPORT &= ~(_BV(MAX0));	// send low to port

	datamax = SPI_Transfer16(0x0000);

	TEMPPORT |= _BV(MAX0);
	TEMPDDR &= ~(_BV(MAX1));
	if(datamax & 0x0004)
	{
		datamax = 0xFFFF;
	}
	else 
	{
		datamax = datamax >> 3;
	}
	return(datamax);
}
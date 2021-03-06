
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>

#include "hw.h"
#include "eadogs.h"
#include "spi_sw.h"

//----------------------------------------------------------
//					GLOBAL VARIABLES
//----------------------------------------------------------
#ifdef GLCD_BUFFER
//	uint8_t lcd_buffer[8][102];
#endif
//----------------------------------------------------------
//						FUNCTIONS
//----------------------------------------------------------

void eadogs_init_port(void)
{
	GLCD_CD_PORT &= ~(_BV(DISP_CD));				//Clear data
	GLCD_CD_DDR &= ~(_BV(DISP_CD));					//set data port to output - reset low
	GLCD_CS_PORT &= ~(_BV(DISP_CS));				//Clear data - external pullup or data low
	GLCD_CS_DDR &= ~(_BV(DISP_CS));				// external pullup
	Spi_sw_Init();
	return;
}

//-----------------------------------------------------------


void eadogs_cmd_write(uint8_t cmd, uint8_t cs) 					//writing 1cmd
{
	GLCD_CS_ASSERT;
	GLCD_CD_DDR |= _BV(DISP_CD);
	GLCD_CD_DDR |= _BV(DISP_CD);
	Spi_sw_Send8_t(cmd);
	if(cs)
		GLCD_CS_DEASSERT;
//	SPCR |= _BV(MSTR);
	return;
}

void eadogs_cmd_write16(uint8_t cmd1, uint8_t cmd2, uint8_t cs) 					//writing 1cmd
{
	GLCD_CS_ASSERT;
	GLCD_CD_DDR |= _BV(DISP_CD);
	GLCD_CD_DDR |= _BV(DISP_CD);
	Spi_sw_Send8_t(cmd1);
	Spi_sw_Send8_t(cmd2);
	if(cs)
		GLCD_CS_DEASSERT;
//	SPCR |= _BV(MSTR);
	return;
}


//-----------------------------------------------------------
//------------------Public functions-------------------------
//-----------------------------------------------------------


void eadogs_data_write(uint8_t dataxx, uint8_t cs)
{
	GLCD_CS_ASSERT;
	GLCD_CD_DDR &= ~(_BV(DISP_CD));
	Spi_sw_Send8_t(dataxx);
	if(cs)
		GLCD_CS_DEASSERT;
	return;
}


//-----------------------------------------------------------

void eadogs_init_glcd(void)
{
	uint8_t x = 0;
	uint8_t y;
	
	eadogs_init_port();
	_delay_ms(100);
	
	eadogs_cmd_write(GLCD_SET_SCROLL_LINE,0);
	eadogs_cmd_write(GLCD_MX | GLCD_MIRROR_X,0);	// reverse X orientation??
	eadogs_cmd_write(GLCD_MY,0);					//normal Y orientation
	eadogs_cmd_write(GLCD_ALL_ON | 0x01,0);		//set all pixels on
	eadogs_cmd_write(GLCD_INVERSE,0);				//set inverse off
	eadogs_cmd_write(GLCD_SET_BIAS,0);
	eadogs_cmd_write(GLCD_SET_POWER_CTRL | 0x07,0);	// set all on
	eadogs_cmd_write16(GLCD_SET_ELEC_VOLUME, 0x19,0);	// set LCD contrast (default)
	eadogs_cmd_write16(GLCD_SET_ADV_PGM_CTRL, 0x90,0);	// set TC to -0.11 %/C
	eadogs_cmd_write(GLCD_ENABLE | 0x01,1);		// display enable

	_delay_ms(1000);
	
	eadogs_cmd_write(GLCD_ALL_ON,1);				//set all pixels off
	for(y=0; y<8; y++)
	{
		eadogs_setxy(0,y);
		for(x=0; x<132; x++)
		{
			eadogs_data_write(0,1);
		}
	}
	return;
}

//-----------------------------------------------------------

void eadogs_setxy(uint8_t x, uint8_t y)
{
	if(x>101)
		x=101;
	if(y>7)
		y=7;
	
	eadogs_cmd_write(GLCD_SET_Y | y,0);
	eadogs_cmd_write(GLCD_SET_XLSB | (x & 0x0F),0);
	eadogs_cmd_write(GLCD_SET_XMSB | ((x & 0xF0)>>4),1);
	return;
}

//-----------------------------------------------------------


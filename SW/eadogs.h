#ifndef	EADOGS_H_
#define EADOGS_H_

#include "hw.h"
#include "spi_sw.h"

//----------------------------------------------------------
//						DEFS
//----------------------------------------------------------

#ifndef GLUE
	#define GLUE(a, b)     a##b
	#define PORT(x)        GLUE(PORT, x)
	#define PIN(x)         GLUE(PIN, x)
	#define DDR(x)         GLUE(DDR, x)
#endif	//GLUE


#define 	GLCD_CS_PORT	PORT(DISP_CS_P)
#define		GLCD_CS_DDR		DDR(DISP_CS_P)
#define 	GLCD_CD_PORT	PORT(DISP_CD_P)
#define		GLCD_CD_DDR		DDR(DISP_CD_P)
#define		GLCD_CD			DISP_RST

#define		GLCD_CS_ASSERT	GLCD_CS_DDR|=_BV(DISP_CS)
#define		GLCD_CS_DEASSERT	GLCD_CS_DDR&=~(_BV(DISP_CS))
#define		GLCD_COMMAND	GLCD_CD_DDR|=_BV(DISP_CD)
#define		GLCD_DATA		GLCD_CD_DDR&=~(_BV(DISP_CD))


// DOG-S command set
#define GLCD_SET_POWER_CTRL		0x28	// 00101XYZ: X = Follower ON/OFF (1/0)
										//			 Y = Regulator ON/OFF
										//			 Z = Booster ON/OFF
#define GLCD_ALL_ON				0xA4	// 1010010X: 1=all pixels on, 0=normal
#define GLCD_INVERSE			0xA6	// 1010011X: 1=inverse, 0=normal
#define GLCD_ENABLE				0xAE	// 1010111X: 1=enable, 0=disable (sleep)
#define GLCD_NOP				0xE3	// 11100011: NOP
#define GLCD_SET_BIAS			0xA2	// 1010001X: set bias ratio 0=1/9, 1=1/7
#define GLCD_SET_VLCD_RES_RAT	0x20	// 00100XXX: set internal resistor ratio 0-7
#define GLCD_SET_ELEC_VOLUME	0x81	// 10000001: adjust contract of LCD, next byte 0-63
#define GLCD_SET_ADV_PGM_CTRL	0xFA	// 11111010: next byte: X00100YZ: 
										// X = Temp comp. 0=-0.05 %/C; 1 = -0.11 %/C
										// Y = Column wrap 0 = OFF
										// Z = Page wrap 0 = OFF
										
#define GLCD_SYSTEM_RESET		0xE2	// 11100010

#define GLCD_SET_Y				0xB0	// 1011YYYY: set lcd page (Y) address 0-7 PA[3:0]
#define GLCD_SET_XLSB			0x00	// 0000XXXX: set lcd X address 0 - 131 CA[3:0]
#define GLCD_SET_XMSB			0x10	// 0001XXXX: set lcd X address 0 - 131 CA[7:4]
#define GLCD_SET_SCROLL_LINE	0x40	// 01XXXXXX: set the display start line 0-63 SL[5:0]

#define GLCD_MX					0xA0	// 1010000X: 0 - normal, 1 - mirror
#define GLCD_MY					0xC0	// 1100X000: 0 - normal, 1 - mirror

//Masky
#define GLCD_MIRROR_Y			0x04	
#define GLCD_MIRROR_X			0x01	
#define GLCD_POWER_FOLLOWER		0x04	
#define GLCD_POWER_REGULATOR	0x02	
#define GLCD_POWER_BOOSTER		0x01	


#define RIGHT				0
#define LEFT				1
#define BUSY				0x80


//-----------------------------------------------------------------------------------------------//
//				structure definitions 
//-----------------------------------------------------------------------------------------------//
typedef struct LCD_LOC
{
	uint8_t page;
	uint8_t adr;
} LCD_LOC;

//typedef struct LCD_BUFF
//{
//	uint8_t page;
//	uint
//}

//----------------------------------------------------------
//					GLOBAL VARIABLES
//----------------------------------------------------------
#ifdef GLCD_BUFFER
//	extern uint8_t lcd_buffer[8][102];
#endif
//extern PGM_P LOGO_LUT[9][132];
//extern PGM_P ALPHA_LUT[26][5];
//extern PGM_P alpha_LUT[26][5];
//extern PGM_P NUM_LUT[10][5];

//----------------------------------------------------------
//						FUNCTIONS
//----------------------------------------------------------


void eadogs_data_write(uint8_t data, uint8_t cs);
//
void eadogs_cmd_write16(uint8_t cmd1, uint8_t cmd2, uint8_t cs);

void eadogs_init_glcd(void);

void eadogs_setxy(uint8_t x, uint8_t y);



#endif 		//EADOGS_H_


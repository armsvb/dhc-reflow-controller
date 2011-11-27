#ifndef FONT_H_
#define FONT_H_

/********************************************************************************
	NAME 		: FONT.H
	
	EXTENDED NAME	: Fonts for graphic LCD based on KS0108 or HD61202
	
	LAYER		: Application
	
	AUTHOR		: Stephane REY			  


REVISIONS :
 				
--------------------------------------------------------------------------------
DATE		VERSION	REVISOR DESCRIPTION				
--------------------------------------------------------------------------------
13.02.2003	V1.0 	SR	Initialization
--------------------------------------------------------------------------------
				
********************************************************************************/
/*******************************************************************************/
/* 			     DECLARATIONS / DEFINITIONS			       */
/*******************************************************************************/

#define FONT_5X7

/* EXTERN Function Prototype(s) */

/* Extern definitions */

typedef struct  
{
	uint8_t W;     		/* Character width for storage        */
	uint8_t H;  		/* Character height for storage       */
	uint8_t O;			/* ASCII char starting the table 	  */
	prog_uint8_t *FontTable;        /* Font table start address in memory */
} FONT_DEF;
#ifdef FONT_3X6
extern FONT_DEF Font_System3x6;
#endif

#ifdef FONT_5X7
//extern FONT_DEF Font_System5x7;
extern prog_uint8_t FontSystem5x7[];
#endif

#ifdef FONT_5X8
extern FONT_DEF Font_System5x8;
#endif

#ifdef FONT_7X8
extern FONT_DEF Font_System7x8;
#endif

#ifdef FONT_COURIER
extern FONT_DEF Font_Courrier8x12;
#endif

//extern static uint8_t FontSystem3x6[] PROGMEM;
//extern static uint8_t FontSystem5x8[] PROGMEM;
//extern static uint8_t FontSystem7x8[] PROGMEM;
//extern static uint8_t FontCourrier8x12[] PROGMEM;

#endif		//FONT_H_
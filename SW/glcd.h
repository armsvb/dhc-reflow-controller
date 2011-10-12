
#ifndef GLCD_H_
#define GLCD_H_

#include "font.h"

//---------------------------------------------------------
//-------------------- Structure Defs ---------------------
//---------------------------------------------------------

typedef struct GLCD_POS_
{
	uint8_t X;		// X position
	uint8_t Y;		// Y position
} GLCD_POS;

//---------------------------------------------------------
//----------------- Function prototypes -------------------
//---------------------------------------------------------

//#define GLCD_Setxy(x,y)			k0108_setxy(x,y)

void GLCD_Init(void);

void GLCD_Clr(void);

void GLCD_Line(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

void GLCD_Circle(uint8_t, uint8_t, uint8_t);

void GLCD_Rectangle(uint8_t, uint8_t, uint8_t, uint8_t);

void GLCD_DisplayPicture (PGM_P PictureData);

void GLCD_Putchar (uint8_t Char, FONT_DEF *toto, uint8_t overlay);

void GLCD_Locate (uint8_t Column, uint8_t Line);

void GLCD_UpdateDisplay(void);

void GLCD_Clr_Buffer(void);

void GLCD_Set_Dot(uint8_t x, uint8_t y);

void GLCD_Clr_Dot(uint8_t x, uint8_t y);

//int glcd_putchar(uint8_t c, FILE*);

void GLCD_Set_Font(FONT_DEF*);

#endif		//GLCD_H_

/*
		Simple menu
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "hw.h"
#include "routines.h"
#include "eeprom.h"

#ifdef LCD
	#include "glcd.h"
	#include "eadogs.h"
#endif


//------------------------------------------------
//			Global PGM constants
//------------------------------------------------

PGM_P PROFILES = " Temp profiles\r\n";
PGM_P PID = " PID debug\r\n";
PGM_P EXIT = " Exit\r\n";
PGM_P STAR = "*";
PGM_P BOTT_ROW = " DOWN   SLCT   UP";
PGM_P EMPTY = "\r\n";


void Menu_simple(int8_t enc_data)
{
	static uint8_t menu = 0, menu_item = 0; max_item = 2; settings_old;
	uint8_t data2, pom;
	PGM_P row1, row2, row3, row4, star1, star2, star3, star4;
	
	if(enc_data)
	{
		Status_task |= TASK_MENUSTART;			// changed
		switch(enc_data)
		{
			case KEYLEFT:	menu_item++;
							if(menu_item > max_item)
								menu_item = 0;
							break;
			case KEYRIGHT: 	if(menu_item == 0)
								menu_item = max_item;
							else
								menu_item--;
							break;
			case KEYSWITCH:
							break;
			case KEYSTOP:	Status_task &= ~TASK_MENU;
							LED3_OFF;
							break;
			default:		break;
		}
	}
	
	switch(menu)
	{
		case 	0:  row1 = PROFILES;
					row2 = PID;
					row3 = EXIT;
					row4 = EMPTY;
					max_item = 2
					break;
		case	1:	row1 = PROFILES;
					row2 = TIME;
					row3 = TEMP;
					row4 = NEXT;
					break;
		case	2:	row1 = PID;
					row2 = P;
					row3 = I;
					row4 = D;
					break;
		case	3:	pom &= 0x80;										//mod klavesnice
					switch (pom)
					{
						case 0x00	:	base = CISLA;
										break;
						case 0x80	:	base = SIPKY;
										break;
					}
					printf_P(PSTR("Klavesnica:%s"), base);
					if(data2==6)
					{
						if (pom)
							pom = 0x00;
					}
					if(data2==4)
					{
						if(!pom)
							pom = 0x80;	
					}
					settings_old &= 0x7F;
					settings_old |= pom;
					break;
		case	4:  printf_P(PSTR("Press 5 to load"));
					if((data2&0x0F)==5)
					{
						ee_load(&settings_old,set);
					}
					break;
		case	5:	printf_P(PSTR("Press 5 to save"));
					if((data2&0x0F)==5)
					{
						ee_save(&settings_old,set);
					}
					break;
		default	 :  break;
	}
	*settings = settings_old;


	if(Status_task & TASK_MENUSTART)			// if changed
	{
#ifdef LCD
	lcd_clr();
	lcd_setxy(0,0);
	printf_P(PSTR("MENU\n"));
	settings_old = (*settings);
	pom = settings_old;
#endif
		if(Status_com & DEBUG)
		{
			pprintf_P(PSTR("\r\n M E N U :\n"),USB_DEF);
			pprintf_P(star1,USB_DEF);
			pprintf_P(row1,USB_DEF);
			pprintf_P(star2,USB_DEF);
			pprintf_P(row2,USB_DEF);
			pprintf_P(star3,USB_DEF);
			pprintf_P(row3,USB_DEF);
			pprintf_P(star4,USB_DEF);
			pprintf_P(row4,USB_DEF);
			pprintf_P(BOTT_ROW,USB_DEF);
		}
	}
	
	
	

}
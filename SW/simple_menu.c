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
PGM_P P = " P :";
PGM_P I = " I :";
PGM_P D = " D :";
PGM_P EXIT = " Exit\r\n";
PGM_P BOTT_ROW = " DOWN   SLCT   UP";
PGM_P EMPTY = "\r\n";


void Menu_simple(int8_t enc_data)
{
	static uint8_t menu = 0, menu2 = 0; menu_item = 0; max_item = 2; number = 0;
	uint8_t data2, pom;
	PGM_P row1, row2, row3, row4;
	
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
			case KEYSWITCH: if(menu2 == 0)
							{
								menu2 = menu_item;
								menu_item = 1;
							}
							else if(menu == 0)
							{
								menu = menu_item;
								menu_item = 1;
							}
							else 
							{
								number = menu_item;
							}
							break;
			case KEYSTOP:	Status_task &= ~TASK_MENU;
							LED3_OFF;
							break;
			default:		break;
		}
	}

	if(menu2)
	else
	{
		switch(menu & 0x03)
		{
			case 	0:  switch(menu_item)
						{
							case 0: row1 = EXIT;
									break;
							case 1: row1 = PROFILES;
									break;
							case 2: row1 = PID;
									break;
						}
						row2 = EMPTY;
						row3 = EMPTY;
						row4 = EMPTY;
						max_item = 2
						break;
			case	1:	row1 = PROFILES;
						case(menu_item)
						{
							case 0: row2 = EXIT;
									break;
							case 1: row2 = Pbsn;
									break;
							case 2: row2 = Pbfree;
									break;
							case 3: row2 = Baking;
									break;
							case 4: row2 = Drying;
									break;
							case 5: 
							case 6:
							case 7:
							case 8:
							case 9:
							case 10: row2 = User;
									break;
						}
						row3 = EMPTY;
						row4 = EMPTY;
						max_item = 10;
						break;

			default	 :  break;
		}
	}

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
			pprintf_P(row1,USB_DEF);
			pprintf_P(row2,USB_DEF);
			pprintf_P(row3,USB_DEF);
			pprintf_P(row4,USB_DEF);
			pprintf_P(BOTT_ROW,USB_DEF);
		}
	}
	
	
	

}
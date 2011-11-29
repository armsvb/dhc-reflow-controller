// routines.c	with stdio printf function (size test)
#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "hw.h"
#include "routines2.h"
#include "spi.h"
#include "led.h"
#include "pwm.h"
#include "usb.h"
#include "pid.h"
#include "glcd.h"
#include "eadogs.h"
#include "max6675.h"
#include "encoder.h"
#include "eeprom.h"
#include "usb/start_boot.h"
#include "usb/usb_drv.h"
#include "usb/usb_standard_request.h"
#include "usb/wdt_drv.h"

//=============================================
//===				PGM constants			===
//=============================================

prog_char Pbsn[]   = "PbSn solder";
prog_char Pbfree[] = "Pb-free solder";
prog_char Baking[] = "Chip baking";
prog_char Drying[] = "Film drying";
prog_char User[]   = "User profile ";
prog_char Manual[] = "Manual ctrl ";
prog_char On[] = "on";
prog_char Off[] = "off";
//prog_char dot[] = ".";
//prog_char dotdot[] = ":";



uint8_t Status_com, Status_task, I;
static uint8_t Heat0, Heat1;
//static uint16_t Temp0;
uint16_t Temp1;
static uint16_t PTemp;
volatile uint8_t INT0_count;
volatile uint8_t PCINT1_count;
volatile uint8_t PCINT1_count2;
static uint16_t Time;
uint8_t Table;
struct PID_DATA PidData;

//void task_no_usb(void);
void task_with_usb(void);


//ovladanie kurenia metoda 1:
//pocita interrupt od neusmerneho signalu = 50Hz
//heater zapnuty tolko period, kolko je hodnota Heatx, zvysok do 100 vypnuty... 

//=============================================
//===				Interrupts  			===
//=============================================


ISR(PCINT1_vect)
{
//	if(!(HEAT_PIN & _BV(PHASE)))
		PCINT1_count++;
		PCINT1_count2++;
}

//=============================================

ISR(TIMER0_OVF_vect)
{
	INT0_count++;
}

//=============================================
//===				Functions   			===
//=============================================

void task_init(void)
{
	MAX_Init_hw();
	Status_task=0;
	Status_com=0;
	INT0_count = 0;
	PCINT1_count = 0;		//used only for method 1
	// defined other way in usb.c
//	fdevopen((int (*)(char, FILE*))(USB_putchar),(int (*)(FILE*))usb_getchar); //for printf redirection 
	Time = 0;
	EE_init_table();
	pid_Init(&PidData);
	EE_get_temp(0,0);
}

//=============================================

void task(void)
{	
	static uint8_t i=0, h, m, s;
//	static uint32_t temp0_local=0;
	static uint32_t temp1_local=0;
// from task_no_usb
	uint8_t encoder, menu;
	uint16_t temp_local, eta, num;
	prog_char *line;
	
	if(INT0_count >= 14) 			//temp average
	{
		SPCR |= _BV(SPE);			//enable SPI
		INT0_count = 0;
//		temp0_local += MAX_Read_temp(0);
		temp1_local += MAX_Read_temp(1);
		i++;
		if(i == 4)
		{
//			Temp0 = (uint16_t)temp0_local>>2;
			Temp1 = (uint16_t)temp1_local>>2;
	//		temp0_local = 0;
			temp1_local = 0;
			i = 0;
		}
		SPCR &= ~(_BV(SPE));			//disable SPI 
	}

	if(Is_device_enumerated())
	{
		task_with_usb();
		LED1_ON;
	}
	else
	{
		LED1_OFF;
	}
	
//	task_no_usb();
//}

/*---------------------------------------------------*/
/*--  task with functions if USB is not enumerated --*/
/*---------------------------------------------------*/
//void task_no_usb(void)
//{
//	uint8_t encoder, menu;
//	uint16_t temp_local = 0;

	encoder = Enc_GetKey(0);

	if (Status_task & TASK_MENU)
	{
		Menu_simple(encoder);
	}
	else
	{
		switch(encoder)
		{
			case KEYLEFT:	if(!(Status_task & TASK_GO))
							{
								if(Table != 0)
									Table--;
								if(Status_task & TASK_MAN)
									Status_task &= ~TASK_MAN;
							}
							else
							{
								if(PTemp)
									PTemp -= 4;
							}
							break;
			case KEYRIGHT:	if(!(Status_task & TASK_GO))
							{
								Table++;
								if(Table >= 10)
								{
									Status_task |= TASK_MAN;
									Table = 10;
								}
							}
							else
								PTemp += 4;
							break;
			case KEYSWITCH:	
							Status_task |= TASK_SW;
							break;
			case KEYSTOP:	
							Status_task &= ~TASK_GO;
			default:		break;
		}
	}

	if(Status_task & TASK_GO)
	{
		if(PCINT1_count < Heat0)			//method 1 only
		{
			HEAT1_ON;
			LED2_ON;
		}
		else
		{
			HEAT1_OFF;
			LED2_OFF;
		}

		if(PCINT1_count < Heat1)
			HEAT2_ON;
		else
			HEAT2_OFF;
	
		if(PCINT1_count >= 100)
		{
			PCINT1_count = 0;

			if(EE_test_table(Table) || (Status_task & TASK_MAN))		//if table not empty or manual control
			{
				LED_PORT = (LED_PORT & _BV(LED0)) ^ _BV(LED0);			//toggle LED0
				Time++;
				eta = 0;
				if(Status_task & TASK_MAN)
					temp_local = PTemp;
				else
				{
					temp_local = EE_get_temp(Time, Table);
					eta = EndTime - Time;
				}
				if(temp_local)
				{
					Heat0 = pid_Controller(temp_local, Temp1, &PidData);
					Heat1 = Heat0;
				}
				GLCD_Locate(0,3);
				fprintf_P(&LCDout,PSTR("Preset:%3d.%02d`C\n\nHeater:%3d%%\n"),temp_local>>2,(temp_local & 0x0003)*25, Heat0);
				if(eta)
				{
					s = eta%60;
					m = eta/60;
					h = m/60;
					fprintf_P(&LCDout, PSTR("ETA   :%02d:%02d:%02d\n"),h,m,s);
				}
				if(Is_device_enumerated())
				{
					fprintf_P(&USBout,PSTR("D %3d\t%3d\t%3d"), Time, temp_local>>2, Temp1>>2);
					if (Status_com & DEBUG)
					{
						fprintf_P(&USBout, PSTR("\t%3d\t%d\t%d\t%d"),Heat0,PidData.p_term,PidData.i_term,PidData.d_term);
					}
					usb_newline();
				}
			}
		}
	}
	else
	{
		Heat0 = 0;
		Heat1 = 0;
		HEAT1_OFF;
		HEAT2_OFF;
		Time = 0;
		EE_get_temp(0,0);
		LED0_ON;
		LED2_OFF;
	}

	
	if(encoder == KEYSWITCH)
	{
		if(PCINT1_count2 >= 100)
		{
			PCINT1_count2 = 0;
			menu++;
		}
		if(menu > 2)
		{
			LED3_ON;
		}
	}
	else
	{
		if (menu > 2)
			Status_task |= (TASK_MENU | TASK_MENUSTART);

		if(!(Status_task & TASK_MENU))
		{
			if(Status_task & TASK_SW)
			{
				Status_task |= TASK_GO;
			}
			Status_task &= ~TASK_SW;
		}
		menu = 0;
	}
	
	if(PCINT1_count2 >= 50)
	{
		PCINT1_count2 = 0;
//		GLCD_Clr();
		GLCD_Locate(0,0);
		num = 0;
		switch(Table)
		{
			case 0:	
					line = Pbsn;
					break;
			case 1: 
					line = Pbfree;
					break;
			case 2: line = Baking;
					break;
			case 3: line = Drying;
					break;
					
			case 10: line = Manual;
					break;
			default: line = User;
					num = Table - 3;
					break;
		}
		if(num)
		{
			fprintf_P(&LCDout, line);
			fprintf_P(&LCDout, PSTR(" %d"), num);
		}
		else
			fprintf_P(&LCDout, line);

		fprintf_P(&LCDout, PSTR("\n\nTemp  :%3d.%02d`C\n"),Temp1>>2,(Temp1&0x0003)*25);
		if (!(Status_task & TASK_GO))
		{
			if(!(Status_task & TASK_MAN) && (EE_test_table(Table) == 0))
			{
				fprintf_P(&LCDout, PSTR("\n Enter data\n in table\n\n"));
			}
			else 
			{
				fprintf_P(&LCDout, PSTR("\n\n\n\n"));
			}
		}
	}
	return;
}

/*---------------------------------------------------*/
/*--    task with functions if USB is enumerated   --*/
/*---------------------------------------------------*/

void task_with_usb()
{
	static uint8_t command[10];
	static uint8_t volume = 0x10;
	uint16_t num;
	prog_char *line;
	prog_char *line2;

	if(usb_test_hit())
	{
		while(rx_counter)
		{
			command[I]=usb_getchar();
			if(command[I]==0x0d)
			{
				Status_com |= COMMAND;
				I=0;
				break;
			}
			I++;
		}
	}
	
	if(Status_com & COMMAND)
	{
		Status_com &= ~COMMAND;
		if(Status_com & ECHO)
		{
			usb_putchar(command[0]);
		}
		switch(command[0])
		{
			case '?':
						if(Status_task & TASK_MAN)
						{
							line = Manual;
							num = PTemp>>2;
						}
						else
						{
							switch(Table)
							{
								case 0:	line = Pbsn;
										break;
								case 1: line = Pbfree;
										break;
								case 2: line = Baking;
										break;
								case 3: line = Drying;
										break;
								default: line = User;
										num = Table-3;
										break;
							}
						}
						if(Status_com & ECHO)
							line2 = On;
						else
							line2 = Off;

						fprintf_P(&USBout,PSTR("\r\nTemp1: %3d.%02d\nHeat0: %3d%%\nTemp. profile: %d\t"), Temp1>>2,(Temp1&0x03)*25, Heat0, Table);
						fprintf_P(&USBout,line);
						fprintf_P(&USBout,PSTR(" %d\nEcho : "), num);
						fprintf_P(&USBout,line2);
						usb_newline();
						break;
			case 'a':
			case 'A':	if(Status_com & DEBUG)
						{
							Status_com &= ~DEBUG;
							line2 = Off;
						}
						else
						{
							line2 = On;
							Status_com |= DEBUG;
						}
						fprintf_P(&USBout,PSTR("Debug : "),line2);
						fprintf_P(&USBout,line2);
						usb_newline();
						break;
			case 'b':
			case 'B': 	//pprintf_P(PSTR("Switching to DFU mode"));
						TCCR0B = 0;				// stop timer 0
						TCCR1B = 0;				// stop timer 1
						TIMSK0 = 0;				// stop interrupt from timer 0
						goto_boot();
						break;
			case 'd':
			case 'D':	if(command[1]==0x0d)
						{
							fprintf_P(&USBout,PSTR("PID D term: %d\n"), PidData.D_Factor);
							break;
						}
						PidData.D_Factor = 100*(command[1]-48) + 10*(command[2]-48) + (command[3]-48);
						break;
			case 'e':
			case 'E':	if(Status_com & ECHO)
						{
							Status_com &= ~ECHO;
							line2 = Off;
						}
						else
						{
							Status_com |= ECHO;
							line2 = On;
						}
						fprintf_P(&USBout, PSTR("Echo : "),line2);
						fprintf_P(&USBout, line2);
						usb_newline();
						break;
			case 'f':
			case 'F':	Status_task &= ~TASK_GO;
						fprintf_P(&USBout,PSTR("STOP\n"));
						break;
			case 'h':
			case 'H':	if(command[1]==0x0d)
						{
							fprintf_P(&USBout,PSTR("Heater %d%%\n"),Heat0);
							break;
						}
						switch(command[1])
						{
							case '0':	Heat0 = 10*(command[2]-48) + (command[3]-48);
//										OCR1B = (uint16_t)Heat0 << 8;	//mode 3
										break;
							case '1':	Heat1 = 10*(command[2]-48) + (command[3]-48);
//										OCR1A = (uint16_t)Heat1 << 8;	//mode 3
										break;
							default :	break;
						}
			case 'i':
			case 'I':	if(command[1]==0x0d)
						{
							fprintf_P(&USBout,PSTR("PID 1/I term: %d\n"), PidData.I_Factor);
							break;
						}
						PidData.I_Factor = 100*(command[1]-48) + 10*(command[2]-48) + (command[3]-48);
						break;
			case 'l':
						if(command[1]==0x0d)
						{
							fprintf_P(&USBout, PSTR("Contrast: %d\n"),volume);
							break;
						}
						volume = 10*(command[1]-48) + (command[2]-48);
						if (volume > 63)
							volume = 63;
						eadogs_cmd_write16(GLCD_SET_ELEC_VOLUME, volume, 1);
						break;
			case 'm':
			case 'M':	if(Status_task & TASK_MAN)
						{
							Status_com &= ~TASK_MAN;
							fprintf_P(&USBout,PSTR("Automatic ctrl\n"));
						}
						else
						{
							Status_com |= TASK_MAN;
							fprintf_P(&USBout,PSTR("Manual ctrl\n"));
						}
						break;
			case 'p':
			case 'P':	if(command[1]==0x0d)
						{
							fprintf_P(&USBout,PSTR("PID P term: %d\n"), PidData.P_Factor);
							break;
						}
						PidData.P_Factor = 100*(command[1]-48) + 10*(command[2]-48) + (command[3]-48);
						break;
			case 'S':	EE_save_pid(&PidData);
						fprintf_P(&USBout,PSTR("PID saved\n\r"));
						break;
			case 's':	Status_task |= TASK_GO;
						break;
			case 't':
			case 'T':	if(command[1]==0x0d)
						{
							fprintf_P(&USBout,PSTR("Preset temp: %3d\n"), PTemp>>2);
							break;
						}
						PTemp = 100*(command[1]-48) + 10*(command[2]-48) + (command[3]-48);
						PTemp = PTemp<<2;
						break;
						
			default :	break;
		}
	}
}

/*---------------------------------------------------*/
/*--    	send number over usb  				   --*/
/*---------------------------------------------------*/
/*
void printnum(int16_t num, uint8_t device)
{
	int16_t c;
    
	if (num < 0)
	{
			if(device == USB_DEF)
				usb_putchar('-');
			else
				GLCD_Putchar('-');
          num = -num;
	}
    c = num / 10;
    if (c)
        printnum(c, device);
		
	if (device == USB_DEF)
		usb_putchar('0' + ((uint8_t) (num % 10)));
	else
		GLCD_Putchar('0' + ((uint8_t) (num % 10)));
}
*/
/*---------------------------------------------------*/
/*--    send text from pgm mem over usb_newline    --*/
/*---------------------------------------------------*/
/*
void pprintf_P(PGM_P txt_P, uint8_t device)
{
    uint8_t c;
	
    while (c = pgm_read_byte(txt_P++))
	{
		if(device == USB_DEF)
			usb_putchar(c);
		else
			GLCD_Putchar(c);
	}
	return;
}
*/

/*---------------------------------------------------*/
/*--    new line sent over USB  				   --*/
/*---------------------------------------------------*/

void usb_newline(void)
{
    usb_putchar('\r');
    usb_putchar('\n');
}
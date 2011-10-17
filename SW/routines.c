// routines.c
//#include <stdio.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "hw.h"
#include "routines.h"
#include "spi.h"
#include "led.h"
#include "pwm.h"
#include "usb.h"
#include "pid.h"
#include "glcd.h"
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

PGM_P Pbsn = "PbSn solder";
PGM_P Pbfree = "Pb-free solder";
PGM_P Baking = "Chip baking";
PGM_P Drying = "Film drying";
PGM_P User = "User profile";
PGM_P Manual = "Manual ctrl ";

uint8_t Status_com, Status_task;
static uint8_t I;
static uint8_t Heat0, Heat1;
//static uint16_t Temp0;
uint16_t Temp1;
static uint16_t PTemp;
static uint16_t a;
volatile uint8_t INT0_count;
volatile uint8_t PCINT1_count;
static uint16_t Time;
uint8_t Table;
struct PID_DATA PidData;

void task_no_usb(void);
void task_with_usb(void);

//ovladanie kurenia metoda 1:
//pocita interrupt od neusmerneho signalu = 50Hz
//heater zapnuty tolko period, kolko je hodnota Heatx, zvysok do 100 vypnuty... 
ISR(PCINT1_vect)
{
//	if(!(HEAT_PIN & _BV(PHASE)))
		PCINT1_count++;
}

ISR(TIMER0_OVF_vect)
{
	INT0_count++;
}

void task_init(void)
{
	MAX_Init_hw();
	I=0;
	a=0;
	Status_task=0;
	Status_com=0;
	INT0_count = 0;
	PCINT1_count = 0;		//used only for method 1
	// defined other way in usb.c
//	fdevopen((int (*)(char, FILE*))(USB_putchar),(int (*)(FILE*))usb_getchar); //for printf redirection 
	Time = 0;
	pid_Init(K_P, K_I, K_D, &PidData);
	EE_init_table();
}

void task(void)
{	
	static uint8_t i=0;
//	static uint32_t temp0_local=0;
	static uint32_t temp1_local=0;
	
	if(INT0_count == 14) 			//temp average
	{
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
			LED_PORT = (LED_PORT & _BV(LED2)) ^ _BV(LED2);		//toggle LED2 
		}
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
	
	task_no_usb();
}

/*---------------------------------------------------*/
/*--  task with functions if USB is not enumerated --*/
/*---------------------------------------------------*/
void task_no_usb(void)
{
	uint8_t encoder;
	uint16_t temp_local = 0;

	encoder = Enc_GetKey(0);
	switch(encoder)
	{
		case KEYLEFT:	if(Table != 0)
							Table--;
						if(Status_task & TASK_MAN)
							Status_task &= ~TASK_MAN;
						break;
		case KEYRIGHT:	if(Table == 9)
						{
							Status_task |= TASK_MAN;
							Table = 10;
						}
						else
							Table++;
						break;
		case KEYSWITCH:	
						Status_task |= TASK_GO;
						break;
		case KEYSTOP:	
						Status_task &= ~TASK_GO;
		default:		break;
	}

	if(Status_task & TASK_GO)
	{
		if(PCINT1_count < Heat0)			//method 1 only
			HEAT1_ON;
		else
			HEAT1_OFF;

		if(PCINT1_count < Heat1)
			HEAT2_ON;
		else
			HEAT2_OFF;
	
		if(PCINT1_count == 100)
		{
			PCINT1_count = 0;
			Time++;
			if(Status_task & TASK_MAN)
				temp_local = PTemp;
			else
				temp_local = EE_get_temp(Time, Table);
			Heat0 = pid_Controller(temp_local, Temp1, &PidData);
			Heat1 = Heat0;
			if(Is_device_enumerated())
			{
				pprintf_P(PSTR("D "), USB);
				printnum(Time, USB);
				usb_putchar('\t');
				printnum(temp_local>>2, USB);
				usb_putchar('\t');
				printnum(Temp1>>2, USB);
				if (Status_com & DEBUG)
				{
					usb_putchar('\t');
					printnum(Heat0, USB);
					usb_putchar('\t');
					printnum(PidData.p_term, USB);
					usb_putchar('\t');
					printnum(PidData.i_term, USB);
					usb_putchar('\t');
					printnum(PidData.d_term, USB);
				}
				usb_newline();
			}
			GLCD_Clr();
			GLCD_Setxy(0,0);
			switch(Table)
			{
				case 0:	pprintf_P(Pbsn, LCD);
						break;
				case 1: pprintf_P(Pbfree, LCD);
						break;
				case 2: pprintf_P(Baking, LCD);
						break;
				case 3: pprintf_P(Drying, LCD);
						break;
				case 11: pprintf_P(Manual, LCD);
						break;
				default: pprintf_P(User, LCD);
						break;
			}
				pprintf_P(PSTR("/n"), LCD);
		}
	}
	else
	{
		Heat0 = 0;
		Heat1 = 0;
		HEAT1_OFF;
		HEAT2_OFF;
		Time = 0;
	}

}

/*---------------------------------------------------*/
/*--    task with functions if USB is enumerated   --*/
/*---------------------------------------------------*/
void task_with_usb()
{
	static uint8_t command[10];
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
//						pprintf_P(PSTR("\r\nTemp0: %3d.%02d\t"), Temp0>>2,(Temp0&0x03)*25);
						pprintf_P(PSTR("Temp1: "), USB);
						printnum(Temp1>>2, USB);
						usb_putchar('.');
						printnum((Temp1&0x03)*25, USB);
						usb_newline();
						pprintf_P(PSTR("Heat0: "), USB);
						printnum(Heat0, USB);
						pprintf_P(PSTR("\tHeat1: "), USB);
						printnum(Heat1, USB);
						usb_newline();
						pprintf_P(PSTR("Temp. profile: "), USB);
						printnum(Table, USB);
						usb_putchar('\t');
						if(Status_task & TASK_MAN)
						{
							pprintf_P(Manual, USB);
							printnum(PTemp>>2, USB);
						}
						else
							switch(Table)
							{
								case 0:	pprintf_P(Pbsn, USB);
										break;
								case 1: pprintf_P(Pbfree, USB);
										break;
								case 2: pprintf_P(Baking, USB);
										break;
								case 3: pprintf_P(Drying, USB);
										break;
								default: pprintf_P(User, USB);
										break;
							}
							usb_newline();
						pprintf_P(PSTR("Echo : "), USB);
						if(Status_com & ECHO)
						{
							pprintf_P(PSTR("ON"), USB);
						}
						else
						{
							pprintf_P(PSTR("OFF"), USB);
						}
						usb_newline();
						break;
			case 'b':
			case 'B': 	//pprintf_P(PSTR("Switching to DFU mode"));
						TCCR0B = 0;				// stop timer 0
						TCCR1B = 0;				// stop timer 1
						TIMSK0 = 0;				// stop interrupt from timer 0
						goto_boot();
						break;
			case 'e':
			case 'E':	if(Status_com & ECHO)
						{
							Status_com &= ~ECHO;
							pprintf_P(PSTR("echo off"), USB);
							usb_newline();
						}
						else
						{
							Status_com |= ECHO;
							pprintf_P(PSTR("echo on"), USB);
							usb_newline();
						}
						break;
			case 'h':
			case 'H':	if(command[1]==0x0d)
							break;
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
			case 'd':
			case 'D':	if(Status_com & DEBUG)
						{
							Status_com &= ~DEBUG;
							pprintf_P(PSTR("debug off"), USB);
							usb_newline();
						}
						else
						{
							Status_com |= DEBUG;
							pprintf_P(PSTR("debug on"), USB);
							usb_newline();
						}
						break;
			case 'm':
			case 'M':	if(Status_task & TASK_MAN)
						{
							Status_com &= ~TASK_MAN;
							pprintf_P(PSTR("Automatic ctrl"), USB);
							usb_newline();
						}
						else
						{
							Status_com |= TASK_MAN;
							pprintf_P(PSTR("Manual ctrl"), USB);
							usb_newline();
						}
						break;
			case 's':
			case 'S':	Status_task |= TASK_GO;
						break;
			case 'f':
			case 'F':	Status_task &= ~TASK_GO;
						break;
			case 't':
			case 'T':	if(command[1]==0x0d)
						{
							pprintf_P(PSTR("Preset temp: "), USB);
							printnum(PTemp>>2, USB);
							usb_newline();
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
/*--    send number over usb_newline			   --*/
/*---------------------------------------------------*/

void printnum(int16_t num, uint8_t device)
{
    int16_t c;
    
    if (num < 0)
    {
			if(device == USB)
				usb_putchar('-');
			else
				lcd_putchar('-');
            num = -num;
    }
    c = num / 10;
    if (c)
        printnum(c, device);
		
	if (device == USB)
		usb_putchar('0' + ((uint8_t) (num % 10)));
	else
		lcd_putchar('0' + ((uint8_t) (num % 10)));
}

/*---------------------------------------------------*/
/*--    send text from pgm mem over usb_newline    --*/
/*---------------------------------------------------*/

void pprintf_P(PGM_P txt_P, uint8_t device)
{
    uint8_t c;
    
    while (c = pgm_read_byte(txt_P++))
	{
		if(device == USB)
			usb_putchar(c);
		else
			lcd_putchar(c);
	}
}

/*---------------------------------------------------*/
/*--    new line sent over USB  				   --*/
/*---------------------------------------------------*/

void usb_newline(void)
{
    usb_putchar('\r');
    usb_putchar('\n');
}
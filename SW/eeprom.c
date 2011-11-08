#include <stdint.h>
#include <util/atomic.h>
#include <avr/eeprom.h>

#include <avr/pgmspace.h>

#include "eeprom.h"
#include "usb.h"
#include "routines.h"
#include "pid.h"


uint8_t EEMEM EEtable[10];
/* = 
{{5,30},{5,50},{3,70},{3,82},{0,94}};
*/

temp_point EEMEM EEtemp[10][8];
/* = 
{{0x0078,0x0258},{0x00f0,0x02dc},{0x012c,0x0384},{0x0140,0x0384},{0x01c2,0x0064},
 {0x0078,0x02D0},{0x00f0,0x0364},{0x012c,0x03E8},{0x014A,0x03E8},{0x01c2,0x0064},
 {0x0078,0x01F4},{0xA8C0,0x01F4},{0xA8FC,0x0064},
 {0x0078,0x0244},{0x0E10,0x0244},{0x0E4C,0x0064}
};
*/

int16_t EEMEM K_I;
int16_t EEMEM K_P;
int16_t EEMEM K_D;

static uint16_t next_time;
static uint16_t last_time;
static uint16_t next_temp;
static uint16_t last_temp;
static int16_t SR;


void EE_init_table(void)
{
	uint8_t pts;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		eeprom_busy_wait();
		pts = eeprom_read_byte(&(EEtable[0]));
	}
	
	if (pts != 0xFF)		//if first entry filled - already initialised
		return;
	
	//initialising temperature table
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[0],5);
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[1],5);
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[2],3);
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[3],3);
		for (pts=4;pts<10;pts++)
		{
			eeprom_busy_wait();
			eeprom_write_byte(&EEtable[pts],0);
		}
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][0].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][0].temp, 150<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][1].time, 240);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][1].temp, 183<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][2].time, 300);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][2].temp, 225<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][3].time, 320);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][3].temp, 225<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][4].time, 450);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0][4].temp, 25<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][0].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][0].temp, 180<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][1].time, 240);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][1].temp, 217<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][2].time, 300);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][2].temp, 250<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][3].time, 330);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][3].temp, 250<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][4].time, 450);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1][4].temp, 25<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2][0].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2][0].temp, 125<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2][1].time, 43200);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2][1].temp, 125<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2][2].time, 43260);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2][2].temp, 25<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3][0].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3][0].temp, 145<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3][1].time, 3600);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3][1].temp, 145<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3][2].time, 3660);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3][2].temp, 25<<2);

		eeprom_busy_wait();
		eeprom_write_word(&K_I, 100);
		eeprom_busy_wait();
		eeprom_write_word(&K_P, 4);
		eeprom_busy_wait();
		eeprom_write_word(&K_D, 50);

	}
	
}

uint16_t EE_get_temp(uint16_t time, uint8_t table_number)
{
	static uint8_t point = 0;
	static uint8_t points = 0;

  if(time == 0)        // if time 0 - reset.
  {
    point = 0;
    points = 0;
    SR = 0;
    next_time = 0;
    next_temp = 25<<2;
	return(0);
  } 
  
	if(table_number > 9)
		return(0);
	if(points == 0)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{		
			eeprom_busy_wait();
			points = eeprom_read_byte((uint8_t*)&(EEtable[table_number]));
//test			
			printnum(points,USB_DEF);
		}
		point = 0;
		next_time = 0;
		next_temp = 25<<2;
		SR = 0;
	}
	
	if(time >= next_time)
	{
		if(point == points)		//if last time point is reached
		{
			Status_task &= ~TASK_GO;			// stop heating
			point = 0;
			points = 0;
			next_time = 0;
			next_temp = 25<<2;
			return(0);
		}
		last_time = next_time;
		last_temp = next_temp;
		eeprom_busy_wait();
		next_time = eeprom_read_word((uint16_t*)&EEtemp[table_number][point].time);
		eeprom_busy_wait();
		next_temp = eeprom_read_word((uint16_t*)&EEtemp[table_number][point].temp);
		SR = (int16_t)((((int32_t)next_temp - last_temp)<<7)/(next_time - last_time));
		point++;
//test
		if(Status_com & DEBUG)
		{
			pprintf_P(PSTR("Setting: "), USB_DEF);
			printnum(next_time, USB_DEF);
			usb_putchar('\t');
			printnum(next_temp>>2, USB_DEF);
			usb_putchar('\t');
			printnum(SR, USB_DEF);
			usb_newline();
		}
	}
	
	return((uint16_t)(((int32_t)(time-last_time) * SR)>>7) + last_temp);
	
}

void EE_save_pid(pidData_t *pid_st)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		eeprom_busy_wait();
		eeprom_write_word(&K_I, pid_st->I_Factor);
		eeprom_busy_wait();
		eeprom_write_word(&K_P, pid_st->P_Factor);
		eeprom_busy_wait();
		eeprom_write_word(&K_D, pid_st->D_Factor);
	}
}

void EE_load_pid(pidData_t *pid_st)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		eeprom_busy_wait();
		pid_st->I_Factor = eeprom_read_word(&K_I);
		eeprom_busy_wait();
		pid_st->P_Factor = eeprom_read_word(&K_P);
		eeprom_busy_wait();
		pid_st->D_Factor = eeprom_read_word(&K_D);
	}
}
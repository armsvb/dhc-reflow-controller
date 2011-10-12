#include <stdint.h>
#include <util/atomic.h>
#include <avr/eeprom.h>

#include <avr/pgmspace.h>

#include "eeprom.h"
#include "usb.h"
#include "routines.h"


table EEMEM EEtable[10];
/* = 
{{5,30},{5,50},{3,70},{3,82},{0,94}};
*/

temp_point EEMEM EEtemp[80];
/* = 
{{0x0078,0x0258},{0x00f0,0x02dc},{0x012c,0x0384},{0x0140,0x0384},{0x01c2,0x0064},
 {0x0078,0x02D0},{0x00f0,0x0364},{0x012c,0x03E8},{0x014A,0x03E8},{0x01c2,0x0064},
 {0x0078,0x01F4},{0xA8C0,0x01F4},{0xA8FC,0x0064},
 {0x0078,0x0244},{0x0E10,0x0244},{0x0E4C,0x0064}
};
*/

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
		pts = eeprom_read_byte(&(EEtable[0].PTS));
	}
	
	if (pts != 0xFF)		//if first entry filled - already initialised
		return;
	
	//initialising temperature table
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[0].PTS,5);
		eeprom_busy_wait();
		eeprom_write_word(&EEtable[0].EEaddress,(uint16_t)(&EEtemp[0]));
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[1].PTS,5);
		eeprom_busy_wait();
		eeprom_write_word(&EEtable[1].EEaddress,(uint16_t)(&EEtemp[5]));
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[2].PTS,3);
		eeprom_busy_wait();
		eeprom_write_word(&EEtable[2].EEaddress,(uint16_t)(&EEtemp[10]));
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[3].PTS,3);
		eeprom_busy_wait();
		eeprom_write_word(&EEtable[3].EEaddress,(uint16_t)(&EEtemp[13]));
		eeprom_busy_wait();
		eeprom_write_byte(&EEtable[4].PTS,0);
		eeprom_busy_wait();
		eeprom_write_word(&EEtable[4].EEaddress,(uint16_t)(&EEtemp[16]));

		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[0].temp, 150<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1].time, 240);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[1].temp, 183<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2].time, 300);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[2].temp, 225<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3].time, 320);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[3].temp, 225<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[4].time, 450);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[4].temp, 25<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[5].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[5].temp, 180<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[6].time, 240);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[6].temp, 217<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[7].time, 300);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[7].temp, 250<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[8].time, 330);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[8].temp, 250<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[9].time, 450);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[9].temp, 25<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[10].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[10].temp, 125<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[11].time, 43200);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[11].temp, 125<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[12].time, 43260);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[12].temp, 25<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[13].time, 120);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[13].temp, 145<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[14].time, 3600);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[14].temp, 145<<2);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[15].time, 3660);
		eeprom_busy_wait();
		eeprom_write_word(&EEtemp[15].temp, 25<<2);

	}
	
}

uint16_t EE_get_temp(uint16_t time, uint8_t table_number)
{
	static uint8_t point = 0;
	static uint8_t points = 0;
	static uint16_t eetable_ptr;
	
	if(table_number > 9)
		return(0);
	if(points == 0)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{		
			eeprom_busy_wait();
			points = eeprom_read_byte((uint8_t*)&(EEtable[table_number].PTS));
			eeprom_busy_wait();
			eetable_ptr = eeprom_read_word((uint16_t*)&(EEtable[table_number].EEaddress));
		}
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
		next_time = eeprom_read_word((uint16_t*)eetable_ptr);
		eeprom_busy_wait();
		next_temp = eeprom_read_word((uint16_t*)(eetable_ptr + 2));
		SR = (int16_t)(((int32_t)(next_temp - last_temp)<<7)/(next_time - last_time));
		eetable_ptr += 4;
		point++;
		if(Status_com & DEBUG)
		{
			usb_printf_P(PSTR("Setting: "));
			usb_printnum(next_time);
			usb_putchar('\t');
			usb_printnum(next_temp>>2);
			usb_newline();
		}
	}
	
	return((uint16_t)(((int32_t)(time-last_time) * SR)>>7) + last_temp);
	
}
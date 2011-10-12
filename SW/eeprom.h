/*
 * eeprom.h
 *
 *  Created on: 2009-09-03
 *      Author: liku
 *
 *  This file contains locations for storing variables which should be
 *  persistent over CPU poweroffs.
 *
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>

#include <avr/eeprom.h>


/* fix for eclipse - remove for final version */
#undef EEMEM
#define EEMEM __attribute__((section(".eeprom")))



typedef struct TEMP_POINT
{
	uint16_t time;		// next time point
	uint16_t temp;		// temp at that time
//	uint8_t SR;			// temperature slew rate (C/min)
} temp_point;

typedef struct TABLE
{
	uint16_t EEaddress;
	uint8_t PTS;
} table;


void EE_init_table(void);

uint16_t EE_get_temp(uint16_t time, uint8_t table_number);

#endif /* EEPROM_H_ */

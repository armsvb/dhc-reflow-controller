/*
============================================================
==                      ENCODER.C                         ==
============================================================
Macros:
	ENC_INT
	ENC_A_PORT
	ENC_A
	ENC_A_IN
	ENC_A_INT
	ENC_B_PORT
	ENC_B
	ENC_B_IN
	ENC_B_INT
	ENC_SW_PORT
	ENC_SW
	ENC_SW_PIN
*/

#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "encoder.h"
#include "hw.h"


//==========================================================
//==                    Structures						  ==
//==========================================================

//==========================================================
//==                 Global variables	  			      ==
//==========================================================

static volatile uint8_t kleft, kright, kswitch, kstop;
//static uint8_t kskip = KEYDEFENCSKIP;


//==========================================================
//==                     Routines	   			          ==
//==========================================================

void Enc_Init(void)
{
	
	//configure the knob pins as inputs
	ENC_DDR &= ~(_BV(ENC_A) | _BV(ENC_B) | _BV(ENC_SW));
	// add internal pull-ups
	ENC_PORT |= _BV(ENC_A) | _BV(ENC_B) | _BV(ENC_SW);		
	
	STOP_DDR &= ~(_BV(STOP));		//STOP button
	STOP_PORT |= _BV(STOP);
	
	//Interrupt from stop button
	EIMSK |= 0x10;		// external interrupt from stop button
	EICRB |= 0x01; 		//any logical change
	EIFR = 0xFF;
	
	PCMSK0 |= 0x70; 
	PCICR |= 0x01;
	PCIFR = 0x03;
	//reset state variables
	Enc_ClrBuf();

	return;
}

//=============================================================================
ISR(STOP_INT_VECT)
{
	if(STOP_PIN & _BV(STOP))
		kstop++;
}

//=============================================================================
ISR(PCINT0_vect)
{

	const  uint16_t patt     = 0x1e1e; // 00|01|11|10|00|01|11|10 - special pattern to recognize direction
	static uint8_t  lastpos  = 0;
	uint8_t keys, i; // current keys state

	keys = (ENC_PIN & (_BV(ENC_A) | _BV(ENC_B)))>>ENC_B; // read current encoder state and move it to LSB

	if(!(ENC_PIN & _BV(ENC_SW)))
		kswitch++;						//if sw is pressed 

	for(i=1; i<=4; i++)
	{
		if((keys & 0x03) == (patt>>(i*2) & 0x03)) // move pattern and see if matches current encoder state
		{
			// compare against last position in patters
			if(i-1 == lastpos)
				kleft++;
			else if(i+1 == lastpos)
				kright++;
			// else - probably some error - skip

			lastpos = i;
 			// job done - exit!
			break;
		}
	}

/*																// nejake makove - nefunguje ako ma...
	uint8_t keys_new, change;
	static uint8_t keys_old = 0;
	
	keys_new = (ENC_PIN & (_BV(ENC_A) | _BV(ENC_B)))>>ENC_B; // read current encoder state and move it to LSB
	
	if (keys_new == keys_old)
		return;
	
	change = (keys_old ^ keys_new);
	
	if (!(ENC_PIN & _BV(ENC_SW)))
		kswitch++;
		
	if (change & 0x02)
	{
		if((change & 0x01)==((change & 0x02)>>1))
			kright++;
		else
			kleft++;
	}
	
	if (change & 0x01)
	{
		if((change & 0x01)==((change & 0x02)>>1))
			kleft++;
		else
			kright++;
	}
	
	keys_old = keys_new;
*/
	return;
}

//=============================================================================

/*
 * gets key from buffer
 */
//============================================================================= 
int8_t Enc_GetKey(uint8_t waitforkey)
{
	int8_t value = 0;
	
	if(waitforkey)
	{
		while(	(kleft < KEYDEFENCSKIP) &&
				(kright < KEYDEFENCSKIP) &&
				(!kswitch));

		_delay_ms(KEYREPEATRATE);
	}

	if(kright > KEYDEFENCSKIP)
	{
		kright = 0;
		value = KEYRIGHT;
	}
	else if(kleft > KEYDEFENCSKIP)
	{
		kleft = 0;
		value = KEYLEFT;
	}
	if(kswitch)
	{
		kswitch = 0;
		value |= KEYSWITCH;
	}
	if(kstop)
	{
		kstop = 0;
		value |= KEYSTOP;
	}
	return value;
}

/*
 * clears keyboard buffer
 */
//=============================================================================
void Enc_ClrBuf(void)
{
	kright = kleft = kswitch = kstop = 0;
}

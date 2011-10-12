
#include <stdio.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/sleep.h>
//#include <avr/eeprom.h>

#include "hw.h"

#include "led.h"
#include "pwm.h"
#include "routines.h"
#include "encoder.h"
#include "glcd.h"
#include "usb.h"
#include "usb/usb_drv.h"
#include "usb/usb_standard_request.h"
#include "usb/wdt_drv.h"

//-------------------------------------
//			Global variables
//-------------------------------------



//-------------------------------------
//			Prototypes
//-------------------------------------

void HW_Init(void);
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

//-------------------------------------
//          MAIN
//-------------------------------------


int main(void)
{
	
	HW_Init();
	GLCD_Init();
	USB_Init_hw();
	Enc_Init();
	task_init();

	sei();

	while(1)
	{
		USB_device_task();
		task();
	}

	return(0);
}

void HW_Init(void)
{
	//clk 16MHZ - set the system clk prescaler to 0
	CLKPR = _BV(CLKPCE);		// enable change
	CLKPR = 0x00;				// div by 1
	
	LED_DDR  |= (_BV(LED0)|_BV(LED1)|_BV(LED2)|_BV(LED3));
	LED_PORT |= (_BV(LED0)|_BV(LED1)|_BV(LED2)|_BV(LED3));
	
	HEAT_DDR |= (_BV(HEAT1)|_BV(HEAT2));
	HEAT_DDR &= ~(_BV(PHASE));		// PHASE pin as input
	HEAT_PORT |= _BV(PHASE);		//pull-up on PHASE pin
	FAN_DDR |= _BV(FAN);
	FAN_OFF;
	HEAT1_OFF;
	HEAT2_OFF;
	
	
// setup of timer0 - interrupt will run USB_task()
	TCNT0 = 0x00;
//	OCR0A = 62;
	TCCR0A = 0x00;			//normal operation of timer
	TCCR0B = 0x05;			//counting to measure 0.22s for MAX6675 to complete conversion
	TIMSK0 = _BV(TOIE0);	//overflow interrupt; count to 14 to have 220ms :-(
	TIFR0 = 0x07;

	PCMSK1 = 0x04;		// heater control method 1 - PCINT10 
	PCICR |= 0x02;		// counting mains periodes repeating each 100 periodes
	PCIFR = 0x03;
	
//	TCNT1 = 0;			//heater control method 2 - tyristor control each mains periode 
//	TCCR1A = 
//	TCCR1B = 

//	TCNT1 = 0;			//heater control method 3 - slow PWM, no relation to mains periode 
//	ICR1 = 25600;
//	OCR1A = 0;
//	OCR1B = 0;

//	TCCR1A = 0xF0;		// WGM mode = 8; set on upcounting, clear on downcounting 
//	TCCR1B = 0x14;
//	TIMSK1 = 0;
//	TIFR1 = 0x2F;

//	GLCD_Init();
	
}

void wdt_init(void)
{
    MCUSR = 0;
    wdtdrv_disable();
    return;
}


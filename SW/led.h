#ifndef LED_H
#define LED_H

#include "hw.h"

//----------------------------------------------------------
//						DEFS
//----------------------------------------------------------

#ifndef GLUE
	#define GLUE(a, b)     a##b
	#define PORT(x)        GLUE(PORT, x)
	#define PIN(x)         GLUE(PIN, x)
	#define DDR(x)         GLUE(DDR, x)
#endif	//GLUE

#define LED_PORT		PORT(LED_P)
#define LED_DDR			DDR(LED_P)
#define LED_PIN			PIN(LED_P)

#define LED0_OFF		LED_PORT&=~(_BV(LED0))
#define LED1_OFF		LED_PORT&=~(_BV(LED1))
#define LED2_OFF		LED_PORT&=~(_BV(LED2))
#define LED3_OFF		LED_PORT&=~(_BV(LED3))

#define LED0_ON			LED_PORT|=_BV(LED0)
#define LED1_ON			LED_PORT|=_BV(LED1)
#define LED2_ON			LED_PORT|=_BV(LED2)
#define LED3_ON			LED_PORT|=_BV(LED3)


#endif //LED_H
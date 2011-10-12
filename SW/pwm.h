#ifndef PWM_H
#define PWM_H

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

#define FAN_PORT		PORT(FAN_P)
#define FAN_DDR			DDR(FAN_P)

#define HEAT_PORT		PORT(CTRL_P)
#define HEAT_PIN		PIN(CTRL_P)
#define HEAT_DDR		DDR(CTRL_P)


#define FAN_OFF			FAN_PORT&=~(_BV(FAN))
#define FAN_ON			FAN_PORT|=_BV(FAN)
#define HEAT1_ON		HEAT_PORT&=~(_BV(HEAT1))
#define HEAT1_OFF		HEAT_PORT|=_BV(HEAT1)
#define HEAT2_ON		HEAT_PORT&=~(_BV(HEAT2))
#define HEAT2_OFF		HEAT_PORT|=_BV(HEAT2)


#endif //PWM_H
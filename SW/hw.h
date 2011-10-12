#ifndef HW_H
#define HW_H

#include "usb/compiler.h"
#include <inttypes.h>

#define FOSC				16000
#define LITTLE_ENDIAN

#define FONT_5X8

#define K_P     3
#define K_I     20
#define K_D     50
#define IMIN	-127
#define IMAX	255
#define AWR		256
#define HMIN	0
#define HMAX	200

#define SPI_P				B
#define SPI_MOSI			PB2
#define SPI_MISO			PB3
#define SPI_SS				PB0
#define SPI_SCK				PB1

#define FAN_P				B
#define FAN					PB7
#define FAN_PWM				OC1C

#define ENC_P				B
#define ENC_A				PB6
#define ENC_B				PB5
#define ENC_SW				PB4

#define STOP_P				C
#define STOP				PC7
#define STOP_INT			INT4
#define STOP_INT_VECT		INT4_vect

#define TEMP_P				D
#define	MAX0				PD6
#define	MAX1				PD7

#define LED_P				D
#define LED0				PD2
#define LED1				PD3
#define LED2				PD4
#define LED3				PD5

#define CTRL_P				C
#define HEAT1				PC5
#define HEAT1_PWM			OC1B
#define HEAT2				PC6
#define HEAT2_PWM			OC1A
#define PHASE				PC4
#define PHASE_INT			PCINT10

#define DISP_P				D
#define DISP_SCL			PD0
#define DISP_SDA			PD1
#define DISP_CS_P			C
#define DISP_CS				PC2
#define DISP_CD_P			B
#define DISP_CD				PB0

#endif //HW_H
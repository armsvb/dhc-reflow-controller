#ifndef _MAX6675_H_
#define _MAX6675_H_


#ifndef GLUE
	#define GLUE(a, b)     a##b
	#define PORT(x)        GLUE(PORT, x)
	#define PIN(x)         GLUE(PIN, x)
	#define DDR(x)         GLUE(DDR, x)
#endif	//GLUE

#define 	TEMPPORT		PORT(TEMP_P)
#define 	TEMPDDR			DDR(TEMP_P)


void MAX_Init_hw(void);

uint16_t MAX_Read_temp(uint8_t sensor);

#endif	//_MAX6675_H_
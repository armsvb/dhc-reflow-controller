#ifndef ENC_H_
#define ENC_H_


//==========================================================
//==                      Macros						  ==
//==========================================================

#ifndef GLUE
	#define GLUE(a, b)     a##b
	#define PORT(x)        GLUE(PORT, x)
	#define PIN(x)         GLUE(PIN, x)
	#define DDR(x)         GLUE(DDR, x)
#endif

#define KEYRIGHT		1
#define KEYLEFT			2
#define KEYSWITCH		4
#define KEYSTOP			8
#define KEYDEFENCSKIP 	2
#define KEYREPEATRATE	100

#define ENC_PORT	PORT(ENC_P)
#define ENC_DDR		DDR(ENC_P)
#define ENC_PIN		PIN(ENC_P)
#define STOP_PORT	PORT(STOP_P)
#define STOP_DDR	DDR(STOP_P)
#define STOP_PIN	PIN(STOP_P)


void Enc_Init(void);

int8_t Enc_GetKey(uint8_t);

void Enc_ClrBuf(void);

#endif	//ENC_H_
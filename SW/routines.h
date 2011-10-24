#ifndef ROUTINES_H
#define ROUTINES_H

// defines for Status_com
#define COMMAND				0x01
#define ECHO				0x02
#define DEBUG				0x04

// defines for Status_task
#define TASK_GO				0x01
#define TASK_MAN			0x02
#define TASK_SLCT			0x04
//#define TASK_SETTING_TIME	0x08
//#define TASK_SETTING_TEMP	0x10
#define TASK_SW				0x10
#define TASK_MENU			0x20
#define TASK_MENUSTART		0x80
#define LCD_DEF				0x00
#define USB_DEF				0x01

extern uint8_t Status_task, Status_com;
//extern uint16_t Temp1;

void task_init(void);

void task(void);

void printnum(int16_t num, uint8_t device);

void pprintf_P(PGM_P txt_P, uint8_t device);

void usb_newline(void);

#endif //ROUTINES_H
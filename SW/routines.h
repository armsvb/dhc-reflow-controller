#ifndef ROUTINES_H
#define ROUTINES_H

// defines for Status_com
#define COMMAND			0x01
#define ECHO			0x02
#define DEBUG			0x04

// defines for Status_task
#define TASK_GO			0x01
#define TASK_MAN		0x02

extern uint8_t Status_task, Status_com;
//extern uint16_t Temp1;

void task_init(void);

void task(void);

void usb_printnum(int16_t num);

void usb_printf_P(PGM_P txt_P);

void usb_newline(void);

#endif //ROUTINES_H
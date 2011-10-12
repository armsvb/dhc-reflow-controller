#ifndef _USB_H_
#define _USB_H_

#include "usb/compiler.h"
#include <stdio.h>

//_____ M A C R O S ________________________________________________________

//#define usb_putchar putchar

      // usb_software_evts USB software Events Management
      // Macros to manage USB events detected under interrupt
#define Usb_send_event(x)               (g_usb_event |= (1<<x))
#define Usb_ack_event(x)                (g_usb_event &= ~(1<<x))
#define Usb_clear_all_event()           (g_usb_event = 0)
#define Is_usb_event(x)                 ((g_usb_event & (1<<x)) ? TRUE : FALSE)
#define Is_not_usb_event(x)             ((g_usb_event & (1<<x)) ? FALSE: TRUE)

#define EVT_USB_POWERED               1         // USB plugged
#define EVT_USB_UNPOWERED             2         // USB un-plugged
#define EVT_USB_DEVICE_FUNCTION       3         // USB in device
#define EVT_USB_HOST_FUNCTION         4         // USB in host
#define EVT_USB_SUSPEND               5         // USB suspend
#define EVT_USB_WAKE_UP               6         // USB wake up
#define EVT_USB_RESUME                7         // USB resume
#define EVT_USB_RESET                 8         // USB reset

#define USB_MODE_UNDEFINED            0x00
#define USB_MODE_HOST                 0x01
#define USB_MODE_DEVICE               0x02
//from conf_usb.h
#include "usb/usb_commun.h"
#include "usb/usb_commun_cdc.h"

// _________________ USB MODE CONFIGURATION ____________________________
//

#define USB_DEVICE_FEATURE          ENABLED
 

// _________________ USB REGULATOR CONFIGURATION _______________________
//
 
#ifndef USE_USB_PADS_REGULATOR
    //#define USE_USB_PADS_REGULATOR   ENABLE      // Possible values ENABLE or DISABLE
#define USE_USB_PADS_REGULATOR   DISABLE      // Possible values ENABLE or DISABLE
#endif
 
// _________________ DEVICE MODE CONFIGURATION __________________________
 
#define USB_DEVICE_SN_USE          DISABLE            // DISABLE
#define USE_DEVICE_SN_UNIQUE       DISABLE            // ignore if USB_DEVICE_SN_USE = DISABLE
 
#define NB_ENDPOINTS          	4  
#define TX_EP                	0x01
#define RX_EP                	0x02
#define INT_EP              	0x03
 
#define USB_REMOTE_WAKEUP_FEATURE     DISABLED   

#define VBUS_SENSING_IO       DISABLED   
 
#define USB_RESET_CPU         DISABLED   
 
#define Usb_unicode(a)         ((U16)(a))
 
    // write here the action to associate to each USB event
    // be carefull not to waste time in order not disturbing the functions
#define Usb_sof_action()         sof_action();
#define Usb_wake_up_action()
#define Usb_resume_action()
#define Usb_suspend_action()
#define Usb_reset_action()
#define Usb_vbus_on_action()
#define Usb_vbus_off_action()
#define Usb_set_configuration_action()
 
extern void sof_action(void);
extern void suspend_action(void);



#define BAUDRATE        38400

//_____ D E C L A R A T I O N S ____________________________________________

extern volatile uint16_t g_usb_event;
extern uint8_t g_usb_mode;
extern bit   usb_suspended;
extern bit   usb_connected;

extern uint8_t rx_counter;
extern uint8_t tx_counter;


void USB_Init_hw (void);

void USB_start_device (void);

void USB_device_task (void);

bit   usb_test_hit(void);

uint8_t usb_getchar(void);

bit   usb_tx_ready(void);

int USB_putchar(char, FILE*);

int16_t  usb_putchar(int);

void  usb_flush(void);

void usb_send_buffer(uint8_t *buffer, uint8_t nb_data);

void sof_action(void);

extern volatile uint8_t private_sof_counter;

#endif	//_USB_H_

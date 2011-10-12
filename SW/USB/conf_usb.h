#ifndef _CONF_USB_H_
#define _CONF_USB_H_

#include "usb_commun.h"
#include "usb_commun_cdc.h"


      //! Possible values ENABLE or DISABLE
      #define USB_DEVICE_FEATURE          ENABLED

#ifndef USE_USB_PADS_REGULATOR
#define USE_USB_PADS_REGULATOR   ENABLE      // Possible values ENABLE or DISABLE
//#define USE_USB_PADS_REGULATOR   DISABLE      // Possible values ENABLE or DISABLE
#endif


// _________________ DEVICE MODE CONFIGURATION __________________________

#define USB_DEVICE_SN_USE          DISABLE            // DISABLE
#define USE_DEVICE_SN_UNIQUE       DISABLE            // ignore if USB_DEVICE_SN_USE = DISABLE

#define NB_ENDPOINTS          4  //!  number of endpoints in the application including control endpoint
#define TX_EP                0x01
#define RX_EP                0x02
#define INT_EP              0x03

#define USB_REMOTE_WAKEUP_FEATURE     DISABLED   //! don't allow remote wake up

#define VBUS_SENSING_IO       DISABLED   //! device will connect directly on reset

#define USB_RESET_CPU         DISABLED   //! an USB reset does not reset the CPU

#define Usb_unicode(a)         ((uint16_t)(a))

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

#endif // _CONF_USB_H_

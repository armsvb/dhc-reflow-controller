
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include <stdio.h>

#include "hw.h"
#include "usb/usb_commun.h"
#include "usb/usb_commun_cdc.h"
#include "usb.h"
#include "usb/usb_drv.h"
#include "usb/usb_descriptors.h"
#include "usb/usb_standard_request.h"
#include "usb/usb_specific_request.h"
#include "usb/pll_drv.h"

//_____ D E F I N I T I O N S ______________________________________________

//! Public : U16 g_usb_event
//! usb_connected is used to store USB events detected upon
//! USB general interrupt subroutine
//! Its value is managed by the following macros (See usb_task.h file)
//! Usb_send_event(x)
//! Usb_ack_event(x)
//! Usb_clear_all_event()
//! Is_usb_event(x)
//! Is_not_usb_event(x)
volatile uint16_t g_usb_event=0;


//! Public : (bit) usb_connected
//! usb_connected is set to TRUE when VBUS has been detected
//! usb_connected is set to FALSE otherwise
//! Used with USB_DEVICE_FEATURE == ENABLED only
//!/
bit   usb_connected=FALSE;

//!
//! Public : (U8) usb_configuration_nb
//! Store the number of the USB configuration used by the USB device
//! when its value is different from zero, it means the device mode is enumerated
//! Used with USB_DEVICE_FEATURE == ENABLED only
//!/
extern uint8_t    usb_configuration_nb;

//!
//! Public : (bit) usb_suspended
//! usb_suspended is set to TRUE when USB is in suspend mode
//! usb_suspended is set to FALSE otherwise
//!/
bit   usb_suspended=FALSE;


//!
//! Public : (U8) remote_wakeup_feature
//! Store a host request for remote wake up (set feature received)
//!/
extern uint8_t remote_wakeup_feature;
extern volatile uint8_t usb_request_break_generation;

uint8_t rx_counter;
uint8_t tx_counter;

volatile uint8_t cpt_sof;

FILE USBout = FDEV_SETUP_STREAM(USB_Putchar, NULL, _FDEV_SETUP_WRITE);
//FILE Usb_str = FDEV_SETUP_STREAM(USB_putchar, NULL, _FDEV_SETUP_WRITE);

// to be removed
S_line_coding line_coding;
S_line_status line_status;      // for detection of serial state input lines
S_serial_state serial_state;   // for serial state output lines


//========================================================
//========================================================

 ISR(USB_GEN_vect)
{
  // - Device start of frame received
   if (Is_usb_sof() && Is_sof_interrupt_enabled())
   {
      Usb_ack_sof();
      Usb_sof_action();
   }
  // - Device Suspend event (no more USB activity detected)
   if (Is_usb_suspend() && Is_suspend_interrupt_enabled())
   {
      usb_suspended=TRUE;
      Usb_ack_wake_up();                 // clear wake up to detect next event
      Usb_send_event(EVT_USB_SUSPEND);
      Usb_ack_suspend();
      Usb_enable_wake_up_interrupt();
      Usb_disable_resume_interrupt();
      Usb_freeze_clock();
      Stop_pll();
      Usb_suspend_action();
   }
  // - Wake up event (USB activity detected): Used to resume
   if (Is_usb_wake_up() && Is_wake_up_interrupt_enabled())
   {
      if(Is_pll_ready()==FALSE)
      {
         Pll_start_auto();
         Wait_pll_ready();
      }
      Usb_unfreeze_clock();
      Usb_ack_wake_up();
      if(usb_suspended)
      {
         Usb_enable_resume_interrupt();
         Usb_enable_reset_interrupt();
         while(Is_usb_wake_up())
         {
            Usb_ack_wake_up();
         }
         _delay_ms(2);
         if(Is_usb_sof() || Is_usb_resume() || Is_usb_reset() )
         {
            Usb_disable_wake_up_interrupt();
            Usb_wake_up_action();
            Usb_send_event(EVT_USB_WAKE_UP);
            Usb_enable_suspend_interrupt();
            Usb_enable_resume_interrupt();
            Usb_enable_reset_interrupt();
            
         }
         else // Workarround to make the USB enter power down mode again (spurious transcient detected on the USB lines)
         {
            if(Is_usb_wake_up()) return;
            Usb_drive_dp_low();
            Usb_direct_drive_usb_enable();
            Usb_direct_drive_disable();
            Usb_disable_wake_up_interrupt();
         }
      }
   }
  // - Resume state bus detection
   if (Is_usb_resume() && Is_resume_interrupt_enabled())
   {
      usb_suspended = FALSE;
      Usb_disable_wake_up_interrupt();
      Usb_ack_resume();
      Usb_disable_resume_interrupt();
      Usb_resume_action();
      Usb_send_event(EVT_USB_RESUME);
   }
  // - USB bus reset detection
   if (Is_usb_reset()&& Is_reset_interrupt_enabled())
   {
      Usb_ack_reset();
      usb_init_device();
      Usb_reset_action();
      Usb_send_event(EVT_USB_RESET);
   }

}

//========================================================

void USB_Init_hw(void)
{

	Usb_enable_regulator();				//Enable the internal 3.3V regulator 
	Usb_disable();						//taken from usb_device_task_init()
	Usb_enable();
	rx_counter = 0;
	usb_connected=FALSE;
	
//	stdout = stderr = &Usb_str;			//setup output stream
	
	Usb_enable_sof_interrupt();

}

//========================================================

void USB_start_device(void)
{
   Usb_freeze_clock();
//   Pll_start_auto();
	PLLCSR = 0x04;
	PLLCSR |= _BV(PLLE);
//   Wait_pll_ready();
	while (!(PLLCSR & _BV(PLOCK)));
   Disable_interrupt();
   Usb_unfreeze_clock();
   Usb_attach();
//#if (USB_RESET_CPU == ENABLED)
//   Usb_reset_all_system();
//#else
   Usb_reset_macro_only();
//#endif
   usb_init_device();         // configure the USB controller EP0
   Usb_enable_suspend_interrupt();
   Usb_enable_reset_interrupt();
   Enable_interrupt();
}

//========================================================

void USB_device_task(void)
{
   if (usb_connected == FALSE)
   {
     usb_connected = TRUE;    // attach if application is not self-powered
     USB_start_device();
     Usb_vbus_on_action();
//	 _delay_ms(1000);
   }

   if(Is_usb_event(EVT_USB_RESET))
   {
      Usb_ack_event(EVT_USB_RESET);
      Usb_reset_endpoint(0);
      usb_configuration_nb=0;
   }

   // Here connection to the device enumeration process
   Usb_select_endpoint(EP_CONTROL);
   
   if (Is_usb_receive_setup())
   {
		usb_process_request();
   }
}

//========================================================

bit usb_test_hit(void)
{
  if(!Is_device_enumerated())
     return FALSE;

  if (!rx_counter)
  {
    Usb_select_endpoint(RX_EP);
    if (Is_usb_receive_out())
    {
      rx_counter = Usb_byte_counter();
      if (!rx_counter)
      {
        Usb_ack_receive_out();
      }
    }
  }
  return (rx_counter!=0);
}

//========================================================

uint8_t usb_getchar(void)
{
  register Uchar data_rx;

  Usb_select_endpoint(RX_EP);
  if (!rx_counter) while (!usb_test_hit());
  data_rx=Usb_read_byte();
  rx_counter--;
  if (!rx_counter) Usb_ack_receive_out();
  return data_rx;
}

//========================================================

bit usb_tx_ready(void)
{
  if(!Is_device_enumerated())
     return FALSE;

  if (!Is_usb_write_enabled())
  {
    return FALSE;
  }
  return TRUE;
}

//========================================================

int USB_Putchar(char data_to_send, FILE *unused)
{
	usb_send_buffer((U8*)&data_to_send, 1);
	return 0;
}

int16_t usb_putchar(int data_to_send)
{
   usb_send_buffer((U8*)&data_to_send, 1);
   return data_to_send;
}

//========================================================

void usb_send_buffer(U8 *buffer, U8 nb_data)
{
   U8 zlp;
   
	if(!Is_device_enumerated())
		return;
   
   // Compute if zlp required
	if(nb_data%TX_EP_SIZE) 
	{ 
		zlp=FALSE;
	} 
	else 
	{ 
		zlp=TRUE; 
	}
   
	Usb_select_endpoint(TX_EP);
	while (nb_data)
	{
		while(Is_usb_write_enabled()==FALSE); // Wait Endpoint ready
		while(Is_usb_write_enabled() && nb_data)
		{
			Usb_write_byte(*buffer);
			buffer++;
			nb_data--;
		}
		Usb_ack_in_ready();
	}
	
	if(zlp)
	{
		while(Is_usb_write_enabled()==FALSE); // Wait Endpoint ready 
		Usb_ack_in_ready();
	}
}

//========================================================

void sof_action()
{
   cpt_sof++;
}

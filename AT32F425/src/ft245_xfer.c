#include <string.h>
#include <stdbool.h>
#include "usb_std.h"
#include "usbd_sdr.h"
#include "usbd_core.h"

#include "ft245rom.h"
#include "ft245_xfer.h"


extern volatile bool g_blaster_rx_req;
extern volatile bool g_blaster_tx_ready;


static uint8_t usb_rx_buf[64];
static uint8_t usb_rx_len;


/**
  * @brief  initialize usb endpoint
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_init_handler(void *udev)
{
	usbd_core_type *usbd = (usbd_core_type *)udev;
	
	usbd_ept_open(usbd, FT245_IN_EP,  EPT_BULK_TYPE, FT245_IN_SZ);
	usbd_ept_open(usbd, FT245_OUT_EP, EPT_BULK_TYPE, FT245_OUT_SZ);

	usbd_ept_recv(usbd, FT245_OUT_EP, usb_rx_buf, FT245_OUT_SZ);
	
	ft245rom_init();

	return USB_OK;
}


/**
  * @brief  clear endpoint or other state
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_clear_handler(void *udev)
{
	usbd_core_type *usbd = (usbd_core_type *)udev;

	usbd_ept_close(usbd, FT245_IN_EP);
	usbd_ept_close(usbd, FT245_OUT_EP);
	
	return USB_OK;
}


/**
  * @brief  usb device class setup request handler
  * @param  udev: to the structure of usbd_core_type
  * @param  setup: setup packet
  * @retval status of usb_sts_type
  */
static usb_sts_type class_setup_handler(void *udev, usb_setup_type *setup)
{
	uint8_t addr;
	static uint8_t report_buf[2] __attribute__((aligned(4)));
	
	usbd_core_type *usbd = (usbd_core_type *)udev;
	
	switch(setup->bmRequestType & USB_REQ_TYPE_RESERVED)
	{
	case USB_REQ_TYPE_STANDARD:
		switch(setup->bRequest)
		{
		default:
			break;
		}
		break;
	
	case USB_REQ_TYPE_VENDOR:
		switch(setup->bRequest)
		{
		case FTDI_VEN_REQ_RD_EEPROM:
			addr = ((setup->wIndex >> 8) & 0x3F) << 1;
			report_buf[0] = ft245rom_read(addr);
			report_buf[1] = ft245rom_read(addr + 1);
			usbd_ctrl_send(usbd, report_buf, 2);
			break;
		
		case FTDI_VEN_REQ_GET_MODEM_STA:
			// return fixed modem status
			report_buf[0] = FTDI_MODEM_STA_DUMMY0;
			report_buf[1] = FTDI_MODEM_STA_DUMMY1;
			usbd_ctrl_send(usbd, report_buf, 2);
			break;
		
		default:
			if(setup->wLength)
			{
				// return dummy data
				report_buf[0] = 0x0;
				report_buf[1] = 0x0;
				usbd_ctrl_send(usbd, report_buf, 2);
			}
			else
			{
				usbd_ctrl_send_status(usbd);
			}
			break;
		}
		break;
	
	default:
		usbd_ctrl_unsupport(usbd);
		break;
	}
	
	return USB_OK;
}


/**
  * @brief  usb device endpoint 0 in status stage complete
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_ept0_tx_handler(void *udev)
{
	return USB_OK;
}


/**
  * @brief  usb device endpoint 0 out status stage complete
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_ept0_rx_handler(void *udev)
{
	return USB_OK;
}


/**
  * @brief  usb device transmision complete handler
  * @param  udev: to the structure of usbd_core_type
  * @param  ept_num: endpoint number
  * @retval status of usb_sts_type
  */
static usb_sts_type class_in_handler(void *udev, uint8_t ept_num)
{
	g_blaster_tx_ready = true;
	
	return USB_OK;
}


extern otg_core_type Otg;
void blaster_usbsend_hw(uint8_t * send_buf, uint8_t send_len)
{
	usbd_ept_send(&Otg.dev, FT245_IN_EP, send_buf, send_len);
}


/**
  * @brief  usb device endpoint receive data
  * @param  udev: to the structure of usbd_core_type
  * @param  ept_num: endpoint number
  * @retval status of usb_sts_type
  */
static usb_sts_type class_out_handler(void *udev, uint8_t ept_num)
{
	usbd_core_type *usbd = (usbd_core_type *)udev;

	usb_rx_len = usbd_get_recv_len(usbd, ept_num);
	
	g_blaster_rx_req = true;

	return USB_OK;
}


extern otg_core_type Otg;
uint8_t blaster_usbrecv_hw(uint8_t * recv_buf)
{
	memcpy(recv_buf, usb_rx_buf, usb_rx_len);
	
	/* start receive next packet */
	usbd_ept_recv(&Otg.dev, FT245_OUT_EP, usb_rx_buf, FT245_OUT_SZ);
	
	return usb_rx_len;
}


/**
  * @brief  usb device sof handler
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_sof_handler(void *udev)
{
	return USB_OK;
}


/**
  * @brief  usb device event handler
  * @param  udev: to the structure of usbd_core_type
  * @param  event: usb device event
  * @retval status of usb_sts_type
  */
static usb_sts_type class_event_handler(void *udev, usbd_event_type event)
{
	switch(event)
	{
	case USBD_RESET_EVENT:
		break;
	
	case USBD_SUSPEND_EVENT:
		break;
	
	case USBD_WAKEUP_EVENT:
		break;
	
	default:
		break;
	}
	
	return USB_OK;
}


usbd_class_handler hid_class_handler =
{
	class_init_handler,
	class_clear_handler,
	class_setup_handler,
	class_ept0_tx_handler,
	class_ept0_rx_handler,
	class_in_handler,
	class_out_handler,
	class_sof_handler,
	class_event_handler,
	NULL
};

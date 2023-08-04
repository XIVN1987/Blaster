#include "usb_std.h"
#include "usbd_core.h"
#include "ft245_xfer.h"


ALIGNED_HEAD uint8_t Blaster_DeviceDescriptor[USB_DEVICE_DESC_LEN] ALIGNED_TAIL =
{
	USB_DEVICE_DESC_LEN,				// bLength
	USB_DESCIPTOR_TYPE_DEVICE,			// bDescriptorType
	0x00, 0x02,             			// bcdUSB
	0x00,               				// bDeviceClass
	0x00,               				// bDeviceSubClass
	0x00,               				// bDeviceProtocol
	USB_MAX_EP0_SIZE,   				// bMaxPacketSize0
	USBD_VID & 0xFF, USBD_VID >> 8,		// idVendor
	USBD_PID & 0xFF, USBD_PID >> 8,		// idProduct
	0x00, 0x04,         				// bcdDevice
	0x01,               				// iManufacture
	0x02,               				// iProduct
	0x03,               				// iSerialNumber
	0x01                				// bNumConfigurations
};


ALIGNED_HEAD uint8_t Blaster_ConfigDescriptor[] ALIGNED_TAIL =
{
	USB_DEVICE_CFG_DESC_LEN,			// bLength
	USB_DESCIPTOR_TYPE_CONFIGURATION,	// bDescriptorType
#define TOTAL_LEN	(USB_DEVICE_CFG_DESC_LEN + (USB_DEVICE_IF_DESC_LEN + USB_DEVICE_EPT_LEN + USB_DEVICE_EPT_LEN))
	TOTAL_LEN & 0xFF, TOTAL_LEN >> 8,	// wTotalLength
	0x01, 								// bNumInterfaces
	0x01,               				// bConfigurationValue
	0x00,               				// iConfiguration
	0x80,               				// bmAttributes, D6: self power  D5: remote wake-up
										// D7 is reserved and must be set to one for historical reasons.
	0xE1,               				// MaxPower, 225 * 2mA = 450mA
	
	// I/F descriptor
	USB_DEVICE_IF_DESC_LEN,      		// bLength
	USB_DESCIPTOR_TYPE_INTERFACE,		// bDescriptorType
	0x00,               				// bInterfaceNumber
	0x00,               				// bAlternateSetting
	0x02,               				// bNumEndpoints    
	0xFF,               				// bInterfaceClass
	0xFF,               				// bInterfaceSubClass
	0xFF,               				// bInterfaceProtocol
	0x00,               				// iInterface

	// EP Descriptor
	USB_DEVICE_EPT_LEN,					// bLength
	USB_DESCIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	FT245_IN_EP,						// bEndpointAddress
	USB_EPT_DESC_BULK,					// bmAttributes
	FT245_IN_SZ, 0x00,					// wMaxPacketSize
	1,									// bInterval
	
	// EP Descriptor
	USB_DEVICE_EPT_LEN,             	// bLength
	USB_DESCIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	FT245_OUT_EP,						// bEndpointAddress    
	USB_EPT_DESC_BULK,					// bmAttributes
	FT245_OUT_SZ, 0x00,					// wMaxPacketSize
	1,									// bInterval
};


ALIGNED_HEAD static uint8_t g_langid_string[] ALIGNED_TAIL =
{
	4,
	USB_DESCIPTOR_TYPE_STRING,
	0x09,
	0x04,
};


ALIGNED_HEAD uint8_t Blaster_StringVendor[] ALIGNED_TAIL =
{
	14,
    USB_DESCIPTOR_TYPE_STRING,
    'A', 0, 'l', 0, 't', 0, 'e', 0, 'r', 0, 'a', 0
};


ALIGNED_HEAD uint8_t Blaster_StringProduct[] ALIGNED_TAIL =
{
	24,
    USB_DESCIPTOR_TYPE_STRING,
    'U', 0, 'S', 0, 'B', 0, '-', 0, 'B', 0, 'l', 0, 'a', 0, 's', 0, 't', 0, 'e', 0, 'r', 0
};


ALIGNED_HEAD uint8_t Blaster_StringSerial[] ALIGNED_TAIL =
{
	18,
	USB_DESCIPTOR_TYPE_STRING,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0
};


static usbd_desc_t *get_device_descriptor(void)
{
	static usbd_desc_t device_descriptor =
	{
		sizeof(Blaster_DeviceDescriptor),
		Blaster_DeviceDescriptor
	};
	
	return &device_descriptor;
}


static usbd_desc_t * get_device_qualifier(void)
{
	return NULL;
}


static usbd_desc_t *get_config_descriptor(void)
{
	static usbd_desc_t config_descriptor =
	{
		sizeof(Blaster_ConfigDescriptor),
		Blaster_ConfigDescriptor
	};

	return &config_descriptor;
}


static usbd_desc_t *get_langid_string(void)
{
	static usbd_desc_t langid_string =
	{
		sizeof(g_langid_string),
		g_langid_string
	};

	return &langid_string;
}


static usbd_desc_t *get_vendor_string(void)
{
	static usbd_desc_t vendor_string =
	{
		sizeof(Blaster_StringVendor),
		Blaster_StringVendor
	};

	return &vendor_string;
}


static usbd_desc_t *get_product_string(void)
{
	static usbd_desc_t product_string =
	{
		sizeof(Blaster_StringProduct),
		Blaster_StringProduct
	};

	return &product_string;
}


static void int_to_unicode(uint32_t value , uint8_t *pbuf , uint8_t len);
static usbd_desc_t *get_serial_string(void)
{
	static usbd_desc_t serial_string =
	{
		sizeof(Blaster_StringSerial),
		Blaster_StringSerial
	};

	uint32_t serial0, serial1, serial2;
	serial0 = *(uint32_t*)0x1FFFF7E8;
	serial1 = *(uint32_t*)0x1FFFF7EC;
	serial2 = *(uint32_t*)0x1FFFF7F0;

	serial0 += serial1;
	serial0 += serial2;

	if(serial0 != 0)
	{
		int_to_unicode(serial0, &Blaster_StringSerial[2], 8);
	}
	
	return &serial_string;
}


usbd_desc_handler hid_desc_handler =
{
	get_device_descriptor,
	get_device_qualifier,
	get_config_descriptor,
	NULL,					// get_device_other_speed
	get_langid_string,
	get_vendor_string,
	get_product_string,
	get_serial_string,
	NULL,
	NULL,					// get_device_config_string
};


static void int_to_unicode(uint32_t value , uint8_t *pbuf , uint8_t len)
{
	for(int i = 0 ; i < len ; i++)
	{
		if((value >> 28) < 0xA)
		{
			pbuf[2 * i] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2 * i] = (value >> 28) + 'A' - 10;
		}

		pbuf[2 * i + 1] = 0;
		
		value = value << 4;
	}
}

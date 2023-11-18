/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_desc.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2019/10/15
 * Description        : USB Descriptors.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
*******************************************************************************/ 
#include "usb_lib.h"
#include "usb_desc.h"


const uint8_t  Blaster_DeviceDescriptor[] = {
    USBD_SIZE_DEVICE_DESC,              // bLength
    DEVICE_DESCRIPTOR,                  // bDescriptorType
    0x00, 0x02,                         // bcdUSB
    0x00,                               // bDeviceClass
    0x00,                               // bDeviceSubClass
    0x00,                               // bDeviceProtocol
    ENDP0_SIZE,                         // bMaxPacketSize0
    USBD_VID & 0xFF, USBD_VID >> 8,     // idVendor
    USBD_PID & 0xFF, USBD_PID >> 8,     // idProduct
    0x00, 0x04,                         // bcdDevice
    0x01,                               // iManufacture
    0x02,                               // iProduct
    0x03,                               // iSerialNumber
    0x01                                // bNumConfigurations
};


const uint8_t  Blaster_ConfigDescriptor[] = {
    USBD_SIZE_CONFIG_DESC,              // bLength
    CONFIG_DESCRIPTOR,                  // bDescriptorType
    USBD_SIZE_CONFIG_TOTAL & 0xFF,
    USBD_SIZE_CONFIG_TOTAL >> 8,        // wTotalLength
    0x01,                               // bNumInterfaces
    0x01,                               // bConfigurationValue
    0x00,                               // iConfiguration
    0x80,                               // bmAttributes, D6: self power  D5: remote wake-up
                                        // D7 is reserved and must be set to one for historical reasons.
    0xE1,                               // MaxPower, 225 * 2mA = 450mA

    // I/F descriptor
    USBD_SIZE_INTERFACE_DESC,           // bLength
    INTERFACE_DESCRIPTOR,               // bDescriptorType
    0x00,                               // bInterfaceNumber
    0x00,                               // bAlternateSetting
    0x02,                               // bNumEndpoints
    0xFF,                               // bInterfaceClass
    0xFF,                               // bInterfaceSubClass
    0xFF,                               // bInterfaceProtocol
    0x00,                               // iInterface

    // EP Descriptor
    USBD_SIZE_ENDPOINT_DESC,            // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    FT245_IN_EP,                        // bEndpointAddress
    USB_EPT_DESC_BULK,                  // bmAttributes
    FT245_IN_SZ, 0x00,                  // wMaxPacketSize
    1,                                  // bInterval

    // EP Descriptor
    USBD_SIZE_ENDPOINT_DESC,            // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    FT245_OUT_EP,                       // bEndpointAddress
    USB_EPT_DESC_BULK,                  // bmAttributes
    FT245_OUT_SZ, 0x00,                 // wMaxPacketSize
    1,                                  // bInterval
};


const uint8_t Blaster_StringLangID[] = {
	USBD_SIZE_STRING_LANGID,
	STRING_DESCRIPTOR,
	0x09, 0x04
};


const uint8_t Blaster_StringVendor[] = {
	USBD_SIZE_STRING_VENDOR,    
	STRING_DESCRIPTOR,
	'A', 0, 'l', 0, 't', 0, 'e', 0, 'r', 0, 'a', 0
};


const uint8_t Blaster_StringProduct[] = {
    USBD_SIZE_STRING_PRODUCT,
    STRING_DESCRIPTOR,
    'U', 0, 'S', 0, 'B', 0, '-', 0, 'B', 0, 'l', 0, 'a', 0, 's', 0, 't', 0, 'e', 0, 'r', 0
};


uint8_t Blaster_StringSerial[] = {
	USBD_SIZE_STRING_SERIAL,          
	STRING_DESCRIPTOR,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0
};

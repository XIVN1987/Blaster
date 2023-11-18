/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_desc.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : This file contains all the functions prototypes for the  
 *                      USB description firmware library.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/ 
#ifndef __USB_DESC_H
#define __USB_DESC_H


#define USBD_VID        0x09FB  // Altera
#define USBD_PID        0x6001  // USB-Blaster


/* Define EP number */
#define FT245_IN_EP     0x81
#define FT245_OUT_EP    0x02


/* Define EP maximum packet size */
#define FT245_IN_SZ     ENDP1_TXSIZE
#define FT245_OUT_SZ    ENDP2_RXSIZE


#define USBD_SIZE_DEVICE_DESC       18
#define USBD_SIZE_CONFIG_DESC       9
#define USBD_SIZE_INTERFACE_DESC    9
#define USBD_SIZE_ENDPOINT_DESC     7
#define USBD_SIZE_STRING_LANGID     4
#define USBD_SIZE_STRING_VENDOR     14
#define USBD_SIZE_STRING_PRODUCT    24
#define USBD_SIZE_STRING_SERIAL     18


#define USBD_SIZE_CONFIG_TOTAL      (USBD_SIZE_CONFIG_DESC + (USBD_SIZE_INTERFACE_DESC + USBD_SIZE_ENDPOINT_DESC + USBD_SIZE_ENDPOINT_DESC))


extern const uint8_t Blaster_DeviceDescriptor[];
extern const uint8_t Blaster_ConfigDescriptor[];
extern const uint8_t Blaster_StringLangID [];
extern const uint8_t Blaster_StringVendor [];
extern const uint8_t Blaster_StringProduct[];
extern       uint8_t Blaster_StringSerial [];


#define USB_EPT_DESC_CONTROL        0x00
#define USB_EPT_DESC_ISO            0x01
#define USB_EPT_DESC_BULK           0x02
#define USB_EPT_DESC_INTERRUPT      0x03


#endif /* __USB_DESC_H */

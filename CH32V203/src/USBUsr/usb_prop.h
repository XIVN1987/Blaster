/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_prop.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : All processing related to Virtual COM Port Demo (Endpoint 0) 
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#ifndef __usb_prop_H
#define __usb_prop_H


#define FTDI_VEN_REQ_RESET              0x00
#define FTDI_VEN_REQ_SET_BAUDRATE       0x01
#define FTDI_VEN_REQ_SET_DATA_CHAR      0x02
#define FTDI_VEN_REQ_SET_FLOW_CTRL      0x03
#define FTDI_VEN_REQ_SET_MODEM_CTRL     0x04
#define FTDI_VEN_REQ_GET_MODEM_STA      0x05
#define FTDI_VEN_REQ_SET_EVENT_CHAR     0x06
#define FTDI_VEN_REQ_SET_ERR_CHAR       0x07
#define FTDI_VEN_REQ_SET_LAT_TIMER      0x09
#define FTDI_VEN_REQ_GET_LAT_TIMER      0x0A
#define FTDI_VEN_REQ_SET_BITMODE        0x0B
#define FTDI_VEN_REQ_RD_PINS            0x0C
#define FTDI_VEN_REQ_RD_EEPROM          0x90
#define FTDI_VEN_REQ_WR_EEPROM          0x91
#define FTDI_VEN_REQ_ES_EEPROM          0x92

#define FTDI_MODEM_STA_SIZE             0x02
#define FTDI_MODEM_STA_DUMMY0           0x01
#define FTDI_MODEM_STA_DUMMY1           0x60


#endif /* __usb_prop_H */








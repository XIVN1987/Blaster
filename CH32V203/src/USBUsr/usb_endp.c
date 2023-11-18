/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_endp.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : Endpoint routines
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include "usb_lib.h"


extern volatile bool g_blaster_rx_req;
extern volatile bool g_blaster_tx_ready;


void EP2_OUT_Callback(void)
{
    g_blaster_rx_req = TRUE;
}

uint8_t blaster_usbrecv_hw(uint8_t * recv_buf)
{
    uint8_t len = USB_SIL_Read(ENDP2, recv_buf);

    SetEPRxValid(ENDP2);

    return len;
}


void EP1_IN_Callback(void)
{
    g_blaster_tx_ready = TRUE;
}

void blaster_usbsend_hw(uint8_t * send_buf, uint8_t send_len)
{
    USB_SIL_Write(ENDP1, send_buf, send_len);

    SetEPTxValid(ENDP1);
}

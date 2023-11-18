/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_prop.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : All processing related to Virtual Com Port Demo
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_desc.h"
#include "usb_prop.h"

#include "ft245rom.h"


ONE_DESCRIPTOR Device_Descriptor =
{
	(uint8_t*)Blaster_DeviceDescriptor,
	USBD_SIZE_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor =
{
	(uint8_t*)Blaster_ConfigDescriptor,
	USBD_SIZE_CONFIG_TOTAL
};

ONE_DESCRIPTOR String_Descriptor[4] =
{
	{(uint8_t*)Blaster_StringLangID,  USBD_SIZE_STRING_LANGID},
	{(uint8_t*)Blaster_StringVendor,  USBD_SIZE_STRING_VENDOR},
	{(uint8_t*)Blaster_StringProduct, USBD_SIZE_STRING_PRODUCT},
	{(uint8_t*)Blaster_StringSerial,  USBD_SIZE_STRING_SERIAL}
};


/*******************************************************************************
 * @fn         USB_Port_Set
 *
 * @brief      Set USB IO port.
 *
 * @param      NewState: DISABLE or ENABLE.
 *             Pin_In_IPU: Enables or Disables intenal pull-up resistance.
 *
 * @return     None
 */
static void USB_Port_Set(FunctionalState NewState, FunctionalState Pin_In_IPU)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    if(NewState)
    {
        _SetCNTR(_GetCNTR()&(~(1<<1)));
        GPIOA->CFGHR&=0XFFF00FFF;
        GPIOA->OUTDR&=~(3<<11);
        GPIOA->CFGHR|=0X00044000;
    }
    else
    {
        _SetCNTR(_GetCNTR()|(1<<1));
        GPIOA->CFGHR&=0XFFF00FFF;
        GPIOA->OUTDR&=~(3<<11);
        GPIOA->CFGHR|=0X00033000;
    }

    if(Pin_In_IPU) EXTEN->EXTEN_CTR |=  EXTEN_USBD_PU_EN;
    else           EXTEN->EXTEN_CTR &= ~EXTEN_USBD_PU_EN;
}

/*******************************************************************************
 * @fn       USBD_init.
 *
 * @brief    init routine.
 * 
 * @return   None.
 */
void USBD_init(void)
{
    pInformation->Current_Configuration = 0;

    PowerOn();

    for(int i = 0; i < 8; i++)
        _SetENDPOINT(i, _GetENDPOINT(i) & 0x7F7F & EPREG_MASK);
    _SetISTR((uint16_t)0x00FF);

    USB_SIL_Init();

    bDeviceState = UNCONNECTED;

    USB_Port_Set(DISABLE, DISABLE);
    for(int i = 0; i < SystemCoreClock / 100; i++) __NOP();
    USB_Port_Set(ENABLE, ENABLE);

    ft245rom_init();
}


/*******************************************************************************
 * @fn      USBD_Reset
 *
 * @brief   USBD reset routine
 *
 * @return  none
 */
void USBD_Reset(void)
{
    pInformation->Current_Configuration = 0;
    pInformation->Current_Interface = 0;
    pInformation->Current_Feature = Blaster_ConfigDescriptor[7];

    SetBTABLE(BTABLE_ADDRESS);

    /* Initialize Endpoint 0 */
    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxCount(ENDP0, ENDP0_SIZE);
    SetEPRxCount(ENDP0, ENDP0_SIZE);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxStatus(ENDP0, EP_RX_VALID);
    _ClearDTOG_TX(ENDP0);
    _ClearDTOG_RX(ENDP0);

    /* Initialize Endpoint 1 */
    SetEPType(ENDP1, EP_BULK);
    SetEPTxAddr(ENDP1, ENDP1_TXADDR);
    SetEPTxCount(ENDP1, ENDP1_TXSIZE);
    SetEPTxStatus(ENDP1, EP_TX_NAK);
    SetEPRxStatus(ENDP1, EP_RX_DIS);
    _ClearDTOG_TX(ENDP1);

    /* Initialize Endpoint 2 */
    SetEPType(ENDP2, EP_BULK);
    SetEPRxAddr(ENDP2, ENDP2_RXADDR);
    SetEPRxCount(ENDP2, ENDP2_RXSIZE);
    SetEPTxStatus(ENDP2, EP_TX_DIS);
    SetEPRxStatus(ENDP2, EP_RX_VALID);
    _ClearDTOG_RX(ENDP2);

    SetDeviceAddress(0);

    bDeviceState = ATTACHED;
}


/*********************************************************************
 * @fn      USBD_Status_In.
 *
 * @brief    USBD Status In Routine.
 *
 * @return   None.
 */
void USBD_Status_In(void)
{
}


/*******************************************************************************
 * @fn       USBD_Status_Out
 *
 * @brief    USBD Status OUT Routine.
 *
 * @return   None.
 */
void USBD_Status_Out(void)
{
}


static uint8_t report_buf[2];

static uint8_t *Blaster_Get_VendorData(uint16_t len)
{
    if(len == 0)
    {
        pInformation->Ctrl_Info.Usb_wLength = sizeof(report_buf);
        return NULL;
    }

    return (uint8_t *)report_buf;
}

/*********************************************************************
 * @fn      USBD_Data_Setup
 *
 * @brief   handle the data class specific requests
 *
 * @param   Request Nb.
 *
 * @return  USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT USBD_Data_Setup(uint8_t RequestNo)
{
    uint8_t *(*CopyRoutine)(uint16_t) = NULL;
    uint8_t addr;

    if(Type_Recipient == (VENDOR_REQUEST | DEVICE_RECIPIENT))
    {
        if(pInformation->USBbmRequestType & 0x80)   // device -> host
        {
            switch(RequestNo)
            {
            case FTDI_VEN_REQ_RD_EEPROM:
                addr = ((pInformation->USBwIndex >> 8) & 0x3F) << 1;
                report_buf[0] = ft245rom_read(addr);
                report_buf[1] = ft245rom_read(addr + 1);
                CopyRoutine = Blaster_Get_VendorData;
                break;

            case FTDI_VEN_REQ_GET_MODEM_STA:
                // return fixed modem status
                report_buf[0] = FTDI_MODEM_STA_DUMMY0;
                report_buf[1] = FTDI_MODEM_STA_DUMMY1;
                CopyRoutine = Blaster_Get_VendorData;
                break;

            default:
                // return dummy data
                report_buf[0] = 0x0;
                report_buf[1] = 0x0;
                CopyRoutine = Blaster_Get_VendorData;
                break;
            }
        }
        else
        {
            return USB_UNSUPPORT;
        }
    }

    if(CopyRoutine)
    {
        pInformation->Ctrl_Info.CopyData = CopyRoutine;
        pInformation->Ctrl_Info.Usb_wOffset = 0;
        (*CopyRoutine)(0);

        return USB_SUCCESS;
    }

    return USB_UNSUPPORT;
}


/*********************************************************************
 * @fn      USBD_NoData_Setup.
 *
 * @brief   handle the no data class specific requests.
 *
 * @param   Request Nb.
 *
 * @return  USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT USBD_NoData_Setup(uint8_t RequestNo)
{
    if(Type_Recipient == (VENDOR_REQUEST | DEVICE_RECIPIENT))
    {
        switch(RequestNo)
        {
        case FTDI_VEN_REQ_RESET:
            break;
        case FTDI_VEN_REQ_SET_BAUDRATE:
            break;
        case FTDI_VEN_REQ_SET_DATA_CHAR:
            break;
        case FTDI_VEN_REQ_SET_FLOW_CTRL:
            break;
        case FTDI_VEN_REQ_SET_MODEM_CTRL:
            break;
        default:
            break;
        }

        return USB_SUCCESS;
    }

    return USB_UNSUPPORT;
}


/*********************************************************************
 * @fn      USBD_Get_Interface_Setting.
 *
 * @brief   test the interface and the alternate setting according to the
 *          supported one.
 *
 * @param   Interface - interface number.
 *          AlternateSetting - Alternate Setting number.
 *
 * @return  USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT USBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
    if((Interface > 0) || (AlternateSetting > 0))
    {
        return USB_UNSUPPORT;
    }

    return USB_SUCCESS;
}


/*********************************************************************
 * @fn      USBD_GetDeviceDescriptor.
 *
 * @brief   Gets the device descriptor.
 *
 * @param   Length.
 *
 * @return  The address of the device descriptor.
 */
uint8_t *USBD_GetDeviceDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}


/*********************************************************************
 * @fn      USBD_GetConfigDescriptor.
 *
 * @brief   get the configuration descriptor.
 *
 * @param   Length.
 *
 * @return   The address of the configuration descriptor.
 */
uint8_t *USBD_GetConfigDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}


/*********************************************************************
 * @fn      USBD_GetStringDescriptor
 *
 * @brief   Gets the string descriptors according to the needed index
 *
 * @param   Length.
 *
 * @return    The address of the string descriptors.
 */
uint8_t *USBD_GetStringDescriptor(uint16_t Length)
{
    uint8_t wValue0 = pInformation->USBwValue0;

    if(wValue0 > 4)
    {
        return NULL;
    }
    else
    {
        return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
    }
}


/*********************************************************************
 * @fn      USBD_SetConfiguration.
 *
 * @brief     Update the device state to configured.
 *
 * @return    None.
 */
void USBD_SetConfiguration(void)
{
    DEVICE_INFO *pInfo = &Device_Info;

    if(pInfo->Current_Configuration != 0)
    {
        bDeviceState = CONFIGURED;
    }
}


/*******************************************************************************
 * @fn         USBD_SetDeviceAddress.
 *
 * @brief      Update the device state to addressed.
 *
 * @return     None.
 */
void USBD_SetDeviceAddress (void)
{
    bDeviceState = ADDRESSED;
}


DEVICE Device_Table =
{
    EP_NUM,
    1
};


DEVICE_PROP Device_Property =
{
    USBD_init,
    USBD_Reset,
    USBD_Status_In,
    USBD_Status_Out,
    USBD_Data_Setup,
    USBD_NoData_Setup,
    USBD_Get_Interface_Setting,
    USBD_GetDeviceDescriptor,
    USBD_GetConfigDescriptor,
    USBD_GetStringDescriptor,
    0,
    ENDP0_SIZE
};


#define USBD_GetConfiguration       NOP_Process
// #define USBD_SetConfiguration    NOP_Process
#define USBD_GetInterface           NOP_Process
#define USBD_SetInterface           NOP_Process
#define USBD_GetStatus              NOP_Process
#define USBD_ClearFeature           NOP_Process
#define USBD_SetEndPointFeature     NOP_Process
#define USBD_SetDeviceFeature       NOP_Process
// #define USBD_SetDeviceAddress    NOP_Process

USER_STANDARD_REQUESTS User_Standard_Requests =
{
    USBD_GetConfiguration,
    USBD_SetConfiguration,
    USBD_GetInterface,
    USBD_SetInterface,
    USBD_GetStatus,
    USBD_ClearFeature,
    USBD_SetEndPointFeature,
    USBD_SetDeviceFeature,
    USBD_SetDeviceAddress
};

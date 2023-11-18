#include "ch32v20x.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_istr.h"

#include "blaster.h"


void USB_Config(void);
void SysTick_Config(uint32_t ticks);

int main(void)
{
    USB_Config();

    blaster_init();

    SysTick_Config(SystemCoreClock / 1000);
 	
	while(1)
	{
	    if(bDeviceState == CONFIGURED)
        {
            blaster_exec();
        }
	}
}


void SysTick_Config(uint32_t ticks)
{
    SysTick->CTLR= 0;
    SysTick->SR  = 0;
    SysTick->CNT = 0;
    SysTick->CMP = ticks;
    SysTick->CTLR= 0x0F;

    NVIC_EnableIRQ(SysTicK_IRQn);
}

volatile uint32_t SysTick_ms = 0;

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
    SysTick->SR = 0;

    SysTick_ms++;
}

uint32_t millis(void)
{
    return SysTick_ms;
}


void USB_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div3);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

    USB_Init();

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Line = EXTI_Line18;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}

void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    USB_Istr();
}

void USBWakeUp_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USBWakeUp_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line18);
}

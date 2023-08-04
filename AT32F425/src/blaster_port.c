/*
blaster_port.c - USB-Blaster IO port

MIT License

Copyright (c) 2016 Yongqian Tang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "at32f425.h"
#include "blaster_port.h"


#define JTAG_TCK_PORT	GPIOA
#define JTAG_TCK_PIN	GPIO_PINS_2
#define JTAG_TMS_PORT	GPIOA
#define JTAG_TMS_PIN	GPIO_PINS_1
#define JTAG_TDI_PORT	GPIOA
#define JTAG_TDI_PIN	GPIO_PINS_3
#define JTAG_TDO_PORT	GPIOA
#define JTAG_TDO_PIN	GPIO_PINS_0

#define LED_STAT_PORT	GPIOA
#define LED_STAT_PIN	GPIO_PINS_4


#define TCK_OUT(b)      gpio_bits_write(JTAG_TCK_PORT, JTAG_TCK_PIN, b ? TRUE : FALSE)
#define TCK_0()         TCK_OUT(0)
#define TCK_1()         TCK_OUT(1)

#define TMS_OUT(b)      gpio_bits_write(JTAG_TMS_PORT, JTAG_TMS_PIN, b ? TRUE : FALSE)

#define TDI_OUT(b)      gpio_bits_write(JTAG_TDI_PORT, JTAG_TDI_PIN, b ? TRUE : FALSE)

#define TDO_IN()        gpio_input_data_bit_read(JTAG_TDO_PORT, JTAG_TDO_PIN)


void bport_init(void)
{
	crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
	crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
	
    gpio_init_type gpio_init_struct;
	gpio_default_para_init(&gpio_init_struct);
	
	gpio_init_struct.gpio_pins = JTAG_TCK_PIN;
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
	gpio_init(JTAG_TCK_PORT, &gpio_init_struct);
	
	gpio_init_struct.gpio_pins = JTAG_TMS_PIN;
	gpio_init(JTAG_TMS_PORT, &gpio_init_struct);

	gpio_init_struct.gpio_pins = JTAG_TDI_PIN;
	gpio_init(JTAG_TDI_PORT, &gpio_init_struct);

	gpio_init_struct.gpio_pins = JTAG_TDO_PIN;
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init(JTAG_TDO_PORT, &gpio_init_struct);
	
	gpio_init_struct.gpio_pins = LED_STAT_PIN;
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init(LED_STAT_PORT, &gpio_init_struct);
}

// bit-band mode output
void bport_state_set(uint8_t d)
{
    TDI_OUT(d & BLASTER_STA_OUT_TDI);
    TMS_OUT(d & BLASTER_STA_OUT_TMS);
    TCK_OUT(d & BLASTER_STA_OUT_TCK);
}

// bit-bang mode input
uint8_t bport_state_get(void)
{
    uint32_t d = 0;

    d |= TDO_IN() << BLASTER_STA_IN_TDO_BIT;

    return d;
}

// shift mode output
void bport_shift_out(uint8_t d)
{
#define BPORT_SHIFT_BIT()    TDI_OUT(d & 1); TCK_1(); d >>= 1; TCK_0()

    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();

#undef BPORT_SHIFT_BIT
}

// shift mode input
uint8_t bport_shift_io(uint8_t d)
{
    uint32_t dshift = d;
    uint32_t din;

#define BPORT_SHIFT_BIT()    TDI_OUT(dshift & 1); din = TDO_IN(); TCK_1(); dshift = (dshift >> 1) | (din << 7); TCK_0()
    
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();
    BPORT_SHIFT_BIT();

#undef BPORT_SHIFT_BIT

    return dshift & 0xff;
}

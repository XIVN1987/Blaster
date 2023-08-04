/*
blaster.c - USB-Blaster core

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

#include <stdint.h>
#include <stdbool.h>
#include "ringbuf.h"
#include "blaster.h"
#include "blaster_port.h"


/*-----------------------------------*/

#define SEND_PACKET_SIZE            (ENDP1_TXSIZE)
#define SEND_PACKET_VALID_SIZE      (SEND_PACKET_SIZE - FTDI_MODEM_STA_SIZE)

#define RECV_BUFF_SIZE              (ENDP2_RXSIZE)
#define SEND_BUFF_SIZE              (ENDP2_RXSIZE)

#define BLASTER_HEARTBEAT_TIME      (10)    // 10ms

/*-----------------------------------*/

// usb rx/tx ready flag
volatile bool g_blaster_rx_req = false;
volatile bool g_blaster_tx_ready = true;

// usb receive buffer, to output
static uint8_t s_recv_buffer[RECV_BUFF_SIZE];
static uint32_t s_recv_buff_len;
static uint32_t s_recv_buff_idx;

// usb send buffer, input from outside
static uint8_t s_send_buffer[SEND_BUFF_SIZE];
static uint8_t s_send_packet_buff[SEND_PACKET_SIZE];
static ringbuf_t s_send_ring_buff;

// operate mode & count
static uint32_t s_blaster_shift_cnt;
static bool s_blaster_shift_en;
static bool s_blaster_read_en;

// send dummy modem status periodicity (FTDI driver required)
static uint32_t s_blaster_sendtime;
static bool s_blaster_senddummy;

/*-----------------------------------*/

static void blaster_usbrecv(void)
{    
    if (!g_blaster_rx_req) {
        return;
    }

    if (s_recv_buff_idx != s_recv_buff_len) {
        return;
    }
    
    s_recv_buff_len = blaster_usbrecv_hw(s_recv_buffer);
    s_recv_buff_idx = 0;
    g_blaster_rx_req = false;
}

static void blaster_usbsend(void)
{
    uint32_t send_len;

    if (!g_blaster_tx_ready) {
        return;
    }

    send_len = ringbuf_use_len(&s_send_ring_buff);
    if (send_len > SEND_PACKET_VALID_SIZE) {
        // send full packet
        send_len = SEND_PACKET_VALID_SIZE;
    } else if (0 == send_len) {
        if (millis() - s_blaster_sendtime >= BLASTER_HEARTBEAT_TIME) {
            // send heart beat packet
        } else if (s_blaster_senddummy) {
            // send dummy packet
            s_blaster_senddummy = false;
        } else {
            // nothing to send
            return;
        }
    } else {
        // send not full packet
    }

    s_blaster_sendtime = millis();

    ringbuf_get(&s_send_ring_buff, &s_send_packet_buff[FTDI_MODEM_STA_SIZE], send_len);
    
    /* Reset the control token to inform upper layer that a transfer is ongoing */
    g_blaster_tx_ready = false;
	
	blaster_usbsend_hw(s_send_packet_buff, FTDI_MODEM_STA_SIZE + send_len);
}

/*-----------------------------------*/

/* 上电默认处于 bit-bang 模式，此时电脑发过来的数据高2位为SHIFT、READ指示，
   低6位为要发送到引脚上的数据，对应TCK、TMS、TDI、NCS、NCE、OE这六个引脚。
   如果检测到SHIFT位置位，切换到shift mode，这个字节的低6位为移位字节个数。
   shift 模式下，每个字节的8位都是数据，且都通过TDI引脚发出，指定个数字节发送完后自动切换回 bit-bang 模式

   FT245方案中的CPLD用于在shift模式下从FIFO中取出并行数据，然后串行传输到TDI引脚上
*/
static void blaster_ioproc(void)
{
    uint8_t d;

    while ((s_recv_buff_idx < s_recv_buff_len) && !ringbuf_is_full(&s_send_ring_buff)) {
        d = s_recv_buffer[s_recv_buff_idx];
        if (0 == s_blaster_shift_cnt) {
            // bit-bang mode (default)
            s_blaster_shift_en = (0 != (d & BLASTER_STA_SHIFT));
            s_blaster_read_en = (0 != (d & BLASTER_STA_READ));
            if (s_blaster_shift_en) {
                s_blaster_shift_cnt = (d & BLASTER_STA_CNT_MASK);
            } else if (s_blaster_read_en)  {
                bport_state_set(d);
                d = bport_state_get();
                ringbuf_putc(&s_send_ring_buff, d);
            } else {
                bport_state_set(d);
            }
        } else {
            // shift mode
            if (s_blaster_read_en) {
                d = bport_shift_io(d);
                ringbuf_putc(&s_send_ring_buff, d);
            } else {
                bport_shift_out(d);
            }
            s_blaster_shift_cnt--;
        }
        
        s_recv_buff_idx++;
    }
}

/*-----------------------------------*/

void blaster_init(void)
{
    g_blaster_rx_req = false;
    g_blaster_tx_ready = true;

    s_blaster_shift_cnt = 0;
    s_blaster_shift_en = false;
    s_blaster_read_en = false;

    s_blaster_sendtime = 0;
    s_blaster_senddummy = true;
    
    s_recv_buff_len = 0;
    s_recv_buff_idx = 0;

    // first two bytes are modem status
    s_send_packet_buff[0] = FTDI_MODEM_STA_DUMMY0;
    s_send_packet_buff[1] = FTDI_MODEM_STA_DUMMY1;

    ringbuf_init(&s_send_ring_buff, s_send_buffer, sizeof(s_send_buffer));

    bport_init();
}

void blaster_exec(void)
{
    blaster_usbrecv();
    blaster_ioproc();
    blaster_usbsend();
}

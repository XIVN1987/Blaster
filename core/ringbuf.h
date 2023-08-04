/*
ringbuf.h

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
#ifndef __RINGBUF_H__
#define __RINGBUF_H__

typedef struct ring_buffer {
    void *buffer;   //缓冲区
    uint32_t size;  //大小
    uint32_t in;    //入口位置
    uint32_t out;   //出口位置
} ringbuf_t;

uint32_t ringbuf_init(ringbuf_t *ring_buf, void *buffer, uint32_t size);
uint32_t ringbuf_use_len(const ringbuf_t *ring_buf);
uint32_t ringbuf_is_full(const ringbuf_t *ring_buf);
uint32_t ringbuf_get(ringbuf_t *ring_buf, void *buffer, uint32_t size);
uint32_t ringbuf_put(ringbuf_t *ring_buf, const void *buffer, uint32_t size);
uint32_t ringbuf_putc(ringbuf_t *ring_buf, const uint8_t c);

#endif //__RINGBUF_H__

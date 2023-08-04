/*
ringbuf.c - common ring buffer

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
#include <string.h>
#include "ringbuf.h"

// 判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

// 初始化缓冲区
uint32_t ringbuf_init(ringbuf_t *ring_buf, void *buffer, uint32_t size)
{
    if (!is_power_of_2(size)) {
        return 0;
    }
    
    memset(ring_buf, 0, sizeof(ringbuf_t));
    ring_buf->buffer = buffer;
    ring_buf->size = size;
    ring_buf->in = 0;
    ring_buf->out = 0;
    
    return 1;
}

// 缓冲区数据的长度
uint32_t ringbuf_use_len(const ringbuf_t *ring_buf)
{
    return (ring_buf->in - ring_buf->out);
}

// 判断缓冲区是否满
uint32_t ringbuf_is_full(const ringbuf_t *ring_buf)
{
    return (ring_buf->in - ring_buf->out == ring_buf->size);
}

// 从缓冲区中取数据
uint32_t ringbuf_get(ringbuf_t *ring_buf, void *buffer, uint32_t size)
{
    uint32_t len = 0;
    
#define MIN(a,b)    ((a)<=(b)?(a):(b))
    size = MIN(size, ring_buf->in - ring_buf->out);
    /* first get the data from fifo->out until the end of the buffer */
    len = MIN(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
    memcpy((uint8_t*)buffer, (uint8_t*)ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1)), len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy((uint8_t*)buffer + len, (uint8_t*)ring_buf->buffer, size - len);
    ring_buf->out += size;
    
    // buffer中没有数据
    if (ring_buf->in == ring_buf->out) {
        ring_buf->in = ring_buf->out = 0;
    }
    
    return size;
}

// 向缓冲区中存数据
uint32_t ringbuf_put(ringbuf_t *ring_buf, const void *buffer, uint32_t size)
{
    uint32_t len = 0;
    
    size = MIN(size, ring_buf->size - ring_buf->in + ring_buf->out);
    /* first put the data starting from fifo->in to buffer end */
    len  = MIN(size, ring_buf->size - (ring_buf->in & (ring_buf->size - 1)));
    memcpy((uint8_t*)ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1)), (uint8_t*)buffer, len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy((uint8_t*)ring_buf->buffer, (uint8_t*)buffer + len, size - len);
    ring_buf->in += size;
    
    return size;
}

// 向缓冲区中存字节数据
uint32_t ringbuf_putc(ringbuf_t *ring_buf, const uint8_t c)
{
    if (ringbuf_is_full(ring_buf)) {
        return 0;
    }

    *((uint8_t*)ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1))) = c;
    ring_buf->in++;
    
    return 1;
}

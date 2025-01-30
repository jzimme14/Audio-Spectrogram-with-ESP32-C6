#include <stdio.h>
#include <stdint.h>

class ringbuffer
{

    uint16_t ringbuf_size = 20;
    uint16_t ringbuf[ringbuf_size] = {0}; // 10^10 Samples
    uint16_t *ringbuf_ptr = ringbuf;
    uint16_t *ringbuf_start = ringbuf;
    uint16_t *ringbuf_end = &(ringbuf[ringbuf_size]);

    void ringbuf_add(uint32_t val)
    {
        if (ringbuf_ptr >= ringbuf_end)
        {
            ringbuf_ptr = ringbuf_start;
        }
        *ringbuf_ptr = val;
        ringbuf_ptr++;
    }
}
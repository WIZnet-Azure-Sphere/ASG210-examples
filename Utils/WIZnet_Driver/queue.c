// queue.c

#include "queue.h"
#include <stdlib.h>

void data_queueinit(str_qData* device, u8* buf, u32 depth)
{
    device->pool = buf;
    device->wp = 0;
    device->ovr = 0;
    device->rp = 0;
    device->depth = depth;

    #ifdef DEBUG_DATA_QUEUEINIT
    printf("Start DEBUG_DATA_QUEUEINIT\r\n");
    printf("device->pool = %#x\r\n", (u32)device->pool);
    printf("device->wp = %#x\r\n", device->wp);
    printf("device->ovr = %#x\r\n", device->ovr);
    printf("device->rp = %#x\r\n", device->rp);
    printf("device->depth = %#x\r\n", device->depth);
    #endif

    memset(device->pool, 0, (device->depth)*sizeof(device->pool));

    #ifdef DEBUG_DATA_QUEUEINIT
    printf("END DEBUG_DATA_QUEUEINIT\r\n");
    #endif
}

u32 data_queue_getlen(str_qData* device)
{
    u32 m4_data_wp = device->wp;
    u32 m4_data_over = device->ovr;
    u32 m4_data_rp = device->rp;
    u32 len;

    if (m4_data_over)
    {
        len = (device->depth - m4_data_rp) + m4_data_wp;
    }
    else
    {
        len = m4_data_wp - m4_data_rp;
    }

    return len;
}

void data_enqueue(str_qData* device, u8* data_p)
{
    u32 i;
    
    #ifdef DEBUG_DATA_ENQUEUE
    printf("Start DEBUG_DATA_ENQUEUE\r\n");
    #endif

    if(device->depth == data_queue_getlen(device))
    {
        // dequeue 1

        #ifdef DEBUG_DATA_ENQUEUE
        printf("device->depth full = %#x\r\n", device->depth);
        printf("force 1 dequeue\r\n");
        #endif

        u8 temp;

        if(0 == data_dequeue(device, &temp, 1))
        {
            printf("%s(%d)\r\n", __FILE__, __LINE__);
            printf("dequeue failed\r\n");
            return ;
        }
    }

    #ifdef DEBUG_DATA_ENQUEUE
    printf("data_p = %#x\r\n", (u32)data_p);
    printf("device->pool = %#x\r\n", (u32)device->pool);
    #endif

    device->pool[device->wp++] = *data_p;

    if (device->wp == device->depth)
    {
        device->wp = 0;
        device->ovr += 1;
    }

    #ifdef DEBUG_DATA_ENQUEUE
    printf("data_queue_getlen() = %#x\r\n", data_queue_getlen(device));
    printf("End DEBUG_DATA_ENQUEUE\r\n");
    #endif
}

void data_1dequeue(str_qData* device, u8* buf)
{
    memcpy(buf, &device->pool[device->rp++], (u32)sizeof(device->pool));
    if (device->rp == device->depth)
    {
        device->rp = 0;
        device->ovr -= 1;
    }
}

u32 data_dequeue(str_qData* device, u8* buf, u32 len)
{
    u32 rlen;

    #ifdef DEBUG_DATA_DEQUEUE
    printf("Start DEBUG_DATA_DEQUEUE\r\n");
    #endif

    #ifdef DEBUG_DATA_DEQUEUE
    printf("buf = %#x\r\n", (u32)buf);
    printf("len = %#x\r\n", len);
    printf("device->wp = %#x\r\n", device->wp);
    printf("device->rp = %#x\r\n", device->rp);
    #endif

    if (buf == NULL)
    {
        // Error return 0

        #ifdef DEBUG_DATA_DEQUEUE
        printf("%s(%d)\r\n", __FILE__, __LINE__);
        printf("device->rp = %#x\r\n", device->rp);
        printf("End DEBUG_DATA_DEQUEUE\r\n");
        #endif
        return 0;
    }
        
    rlen = data_queue_getlen(device);

    #ifdef DEBUG_DATA_DEQUEUE
    printf("rlen = %#x\r\n", rlen);
    #endif
    
    u32 i;
    if (len > rlen)
    {
        len = rlen;
    }

    for (i = 0; i < len; i++)
    {
        data_1dequeue(device, buf+i);
    }

    #ifdef DEBUG_DATA_DEQUEUE
    printf("%s(%d)\r\n", __FILE__, __LINE__);
    printf("device->rp = %#x\r\n", device->rp);
    printf("m4_data_queue_getlen(device) = %#x\r\n", data_queue_getlen(device));
    printf("End DEBUG_DATA_DEQUEUE\r\n");
    #endif

    return len;
}

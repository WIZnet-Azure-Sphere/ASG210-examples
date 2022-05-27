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

// 20210305 taylor
#if 1
    memcpy(buf, &device->pool[device->rp++], 1);
#else
    memcpy(buf, &device->pool[device->rp++], (u32)sizeof(device->pool));
#endif
    if (device->rp == device->depth)
    {
        device->rp = 0;
        device->ovr -= 1;
    }
}

void data_ndequeue(str_qData* device, u8* buf, u32 len)
{

    memcpy(buf, &device->pool[device->rp], len);

    device->rp += len;
        
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

// 20210311 taylor    
#if 1
    if (len > rlen)
    {
        len = rlen;
    }
    rlen = 0;
    
    if(device->ovr)
    {
        rlen = (device->depth)-(device->rp);
        #ifdef DEBUG_DATA_DEQUEUE
        printf("len = %d rlen = %d\r\n", len, rlen);
        #endif
        
        if(len > rlen)
        {
            // dequeue from device->rp to device->depth
            #ifdef DEBUG_DATA_DEQUEUE
            printf("1st read %d/%d\r\n", rlen, len);
            #endif
            data_ndequeue(device, buf, rlen);

            // dequeue from 0 to device->wp
            #ifdef DEBUG_DATA_DEQUEUE
            printf("2nd read %d/%d\r\n", len-rlen, len);
            #endif
            data_ndequeue(device, buf+rlen, len-rlen);
        }
        else
        {
            rlen = 0;
        }
    }
    
    if(rlen == 0)
    {
        #ifdef DEBUG_DATA_DEQUEUE
        printf("read %d\r\n", len);
        #endif
        data_ndequeue(device, buf, len);
    }
#else
    u32 i;
    if (len > rlen)
    {
        len = rlen;
    }

    for (i = 0; i < len; i++)
    {
        data_1dequeue(device, buf+i);
    }
#endif

    #ifdef DEBUG_DATA_DEQUEUE
    printf("%s(%d)\r\n", __FILE__, __LINE__);
    printf("device->rp = %#x\r\n", device->rp);
    printf("m4_data_queue_getlen(device) = %#x\r\n", data_queue_getlen(device));
    printf("End DEBUG_DATA_DEQUEUE\r\n");
    #endif

    return len;
}

void data_queue_disp(str_qData* device)
{
    u32 i;

    #ifdef DEBUG_DATA_DEQUEUE_DISP
    printf("Start DEBUG_DATA_QUEUE_DISP\r\n");
    #endif

    #ifdef DEBUG_DATA_DEQUEUE_DISP
    printf("ovr %d\r\n", device->ovr);
    printf("rp %d\r\n", device->rp);
    printf("wp %d\r\n", device->wp);
    printf("depth %d\r\n", device->depth);
    #endif
    
    for(i=0; i<device->depth; i++)
    {
        if(i%16 == 0)
        {
            printf("0x%.8x : ", i);
        }
        #if 0
        printf("%c ", device->pool[i]);
        #else
        printf("0x%.2x ", device->pool[i]);
        #endif
        if(i!=0 && i%16 == 15)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");

    #ifdef DEBUG_DATA_DEQUEUE_DISP
    printf("End DEBUG_DATA_QUEUE_DISP\r\n");
    #endif
}


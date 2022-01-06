// m4_data_queue.c

#include "m4_data_queue.h"
#if 1
// 20200730 taylor
#include <stdlib.h>
#endif

#if 0
#if 1
// 20200714 taylor
#define M4_DATA_ARRAY_MAX 1024
M4_Data* m4_data_array[M4_DATA_ARRAY_MAX];
u32 m4_data_array_wp;
#if 1
// 20200720 taylor
u32 m4_data_array_wp_over;
#endif
u32 m4_data_array_rp;
#endif
#endif

#if 1
// 20200722 taylor
#if 1
// 20200730 taylor
void m4_data_queueinit(str_qM4_Data* device, M4_Data** buf, u32 depth)
#else
void m4_data_queueinit(str_qM4_Data* device, M4_Data* buf, u32 depth)
#endif
{
    device->pool = buf;
    device->wp = 0;
    device->ovr = 0;
    device->rp = 0;
    device->depth = depth;

    #ifdef DEBUG_M4_DATA_QUEUEINIT
    printf("Start DEBUG_M4_DATA_QUEUEINIT\r\n");
    // 20200722 taylor
    #if 1
    // 20200730 taylor
    printf("device->pool = %#x\r\n", (u32)device->pool);
    #else
    printf("device->pool = %#x\r\n", device->pool);
    #endif
    printf("device->wp = %#x\r\n", device->wp);
    printf("device->ovr = %#x\r\n", device->ovr);
    printf("device->rp = %#x\r\n", device->rp);
    printf("device->depth = %#x\r\n", device->depth);
    #endif

    memset(device->pool, 0, (device->depth)*sizeof(device->pool));

    #ifdef DEBUG_M4_DATA_QUEUEINIT
    printf("END DEBUG_M4_DATA_QUEUEINIT\r\n");
    #endif
}
#else
void m4_data_queueinit()
{
    m4_data_array_wp = 0;
    #if 1
    // 20200720 taylor
    m4_data_array_wp_over = 0;
    #endif
    m4_data_array_rp = 0;
    memset(m4_data_array, 0, sizeof(m4_data_array));
}
#endif

#if 1
// 20200722 taylor
u32 m4_data_queue_getlen(str_qM4_Data* device)
{
    u32 m4_data_wp = device->wp;
    u32 m4_data_over = device->ovr;
    u32 m4_data_rp = device->rp;

    if (m4_data_over)
    {
        return (device->depth - m4_data_rp) + m4_data_wp;
    }
    else
    {
        return m4_data_wp - m4_data_rp;
    }
    return 0;
}
#else
u32 m4_data_queue_getlen()
{
    u32 m4_data_array_wp_temp = m4_data_array_wp;
    #if 1
    // 20200720 taylor
    u32 m4_data_array_wp_over_temp = m4_data_array_wp_over;
    #endif
    u32 m4_data_array_rp_temp = m4_data_array_rp;

    #if 1
    // 20200720 taylor

    if (m4_data_array_wp_over_temp)
    {
        return (M4_DATA_ARRAY_MAX-m4_data_array_rp_temp)+m4_data_array_wp_temp;
    }
    else
    {
        return m4_data_array_wp_temp - m4_data_array_rp_temp;
    }
    #else
    if (m4_data_array_rp_temp > m4_data_array_wp_temp)
    {
        return m4_data_array_wp_temp + (sizeof(m4_data_array) - m4_data_array_rp_temp);
    }
    else
    {
        return m4_data_array_wp_temp - m4_data_array_rp_temp;
    }
    #endif
}
#endif

#if 1
// 20200722 taylor
void m4_data_enqueue(str_qM4_Data* device, M4_Data* m4_data_p)
{
    #if 1
    // 20200730 taylor
    u32 i;
    #else
    u8* data;
    u32 i, j, k;
    #endif
    
    #ifdef DEBUG_M4_DATA_ENQUEUE
    printf("Start DEBUG_M4_DATA_ENQUEUE\r\n");
    #endif

    #if 1
    // 20200720 taylor
    
    if(device->depth == m4_data_queue_getlen(device))
    {
        // dequeue 1

        #ifdef DEBUG_M4_DATA_ENQUEUE
        printf("device->depth full = %#x\r\n", device->depth);
        printf("force 1 dequeue\r\n");
        #endif

        M4_Data* temp;

        #if 1
        // 20200730 taylor
        if(0 == m4_data_dequeue(device, (M4_Data**)&temp, 1))
        #else
        if(0 == m4_data_dequeue(device, &temp, 1))
        #endif
        {
            printf("%s(%d)\r\n", __FILE__, __LINE__);
            printf("dequeue failed\r\n");
            return ;
        }

        #ifdef DEBUG_M4_DATA_ENQUEUE
        printf("temp->Index[0] = %#x\r\n", temp->Index[0]);
        printf("temp->Index[1] = %#x\r\n", temp->Index[1]);
        printf("temp->DataCount = %#x\r\n", temp->DataCount);
        #endif

        // free allocated memory

        for (i=0; i<temp->DataCount; i++)
        {
            #ifdef DEBUG_M4_DATA_ENQUEUE
            #if 1
            // 20200730 taylor
            printf("temp->Data[%d] = %#x\r\n", i, (u32)temp->Data[i]);
            #else
            printf("temp->Data[%d] = %#x\r\n", i, temp->Data[i]);
            #endif
            #endif

            // free get_data.Data[i]
            if(temp->Data[i] != NULL)
            {
                #ifdef DEBUG_SEND_PARAMETER
                printf("free(temp->Data[%d]) = %#x\r\n", i, temp->Data[i]);
                #endif
                free(temp->Data[i]);
            }
        }

        // free get_data
        #ifdef DEBUG_SEND_PARAMETER
        printf("free(temp) = %#x\r\n", temp);
        #endif
        free(temp);
    }
    #endif

    #ifdef DEBUG_M4_DATA_ENQUEUE
    #if 1
    // 20200730 taylor
    printf("m4_data_p = %#x\r\n", (u32)m4_data_p);
    printf("device->pool = %#x\r\n", (u32)device->pool);
    #else
    printf("m4_data_p = %#x\r\n", m4_data_p);
    printf("device->pool = %#x\r\n", device->pool);
    #endif
    #endif

    device->pool[device->wp++] = m4_data_p;

    #if 1
    // 20200720 taylor

    if (device->wp == device->depth)
    {
        device->wp = 0;
        device->ovr += 1;
    }
    #endif

    #ifdef DEBUG_M4_DATA_ENQUEUE
    printf("m4_data_queue_getlen() = %#x\r\n", m4_data_queue_getlen(device));
    printf("End DEBUG_M4_DATA_ENQUEUE\r\n");
    #endif
}
#else
void m4_data_enqueue(M4_Data* m4_data_p)
{
    #define DEBUG_M4_DATA_ENQUEUE

    u8* data;
    u32 i, j, k;
    
    #ifdef DEBUG_M4_DATA_ENQUEUE
    printf("Start DEBUG_M4_DATA_ENQUEUE\r\n");
    #endif

    #if 1
    // 20200720 taylor
    
    if(M4_DATA_ARRAY_MAX == m4_data_queue_getlen())
    {
        // dequeue 1

        M4_Data* temp;

        if(0 == m4_data_dequeue(&temp, 1))
        {
            printf("%s(%d)\r\n", __FILE__, __LINE__);
            printf("dequeue failed\r\n");
            return ;
        }

        #ifdef DEBUG_M4_DATA_ENQUEUE
        printf("temp->Index[0] = %#x\r\n", temp->Index[0]);
        printf("temp->Index[1] = %#x\r\n", temp->Index[1]);
        printf("temp->DataCount = %#x\r\n", temp->DataCount);
        #endif

        // free allocated memory

        for (i=0; i<temp->DataCount; i++)
        {
            #ifdef DEBUG_M4_DATA_ENQUEUE
            printf("temp->Data[%d] = %#x\r\n", i, temp->Data[i]);
            #endif

            // free get_data.Data[i]
            if(temp->Data[i] != NULL)
            {
                free(temp->Data[i]);
            }
        }

        // free get_data
        free(temp);
    }
    #endif

    m4_data_array[m4_data_array_wp++] = m4_data_p;

    #if 1
    // 20200720 taylor

    if (m4_data_array_wp == M4_DATA_ARRAY_MAX)
    {
        m4_data_array_wp = 0;
        m4_data_array_wp_over += 1;
    }
    #endif

    #ifdef DEBUG_M4_DATA_ENQUEUE
    printf("m4_data_queue_getlen() = %#x\r\n", m4_data_queue_getlen());
    printf("End DEBUG_M4_DATA_ENQUEUE\r\n");
    #endif
}
#endif

#if 1
// 20200722 taylor
#if 1
// 20200730 taylor
void m4_data_1dequeue(str_qM4_Data* device, M4_Data** buf)
#else
void m4_data_1dequeue(str_qM4_Data* device, M4_Data* buf)
#endif
{
    #if 1
    // 20200730 taylor
    memcpy(buf, &device->pool[device->rp++], (u32)sizeof(device->pool));
    #else
    memcpy(buf, &device->pool[device->rp++], sizeof(device->pool));
    #endif
    if (device->rp == device->depth)
    {
        device->rp = 0;
        device->ovr -= 1;
    }
}
#else
void m4_data_1dequeue(M4_Data* buf)
{
    memcpy(buf, &m4_data_array[m4_data_array_rp++], sizeof(M4_Data*));
    if (m4_data_array_rp == M4_DATA_ARRAY_MAX)
    {
        m4_data_array_rp = 0;
        m4_data_array_wp_over -= 1;
    }
}
#endif

#if 1
// 20200722 taylor
#if 1
// 20200730 taylor
u32 m4_data_dequeue(str_qM4_Data* device, M4_Data** buf, u32 len)
#else
u32 m4_data_dequeue(str_qM4_Data* device, M4_Data* buf, u32 len)
#endif
{
    u32 rlen;
    #if 0
    // 20200730 taylor
    #if 1
    // 20200720 taylor
    u32 rp1len;
    u32 rp2len;
    #endif
    u32 m4_data_wp = device->wp;
    #endif

    #if 0
    // 20200720 taylor
    if (m4_data_array_rp == M4_DATA_ARRAY_MAX)
    {
        m4_data_array_rp = 0;
    }
    #endif

    #ifdef DEBUG_M4_DATA_DEQUEUE
    printf("Start DEBUG_M4_DATA_DEQUEUE\r\n");
    #endif

    #ifdef DEBUG_M4_DATA_DEQUEUE
    #if 1
    // 20200730 taylor
    printf("buf = %#x\r\n", (u32)buf);
    #else
    printf("buf = %#x\r\n", buf);
    #endif
    printf("len = %#x\r\n", len);
    printf("device->wp = %#x\r\n", device->wp);
    printf("device->rp = %#x\r\n", device->rp);
    #endif

    if (buf == NULL)
    {
        // Error return 0

        #ifdef DEBUG_M4_DATA_DEQUEUE
        printf("%s(%d)\r\n", __FILE__, __LINE__);
        printf("device->rp = %#x\r\n", device->rp);
        printf("End DEBUG_M4_DATA_DEQUEUE\r\n");
        #endif
        return 0;
    }
        
    rlen = m4_data_queue_getlen(device);

    #ifdef DEBUG_M4_DATA_DEQUEUE
    printf("rlen = %#x\r\n", rlen);
    #endif
    
    u32 i;
    if (len > rlen)
    {
        len = rlen;
    }

    for (i = 0; i < len; i++)
    {
        m4_data_1dequeue(device, buf+i);
    }

    #ifdef DEBUG_M4_DATA_DEQUEUE
    printf("%s(%d)\r\n", __FILE__, __LINE__);
    printf("device->rp = %#x\r\n", device->rp);
    printf("m4_data_queue_getlen(device) = %#x\r\n", m4_data_queue_getlen(device));
    printf("End DEBUG_M4_DATA_DEQUEUE\r\n");
    #endif

    return len;
}
#else
u32 m4_data_dequeue(M4_Data* buf, u32 len)
{
    //#define DEBUG_M4_DATA_DEQUEUE

    u32 rlen;
    #if 1
    // 20200720 taylor
    u32 rp1len;
    u32 rp2len;
    #endif
    u32 m4_data_array_wp_temp = m4_data_array_wp;

    #if 0
    // 20200720 taylor
    if (m4_data_array_rp == M4_DATA_ARRAY_MAX)
    {
        m4_data_array_rp = 0;
    }
    #endif

    #ifdef DEBUG_M4_DATA_DEQUEUE
    printf("Start DEBUG_M4_DATA_DEQUEUE\r\n");
    #endif

    #ifdef DEBUG_M4_DATA_DEQUEUE
    printf("buf = %#x\r\n", buf);
    printf("len = %#x\r\n", len);
    #endif

    if (buf == NULL)
    {
        // Error return 0

        #ifdef DEBUG_M4_DATA_DEQUEUE
        printf("%s(%d)\r\n", __FILE__, __LINE__);
        printf("m4_data_array_rp = %#x\r\n", m4_data_array_rp);
        printf("End DEBUG_M4_DATA_DEQUEUE\r\n");
        #endif
        return 0;
    }
        
    rlen = m4_data_queue_getlen();

    #ifdef DEBUG_M4_DATA_DEQUEUE
    printf("rlen = %#x\r\n", rlen);
    #endif
    
    u32 i;
    if (len > rlen)
    {
        len = rlen;
    }

    for (i = 0; i < len; i++)
    {
        m4_data_1dequeue(buf+i);
    }

    #ifdef DEBUG_M4_DATA_DEQUEUE
    printf("%s(%d)\r\n", __FILE__, __LINE__);
    printf("m4_data_array_rp = %#x\r\n", m4_data_array_rp);
    printf("m4_data_queue_getlen() = %#x\r\n", m4_data_queue_getlen());
    printf("End DEBUG_M4_DATA_DEQUEUE\r\n");
    #endif

    return len;
}
#endif


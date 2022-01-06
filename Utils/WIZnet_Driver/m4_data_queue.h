// m4_data_queue.h

#ifndef __M4_DATA_QUEUE_H__
#define __M4_DATA_QUEUE_H__

#include "mhal_osai.h"
#include "ParseMboxData.h"

#if 1
// 20200722 taylor
typedef struct _M4_Data_Queue
{
    M4_Data** pool;
    u32 wp;
    u32 ovr;
    u32 rp;
    u32 depth;

}str_qM4_Data;
#endif

//#define DEBUG_M4_DATA_QUEUEINIT
//#define DEBUG_M4_DATA_ENQUEUE
//#define DEBUG_M4_DATA_DEQUEUE

#if 1
// 20200722 taylor
#if 1
// 20200730 taylor
void m4_data_queueinit(str_qM4_Data* device, M4_Data** buf, u32 depth);
#else
void m4_data_queueinit(str_qM4_Data* device, M4_Data* buf, u32 depth);
#endif
u32 m4_data_queue_getlen(str_qM4_Data* device);
void m4_data_enqueue(str_qM4_Data* device, M4_Data* m4_data_p);
#if 1
// 20200730 taylor
void m4_data_1dequeue(str_qM4_Data* device, M4_Data** buf);
u32 m4_data_dequeue(str_qM4_Data* device, M4_Data** buf, u32 len);
#else
void m4_data_1dequeue(str_qM4_Data* device, M4_Data* buf);
u32 m4_data_dequeue(str_qM4_Data* device, M4_Data* buf, u32 len);
#endif
#else
void m4_data_queueinit();
u32 m4_data_queue_getlen();
void m4_data_enqueue(M4_Data* m4_data_p);
u32 m4_data_dequeue(M4_Data* buf, u32 len);
#endif

#endif


// queue.h

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "mhal_osai.h"

typedef struct _Queue
{
    u8* pool;
    u32 wp;
    u32 ovr;
    u32 rp;
    u32 depth;

}str_qData;

//#define DEBUG_DATA_QUEUEINIT
//#define DEBUG_DATA_ENQUEUE
//#define DEBUG_DATA_DEQUEUE

void data_queueinit(str_qData* device, u8* buf, u32 depth);
u32 data_queue_getlen(str_qData* device);
void data_enqueue(str_qData* device, u8* data_p);
void data_1dequeue(str_qData* device, u8* buf);
u32 data_dequeue(str_qData* device, u8* buf, u32 len);

#endif

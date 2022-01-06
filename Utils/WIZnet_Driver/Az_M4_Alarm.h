#ifndef __AZ_M4_ALARM_H
#define __AZ_M4_ALARM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ParseMboxData.h"

typedef struct Alarm_Period_
{
    unsigned int pre_time;
    unsigned int Al_period;
    M4_Data Al_M4_Data;
}Alarm_Period;

int Set_Alarm_M4_Data(M4_Data *M_Data, Alarm_Period *Al_Data);
int Get_Alarm_M4_Data(M4_Data *M_Data, Alarm_Period *Al_Data, char Index);
#if 1   //teddy 200730
int Alarm_Process(unsigned int Times, M4_Data **M_Data, Alarm_Period *Al_Data);
#else 
int Alarm_Process(unsigned int Times, M4_Data* M_Data, Alarm_Period* Al_Data);
#endif

#ifdef __cplusplus
}
#endif
#endif
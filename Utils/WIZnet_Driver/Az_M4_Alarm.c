#include "Az_M4_Alarm.h"
#include "Az_M4_WIZ750SR.h"
#include "ParseMboxData.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "printf.h"

int Set_Alarm_M4_Data(M4_Data *M_Data, Alarm_Period *Al_Data)
{
    unsigned int Temp_Data = 0;
    int Temp_Index = M_Data->Index[1] & 0x0F;
    int i;
    if((M_Data->Index[1] & 0xf0) != 0x80)// 0x80 ~0x8f
    {
        printf("Error Alarm Index!![%02x] \r\n", M_Data->Index[1]);
        return -1;
    }
    Temp_Data = atoi(M_Data->Data[0]);
    printf("Alarm Period : %d\r\n", Temp_Data);
    if((Temp_Data == 0)&&(Temp_Data >30000000))   //1year=31,536,000s 
    {
        printf("Error Alarm Period!! \r\n");
        return -2;
    }
    Al_Data[Temp_Index].Al_period = Temp_Data;
    Al_Data[Temp_Index].pre_time = 0;
    if(strlen(M_Data->Data[1]) > 4)
    {
        printf("Error Alarm CMD Index Format!! \r\n");
        return -3;
    }
    Al_Data[Temp_Index].Al_M4_Data.Index[0] = char2hex(M_Data->Data[1]);
    Al_Data[Temp_Index].Al_M4_Data.Index[1] = char2hex(M_Data->Data[1] + 2);
    printf("Al CMD Index[%02x][%02x]\r\n", Al_Data[Temp_Index].Al_M4_Data.Index[0], Al_Data[Temp_Index].Al_M4_Data.Index[1]);
    for(i=2; i<M_Data->DataCount; i++)
    {
        if(M_Data->Data[i] == NULL)
            continue;
        else
        {
            Al_Data[Temp_Index].Al_M4_Data.Data[i - 2] = (char *)calloc(strlen(M_Data->Data[i]) + 1, sizeof(char));
            memcpy(Al_Data[Temp_Index].Al_M4_Data.Data[i - 2], M_Data->Data[i], strlen(M_Data->Data[i]));
        }
    }
    Al_Data[Temp_Index].Al_M4_Data.DataCount = M_Data->DataCount - 2;
    return 1;
}

int Get_Alarm_M4_Data(M4_Data *M_Data, Alarm_Period *Al_Data, char Index)
{
    int i;
    int Temp_Index = Index & 0x0F;
    char Temp_buffer[50]={0,};
    int Temp_buffer_size = 0;
    M_Data->DataCount = 0;
    if((Index & 0xF0) != 0x00)// 0x00 ~0x0f
    {
        printf("Error Alarm Index!![%02x] \r\n", M_Data->Index[1]);
        return -1;
    }
    M_Data->Index[0] = 0x00;
    M_Data->Index[0] = Index;
    Temp_buffer_size = sprintf(Temp_buffer, "%d", Al_Data[Temp_Index].Al_period);
    M_Data->Data[M_Data->DataCount] = (char *)calloc(Temp_buffer_size + 1, sizeof(char));
    memcpy(M_Data->Data[M_Data->DataCount++], Temp_buffer, sizeof(char)*Temp_buffer_size);
    memset(Temp_buffer, 0, sizeof(Temp_buffer));
    Temp_buffer_size = sprintf(Temp_buffer, "%02x%02x", Al_Data[Temp_Index].Al_M4_Data.Index[0], Al_Data[Temp_Index].Al_M4_Data.Index[1]);
    M_Data->Data[M_Data->DataCount] = (char *)calloc(Temp_buffer_size + 1, sizeof(char));
    memcpy(M_Data->Data[M_Data->DataCount++], Temp_buffer, sizeof(char)*Temp_buffer_size);
    for(i=0;i < Al_Data[Temp_Index].Al_M4_Data.DataCount;i++)
    {
        if(Al_Data[Temp_Index].Al_M4_Data.Data[i] == NULL)
        {
            M_Data->Data[M_Data->DataCount++] = NULL;
            continue;
        }
        Temp_buffer_size = (int)strlen(Al_Data[Temp_Index].Al_M4_Data.Data[i]);
        M_Data->Data[M_Data->DataCount] = (char *)calloc(Temp_buffer_size + 1, sizeof(char));
        memcpy(M_Data->Data[M_Data->DataCount++], Al_Data[Temp_Index].Al_M4_Data.Data[i], Temp_buffer_size);
    }
    return 1;
}
#if 1   //teddy 200730
int Alarm_Process(unsigned int Times, M4_Data **M_Data, Alarm_Period* Al_Data)
#else
int Alarm_Process(unsigned int Times, M4_Data *M_Data, Alarm_Period *Al_Data)
#endif
{
    if(Al_Data->Al_period == 0)
        return -1;
    if(Al_Data->pre_time == 0)
    {
        Al_Data->pre_time = Times;
        printf("alarm Timer reset %d \r\n", Al_Data->pre_time);
        return 0;
    }
    if(Times >= Al_Data->pre_time + Al_Data->Al_period)
    {
        //make M4_Data;
        Al_Data->pre_time = Times;
        printf("alarm Timer execute %d \r\n", Al_Data->pre_time);
        *M_Data = (M4_Data*)calloc(1, sizeof(M4_Data));
        M4_Data_cpy(*M_Data, &Al_Data->Al_M4_Data);
        return 1;
    }
    return 0;
}


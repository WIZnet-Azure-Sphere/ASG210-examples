#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include "printf.h"
#include "ParseMboxData.h"


void MessagePrint(char *indata, int size)
{
    char TempBuff[1024] = {0,};
    M_Lens tempLens;

// 20201208 taylor
#if 1
    printf("Start DEBUG_MESSAGEPRINT\r\n");
#endif
    
    tempLens.len_8bit[1] = indata[1];
    tempLens.len_8bit[0] = indata[2];
    if (tempLens.len_16bit == 2)
    {
        printf("message lens : %d\r\n", size);
        printf("[%02x] [%02x] [%02x] [%02x] [%02x] [%02x]\r\n", indata[0], indata[1], indata[2], indata[3], indata[4], indata[5]);
// 20201208 taylor
#if 1
        printf("End DEBUG_MESSAGEPRINT\r\n");
#endif
        return;
    }
    strncpy(TempBuff, indata + 5, size - 6);
    printf("message lens : %d\r\n", size);
    printf("[%02x] [%02x] [%02x] [%02x] [%02x] [%s] [%02x]\r\n", indata[0], indata[1], indata[2], indata[3], indata[4], TempBuff, indata[size - 1]);
// 20201208 taylor
#if 1
    printf("End DEBUG_MESSAGEPRINT\r\n");
#endif
}

int parse_Str_M4_Data(char *indata, M4_Data *M_Data)
{
//#define DEBUG_PARSE_STR_M4_DATA

    char *p = indata;
    char *Temp_Data = p;
    M_Data->DataCount = 0;
    char checkETX = 1;
    int Temp_Data_Len = 0;
    if (*indata == 0x03)
    {
        // Data is not exist
        M_Data->DataCount = 1;
        M_Data->Data[0] = NULL;
        printf("Data is ETX[%02x]!! \r\n", *indata);
        return M_Data->DataCount;
    }
    else if (isgraph(*indata) == 0)
    {
        // Data is not ascii code
        printf("Data is not ASCii Code[%c]!! \r\n", *indata);
        return -10;
    }
    while(1)
	{
		p = strchr(p, ',');
		if(p == NULL)
		{
			//end loop & save last data
			p = strchr(Temp_Data, 0x03);
// 20201210 taylor
#ifdef DEBUG_PARSE_STR_M4_DATA
			printf("Last Data %d \r\n", M_Data->DataCount);
            printf("Str M4 Index[%02x][%02x] \r\n", M_Data->Index[0], M_Data->Index[1]);
#endif
			checkETX = 0;
		}
		if(Temp_Data == p)
		{
			//data none
			M_Data->Data[M_Data->DataCount++] = NULL;
		}
		else
		{
			//data exist
			*p = 0;
			Temp_Data_Len = (int)strlen(Temp_Data);
			if(Temp_Data_Len == 0)
			{
				//data len = 0
				M_Data->Data[M_Data->DataCount++] = NULL;
			}
			else
			{
				//data len > 0
				M_Data->Data[M_Data->DataCount] = (char *)calloc(Temp_Data_Len + 1, sizeof(char));
				memcpy(M_Data->Data[M_Data->DataCount++], Temp_Data, Temp_Data_Len);
			}
		}
// 20201210 taylor
#ifdef DEBUG_PARSE_STR_M4_DATA
		printf("count : %d M_Data->DataCount : %d\r\n", M_Data->DataCount, M_Data->DataCount);
#endif
		if(checkETX == 0)
			return M_Data->DataCount;
		Temp_Data = ++p;
	}
	return 0;
}
int parse_M4_Data(char *indata, M4_Data *M_Data)
{
//#define DEBUG_PARSE_M4_DATA
	char *p = indata;
	M_Lens tempLens;
	//char *Temp_Data = NULL;
	//int Temp_Data_Len = 0;
	//int Index_Data = 0;
	int cnt = 0;
	char checkETX = 0;

// 20201208 taylor
#ifdef DEBUG_PARSE_M4_DATA
    printf("Start DEBUG_PARSE_M4_DATA\r\n");
#endif

	if(*p++ != 0x02)
	{
		printf("STX Error\r\n");
// 20201208 taylor
#ifdef DEBUG_PARSE_M4_DATA
        printf("End DEBUG_PARSE_M4_DATA\r\n");
#endif
		return -1;
	}
	tempLens.len_8bit[1] = *p++;
	tempLens.len_8bit[0] = *p++;
    //tempLens.len_8bit[0] = *p++;
    //tempLens.len_8bit[1] = *p++;
// 20201210 taylor
#ifdef DEBUG_PARSE_M4_DATA    
	printf("In len[%02x][%02x] = %d \r\n", tempLens.len_8bit[0], tempLens.len_8bit[1], tempLens.len_16bit);
#endif
	M_Data->Index[0] = *p++;
	M_Data->Index[1] = *p++;
// 20201210 taylor
#ifdef DEBUG_PARSE_M4_DATA    
	printf("index[0]:%02x [1]:%02x \r\n", M_Data->Index[0], M_Data->Index[1]);
#endif

// 20210121 taylor
#if 0
    if(tempLens.len_16bit == 0)
    {
        if(indata[cnt] == 0x03)
		{
			checkETX = 1;
		}
    }
    else
    {
        for(cnt = 5; cnt < tempLens.len_16bit + 4; cnt++)
    	{
    		if(indata[cnt] == 0x03)
    		{
    			checkETX = 1;
    			break;
    		}
    	}
    }
#else
	for(cnt = 5; cnt < tempLens.len_16bit + 4; cnt++)
	{
		if(indata[cnt] == 0x03)
		{
			checkETX = 1;
			break;
		}
	}
#endif
	if(checkETX == 0)
	{
		// don't find ETX
		printf("ETX Error\r\n");
// 20201208 taylor
#ifdef DEBUG_PARSE_M4_DATA
        printf("End DEBUG_PARSE_M4_DATA\r\n");
#endif
		return -2;
	}
// 20201208 taylor
#ifdef DEBUG_PARSE_M4_DATA
    printf("End DEBUG_PARSE_M4_DATA\r\n");
#endif
    return parse_Str_M4_Data(p, M_Data);
}
int check_M4_Data_Size(M4_Data *M_Data)
{
    int DataSize = 0;
    int i;

#if 1
// 20201123 taylor
    if(M_Data->DataCount == 0)
    {
        return 0;
    }
#endif
    
    for(i=0; i < M_Data->DataCount; i++)
    {
        if(M_Data->Data[i] != NULL)
        {
            DataSize+=(int)strlen(M_Data->Data[i]) + 1;
        }
        else
        {
            DataSize++;
        }
        
    }
    return DataSize - 1;
}
int check_Mass_M4_Data_Size(M4_Data *M_Data, int count)
{
    int DataSize = 0;
    int i, j;
    for(j=0; j < count; j++)
    {
        for(i=0; i < M_Data[j].DataCount; i++)
        {
            if(M_Data[j].Data[i] != NULL)
            {
                DataSize+=(int)strlen(M_Data[j].Data[i]) + 1;
            }
            else
            {
                DataSize++;
            }
        }
    }
    return DataSize - 1;
}
int make_M4_Data(char *outdata, M4_Data *M_Data)
{
//#define DEBUG_MAKE_M4_DATA

    char *p = outdata;
    M_Lens tempLens;
    int i;
    *p++ = 0x02;
    tempLens.len_16bit = check_M4_Data_Size(M_Data) + 2;
// 20201210 taylor
#ifdef DEBUG_MAKE_M4_DATA
    printf("In len = %d \r\n", tempLens.len_16bit);
#endif
    *p++ = tempLens.len_8bit[1];
    *p++ = tempLens.len_8bit[0];
    *p++ = M_Data->Index[0];
    *p++ = M_Data->Index[1];
    for(i=0; i < M_Data->DataCount; i++)
    {
        if(M_Data->Data[i] != NULL)
        {
            memcpy(p, M_Data->Data[i], strlen(M_Data->Data[i]));
            p+= (int)strlen(M_Data->Data[i]);
        }
        if(i != M_Data->DataCount - 1)
        {
            *p++ = ',';
        }
    }
    *p++ = 0x03;
    return tempLens.len_16bit + 4;
}
int make_Mass_M4_Data(char *outdata, M4_Data *M_Data, int count)
{
    char *p = outdata;
    M_Lens tempLens;
    int i, j;
    *p++ = 0x02;
    tempLens.len_16bit = check_Mass_M4_Data_Size(M_Data, count) + 2;
    printf("In len = %d \r\n", tempLens.len_16bit);
    *p++ = tempLens.len_8bit[1];
    *p++ = tempLens.len_8bit[0];
    *p++ = M_Data[0].Index[0];
    *p++ = M_Data[0].Index[1];
    for(j = 0; j < count; j++)
    {
        for(i=0; i < M_Data[j].DataCount; i++)
        {
            if(M_Data[j].Data[i] != NULL)
            {
                memcpy(p, M_Data[j].Data[i], strlen(M_Data[j].Data[i]));
                p+= (int)strlen(M_Data[j].Data[i]);
            }
            if(i != M_Data[j].DataCount - 1)
            {
                *p++ = ',';
            }
        }
        if(j != count - 1)
            *p++ = ';';
    }
    *p++ = 0x03;
    return tempLens.len_16bit + 4;
}
void FreeM4_Data(M4_Data *M_Data)
{
	int i;
	for(i = 0; i < M_Data->DataCount; i++)
	{
		if(M_Data->Data[i] != NULL)
		{
			free(M_Data->Data[i]);
			M_Data->Data[i] = NULL;
		}
	}
	M_Data->DataCount = 0;
}
void FreeMassM4_Data(M4_Data *M_Data, int count)
{
    int i, j;
    for(j = 0; j < count; j++)
    {
        for(i = 0; i < M_Data[j].DataCount; i++)
        {
            if(M_Data[j].Data[i] != NULL)
            {
                free(M_Data[j].Data[i]);
                M_Data[j].Data[i] = NULL;
            }
        }
        M_Data[j].DataCount = 0;
    }
}

int M4_Data_cpy(M4_Data *out_Data, M4_Data *in_Data)
{
    int i, Temp_size = 0;
    out_Data->Index[0] = in_Data->Index[0];
    out_Data->Index[1] = in_Data->Index[1];
    out_Data->DataCount = in_Data->DataCount;
    for(i=0; i < in_Data->DataCount; i++)
    {
        if(in_Data->Data[i] == NULL)
        {
            out_Data->Data[i] = NULL;
            continue;
        }
        Temp_size = (int)strlen(in_Data->Data[i]);
        out_Data->Data[i] = (char *)calloc(Temp_size + 1, sizeof(char));
        memcpy(out_Data->Data[i], in_Data->Data[i], Temp_size);
    }
    return 0;
}
#ifndef __PARSEMBOXDATA_H
#define __PARSEMBOXDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_D_CNT 		50

typedef union M_Lens_
{
	char len_8bit[2];
// 20201208 taylor
#if 1
    unsigned short len_16bit;
#else
	int len_16bit;
#endif
}M_Lens;

typedef struct M4_Data_
{
	char Index[2];
	int DataCount;
	char *Data[MAX_D_CNT];
}M4_Data;

int parse_M4_Data(char *indata, M4_Data *M_Data);
int parse_Str_M4_Data(char *indata, M4_Data *M_Data);
int make_M4_Data(char *outdata, M4_Data *M_Data);
int make_Mass_M4_Data(char *outdata, M4_Data *M_Data, int count);
int check_M4_Data_Size(M4_Data *M_Data);
int check_Mass_M4_Data_Size(M4_Data *M_Data, int count);
void FreeM4_Data(M4_Data *M_Data);
void FreeMassM4_Data(M4_Data *M_Data, int count);
void MessagePrint(char *indata, int size);
int M4_Data_cpy(M4_Data *out_Data, M4_Data *in_Data);
#ifdef __cplusplus
}
#endif
#endif
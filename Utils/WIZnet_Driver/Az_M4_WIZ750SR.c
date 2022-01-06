#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Az_M4_WIZ750SR.h"
#include "printf.h"

uint16_t myport = 12345;
uint16_t cnt = 0;
uint8_t init_flag = true;
WIZ750SR_INFO  info;

uint8_t destip[4] = {255,255,255,255};
uint16_t destport = 50001;

// becky 200805
#if 1
uint8_t broad_ip[4] = { 255,255,255,255 };
uint16_t upd_dest_port = 50001;

#define FWSENDSIZE      1024
#endif

#define SEARCHTIME      50  //100ms
#define SETTIME         20  //100ms
#define WIZ750SRINDEX   0x11
uint8_t UDPSocket;
uint8_t TCPSocket;

#if 1   //teddy 200805
str_qM4_Data Wiz750SR;
#define Wiz750SR_BUF_MAX 1024
M4_Data* Wiz750SR_buf[Wiz750SR_BUF_MAX];
#endif
uint8_t SearchMAC[23] = {0x4D, 0x41, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0d, 0x0a, 0x50, 0x57, 0x20, 0x0d, 0x0a, 0x4d, 0x43, 0x0d, 0x0a, 0x56, 0x52, 0x0d, 0x0a};

// 20201204 taylor
#if 1
char *eCmd[255] = {"MC", "VR", "MN", "IM", "OP", "DD", "CP", "PO", "DG", "KA",
              "KI", "KE", "RI", "LI", "SM", "GW", "DS", "PI", "PP", "DX",
              "DP", "DI", "DW", "DH", "LP", "RP", "RH", "BR", "DB", "PR", 
              "SB", "FL", "IT", "PT", "PS", "PD", "TE", "SS", "NP", "SP", 
              "LG", "ER", "FW", "MA", "PW", "SV", "EX", "RT", "UN", "ST", 
              "FR", "EC", "K1", "UE", "GA", "GB", "GC", "GD", "CA", "CB",
              "CC", "CD", "SC", "S0", "S1", "RX", "FS", "FC", "FP", "FD",
              "FH", "UI", "AB", "TR", "BU"
              // For WIZ750SR Waton
              #if 1
              ,
              "AL", "AR", "AP", "TL", "T0", "T1", "T2", "T3", "T4", "P0",
              "P1", "P2", "P3", "P4"
              #endif
              };
#else
char *eCmd[255] = {"MC", "VR", "MN", "IM", "OP", "DD", "CP", "PO", "DG", "KA",
              "KI", "KE", "RI", "LI", "SM", "GW", "DS", "PI", "PP", "DX",
              "DP", "DI", "DW", "DH", "LP", "RP", "RH", "BR", "DB", "PR", 
              "SB", "FL", "IT", "PT", "PS", "PD", "TE", "SS", "NP", "SP", 
              "LG", "ER", "FW", "MA", "PW", "SV", "EX", "RT", "UN", "ST", 
              "FR", "EC", "K1", "UE", "GA", "GB", "GC", "GD", "CA", "CB",
              "CC", "CD", "SC", "S0", "S1", "RX", "FS", "FC", "FP", "FD",
              "FH", "UI", "AB", "TR", "BU"};
#endif

#if 1 //becky 200806
unsigned int Command_List[100][100] = { {9, CMD_MC, CMD_VR, CMD_MN, CMD_ST, CMD_IM, CMD_OP, CMD_LI, CMD_SM, CMD_GW},  //0x00 search
                                {5, CMD_MC,CMD_VR, CMD_MN, CMD_ST, CMD_UN}, //0x01 get device info
                                {9, CMD_OP,CMD_IM, CMD_LI, CMD_SM, CMD_GW, CMD_DS, CMD_LP, CMD_RH, CMD_RP}, //0x02 get network
                                {11, CMD_OP,CMD_IM, CMD_LI, CMD_SM, CMD_GW, CMD_DS, CMD_LP, CMD_RH, CMD_RP, CMD_SV, CMD_RT}, //0x03 set network
                                {5, CMD_BR, CMD_DB, CMD_PR, CMD_SB, CMD_FL}, //0x04 Get Data UART Settings
                                {7, CMD_BR, CMD_DB, CMD_PR, CMD_SB, CMD_FL, CMD_SV, CMD_RT}, //0x05 Set Data UART Settings
                                {3, CMD_PT, CMD_PS, CMD_PD}, // 0x06 Get Serial Data Packing Options
                                {5, CMD_PT, CMD_PS, CMD_PD, CMD_SV, CMD_RT}, // 0x07 Set Serial Data Packing Options
                                {10, CMD_IT, CMD_CP, CMD_NP, CMD_SP, CMD_DG, CMD_KA, CMD_KI, CMD_KE, CMD_RI, CMD_EC}, //0x08 Get Options
                                {12, CMD_IT, CMD_CP, CMD_NP, CMD_SP, CMD_DG, CMD_KA, CMD_KI, CMD_KE, CMD_RI, CMD_EC, CMD_SV, CMD_RT}, //0x09 Set Options
                                {2, CMD_TE, CMD_SS}, // 0x0A get command mode switch settings
                                {4, CMD_TE, CMD_SS, CMD_SV, CMD_RT}, //0x0B set command mode switch settings    
                                {4, CMD_EX, CMD_SV, CMD_RT, CMD_FR}, //0x0C get device control
                                {6, CMD_EX, CMD_SV, CMD_RT, CMD_FR, CMD_SV, CMD_RT}  //0x0D set device control
                                // 20201204
                                #if 1
                                ,
                                {0},    // 0x0E
                                {0},    // 0x0F
                                {0},    // 0x10
                                {0},    // 0x11
                                {0},    // 0x12
                                {14, CMD_AL, CMD_AR, CMD_AP, CMD_TL, CMD_T0, CMD_P0, CMD_T1, CMD_P1, CMD_T2, CMD_P2, CMD_T3, CMD_P3, CMD_T4, CMD_P4},                   // 0x13 Get Waton
                                {14+2, CMD_AL, CMD_AR, CMD_AP, CMD_TL, CMD_T0, CMD_P0, CMD_T1, CMD_P1, CMD_T2, CMD_P2, CMD_T3, CMD_P3, CMD_T4, CMD_P4, CMD_SV, CMD_RT}  // 0x14 Set Waton
                                #endif

};
#else
unsigned int Command_List[100][100] = {	{9, CMD_MC, CMD_VR, CMD_MN, CMD_ST, CMD_IM, CMD_OP, CMD_LI, CMD_SM, CMD_GW},  //0x00 search
								{5, CMD_MC,CMD_VR, CMD_MN, CMD_ST, CMD_UN}, //0x01 get device info
								{9, CMD_OP,CMD_IM, CMD_LI, CMD_SM, CMD_GW, CMD_DS, CMD_LP, CMD_RH, CMD_RP}, //0x02 get network
								{9, CMD_OP,CMD_IM, CMD_LI, CMD_SM, CMD_GW, CMD_DS, CMD_LP, CMD_RH, CMD_RP}, //0x03 set network
								{5, CMD_BR, CMD_DB, CMD_PR, CMD_SB, CMD_FL}, //0x04 Get Data UART Settings
								{5, CMD_BR, CMD_DB, CMD_PR, CMD_SB, CMD_FL}, //0x05 Set Data UART Settings
								{3, CMD_PT, CMD_PS, CMD_PD}, // 0x06 Get Serial Data Packing Options
								{3, CMD_PT, CMD_PS, CMD_PD}, // 0x07 Set Serial Data Packing Options
                                {10, CMD_IT, CMD_CP, CMD_NP, CMD_SP, CMD_DG, CMD_KA, CMD_KI, CMD_KE, CMD_RI, CMD_EC}, //0x08 Get Options
                                {10, CMD_IT, CMD_CP, CMD_NP, CMD_SP, CMD_DG, CMD_KA, CMD_KI, CMD_KE, CMD_RI, CMD_EC}, //0x09 Set Options
                                {2, CMD_TE, CMD_SS}, // 0x0A get command mode switch settings
                                {2, CMD_TE, CMD_SS}, //0x0B set command mode switch settings    
                                {4, CMD_EX, CMD_SV, CMD_RT, CMD_FR}, //0x0C get device control
                                {4, CMD_EX, CMD_SV, CMD_RT, CMD_FR}  //0x0D set device control

								};
#endif
//입력(data)'0'(0x30) or 'a'(0x41) or 'A'(0x61) 
//출력(return) 0x00 or '0x0a'  
char char2hex(char* data)
{
    unsigned char result;
    if (data[0] & 0x40) result = (data[0] & 0x0f) + 9;
    else result = data[0] & 0x0f;
    if (data[1] & 0x40) result = (result << 4) | ((data[1] & 0x0f) + 9);
    else result = (result << 4) | (data[1] & 0x0f);
    return result;
}

//입력(intmac) 0x00
//출력(strmac) "00"
char hex2char(unsigned char intmac, char *strmac)
{
    char *data=strmac;
    char tmp;
    tmp=(intmac&0xf0) >>4;
    if( tmp < 10 ) data[0]= tmp+ 0x30;
    else data[0] = tmp + 0x41 - 0x0a;
    // printf(" data0 : %c\r\n", data[0]);
    tmp=(intmac&0x0f);
    
    if(tmp  < 10 ) data[1]= tmp+ 0x30;
    else data[1] = tmp + 0x41 - 0x0a;
    data[2]=0;
    // printf(" data1 : %c\r\n", data[1]);
    // printf("hex2char\r\n");
    // printf("%s\r\n", data);
    return 0;
}

//입력(strmac)00:08:dc:57:bb:e8
//출력(intmac) {0x00, 0x08, 0xdc, 0x57, 0xbb, 0xe8} 
void str2hex_mac(char* strmac, unsigned char* intmac)
{
	intmac[0]= char2hex(strmac);
	intmac[1]=char2hex(strmac+3);
	intmac[2]=char2hex(strmac+6);
	intmac[3]=char2hex(strmac+9);
	intmac[4]=char2hex(strmac+12);
	intmac[5]=char2hex(strmac+15);
}

//입력(intmac) {0x00, 0x08, 0xdc, 0x57, 0xbb, 0xe8} 
//출력(strmac) "00:08:dc:57:bb:e8"
void hex2str_mac(unsigned char* intmac, char* strmac)
{
    char tmp[2];
    hex2char(intmac[0], tmp); strcpy(strmac , tmp);
    strcat(strmac, ":");
    hex2char(intmac[1],tmp); strcat(strmac , tmp);
    strcat(strmac, ":");
    hex2char(intmac[2],tmp); strcat(strmac , tmp);
    strcat(strmac, ":");
    hex2char(intmac[3],tmp); strcat(strmac , tmp);
    strcat(strmac, ":");
    hex2char(intmac[4],tmp); strcat(strmac , tmp);
    strcat(strmac, ":");
    hex2char(intmac[5],tmp); strcat(strmac , tmp);

}

int makeMAMSG(M4_Data *M_Data, char* pBuf){
    char destMAC[6]={0xff,0xff,0xff,0xff,0xff,0xff};
    char *p=pBuf;
    sprintf(p, "MA      \r\n");
    p+=2;
    if(M_Data->Index[1]!=0x00)
    {   
       str2hex_mac(M_Data->Data[0], (unsigned char *)destMAC);
    }
    memcpy(p, destMAC, sizeof(char)*6);
    return 10;
}

int check_F2MsgSize(M4_Data *M_Data)
{
    int DataSize = 0;
    for(int i=0; i<Command_List[(int)M_Data->Index[1]][0]; i++){
        if(M_Data->Data[i+1]!=NULL){
            DataSize+=(int)strlen(M_Data->Data[i+1]);
            DataSize+=4;
        }
        DataSize+=4;
    }
    return DataSize;    
}
int makeWIZ750SRMSG(M4_Data *M_Data, char* pBuf){
    char *cmd_str;
    int buf_cnt;
    buf_cnt=makeMAMSG(M_Data, pBuf);

    cmd_str = malloc(check_F2MsgSize(M_Data)+5);
    strcpy(cmd_str, "PW \r\n");
#if 1 //becky 200806
    for (int i = 0; i < (M_Data->DataCount -1); i++) {
#else
    for(int i=0; i<Command_List[(int)M_Data->Index[1]][0]; i++){
#endif
        if(M_Data->Data[i+1]!=NULL){
            strcat(cmd_str, eCmd[ Command_List[(int)M_Data->Index[1]][i+1] ]);
            strcat(cmd_str, M_Data->Data[i+1]);
		    strcat(cmd_str,"\r\n");
        }
    }
    for(int i=0; i<Command_List[(int)M_Data->Index[1]][0]; i++){
            strcat(cmd_str, eCmd[ Command_List[(int)M_Data->Index[1]][i+1] ]);
		    strcat(cmd_str,"\r\n");
    }
    printf("cmd str : %d\r\n", strlen(cmd_str));
    for(int i=0; i<strlen(cmd_str); i++){
		pBuf[buf_cnt++]=cmd_str[i];
	}
    free(cmd_str);

    for(int i=0; i<buf_cnt; i++)
    printf("%.2x ", pBuf[i]);

    printf("\r\n");
    return buf_cnt;

}
int parseWIZ750SRMSG(M4_Data* M_Data, char* recvBuffer)
{   
    char *p1,*p2;
    unsigned char TempMac[6];
    int cnt=0;
    
    p1=strstr(recvBuffer, "MA");
    memcpy(TempMac, p1+2, 6);
    M_Data->Data[cnt] = (char *)calloc(17, sizeof(char));
    hex2str_mac(TempMac, M_Data->Data[cnt]);
    printf("%d:%s\r\n", cnt, M_Data->Data[cnt]);
    p1=recvBuffer+15;
    
    while(1){
        cnt++;
        p2=strstr(p1, "\r\n");
        if(p2==0) break;
        *p2=0;
        M_Data->Data[cnt] = (char *)calloc(strlen(p1+2) + 1, sizeof(char));
        strcpy(M_Data->Data[cnt], p1+2);
        printf("%d:%s\r\n",cnt, M_Data->Data[cnt]);
        *(M_Data->Data[cnt] + strlen(p1 + 2)) = 0;
        p1=p2+2;
    }
    M_Data->DataCount=cnt;
    return cnt;
}

void Sample_R_MakeMessage(char *inData, char *outData)
{
    char TempMac[6] ={0x00, 0x08, 0xdc, 0x12, 0x34, 0x56};
    char *p = outData;
    sprintf(p,"MA      \r\nPW \r\n%s", inData);
    p += 2;
    memcpy(p, TempMac, sizeof(char)*6);
    printf("make message : %c%c [%02x] [%02x] [%02x] [%02x] [%02x] [%02x] %s \r\n", outData[0], outData[1], outData[2]&0xFF, outData[3]&0xFF, outData[4]&0xFF, outData[5]&0xFF, outData[6]&0xFF, outData[7]&0xFF, outData + 8);
}


#if 1 //becky 200803
int wiz750sr_recv(M4_Data* M_Data, int sn, unsigned int time_100ms)
#else
int wiz750sr_recv(M4_Data* M_Data, int sn, WIZ750SR_INFO* info, unsigned int time_100ms)
#endif
{
    static unsigned int pre_time = 0;
    static uint8_t recv_count = 0;
#if 1 //becky 2000804
    static uint8_t timer_count = 0;
#endif
    char RecvUDP_Buff[1024] = { 0, };
#if 0 //becky 200804
    uint8_t timer_count = 0;
#endif 
    uint8_t destMAC[6];
    int32_t RecvUDP_Len = 0;
    int32_t ret=0;
    //printf("wiz750sr_recv\r\n");
    
    #if 1 //becky 200803
    switch (info.status) {
    #else
    switch (info->status) {
    #endif
    case Ot_start:
        printf("ot start\r\n");
        pre_time = time_100ms;
        recv_count = 0;
        
        #if 1 //becky
        if (info.index == I_Search) timer_count = SEARCHTIME;
        else timer_count = SETTIME;
        info.status = Ot_busy;
        #else
        if (info->index == I_Search) timer_count = SEARCHTIME;
        else timer_count = SETTIME;
        info->status = Ot_busy;

        #endif
        break;

    case Ot_busy:
        if ((time_100ms - pre_time) > timer_count)
        {
            if (recv_count == 0)
            {
                M_Data[recv_count].Index[0] = WIZ750SRINDEX;
                M_Data[recv_count].Index[1] = I_noResponse;
                #if 0 //200806 becky
                ret = 0; //not receive
                #endif
            #if 1 //becky 200803
            }
            #if 1 //becky 200804
            info.status = Ot_stop;
            ret = recv_count;
            printf("%d:Timeout, 0x%.2x:Index\r\n", recv_count, info.index);
            #else
            else
                ret = recv_count;
            info.status = Ot_stop;
            printf("Ot_stop\r\n");
            #endif
            return ret;
            #else
                return ret;
            }
            else
                ret = recv_count;
            info->status = Ot_stop;
            #endif
        }
        if ((RecvUDP_Len = getSn_RX_RSR(sn)) > 0)
        {
            // recv data
            printf("Recv UDP\r\n");
            uint8_t destip[4]; //for debug
            uint16_t destport = 0; // for debug
            RecvUDP_Len = sock_recvfrom(sn, (uint8_t*)RecvUDP_Buff, (uint16_t)RecvUDP_Len, destip, (uint16_t*)&destport);
            if (RecvUDP_Len <= 0)
            {
                printf("%d: recvfrom error. %ld\r\n", sn, RecvUDP_Len);
            }
            else
            {
                RecvUDP_Buff[RecvUDP_Len] = 0;
                printf("sk:%d ip:%d.%d.%d.%d port:%d \r\n", sn, destip[0], destip[1], destip[2], destip[3], destport); // for debug
                printf("Recv : %c %c %02x %02x %02x %02x %02x %02x %s \r\n<Recv End>\r\n", RecvUDP_Buff[0], RecvUDP_Buff[1], RecvUDP_Buff[2]\
                    , RecvUDP_Buff[3], RecvUDP_Buff[4], RecvUDP_Buff[5], RecvUDP_Buff[6], RecvUDP_Buff[7], RecvUDP_Buff + 8); // for debug
                if (RecvUDP_Buff[0] == 'M' && RecvUDP_Buff[1] == 'A')
                {
                    #if 1 //becky 200803
                    if (info.index != I_Search) str2hex_mac(info.destMac, destMAC);
                    #if 1 // becky 200806
                    if ((info.index == I_Search) || memcmp(RecvUDP_Buff + 2, destMAC, 6)==0)
                    #else
                    if ((info.index == I_Search) || memcmp(RecvUDP_Buff+2, destMAC, 6) )
                    #endif
                    {
                        M_Data[recv_count].Index[0] = WIZ750SRINDEX;
                        M_Data[recv_count].Index[1] = info.index;
                        parseWIZ750SRMSG(&M_Data[recv_count++], RecvUDP_Buff);
                        if (info.index != I_Search) {
                            info.status = Ot_stop;
                            #if 1 //200806
                            printf("%d:Not Search, 0x%.2x:Index\r\n", recv_count, info.index);
                            ret = recv_count;
                            #else
                            ret = 1;
                            #endif
                            return ret;
                        }
                    }
                    #else
                    if (info->index != I_Search) str2hex_mac(info->destMac, destMAC);
                    if ((info->index == I_Search) || ( RecvUDP_Buff[2] == destMAC[0] && RecvUDP_Buff[3] == destMAC[1] &&
                        RecvUDP_Buff[4] == destMAC[2] && RecvUDP_Buff[5] == destMAC[3] && RecvUDP_Buff[6] == destMAC[4] && RecvUDP_Buff[7] == destMAC[5]))
                    {
                        M_Data[recv_count].Index[0] = WIZ750SRINDEX;
                        M_Data[recv_count].Index[1] = info->index;
                        parseWIZ750SRMSG(&M_Data[recv_count++], RecvUDP_Buff);
                        if (info->index != I_Search) {
                            info->status = Ot_stop;
                            ret = 1;
                            return ret;
                        }
                    }
                    #endif
                }
            }
        }
        break;

    }
    return ret;
}


/*
 * @brief write function
 * @param  n : pointer to a Network structure
 *         that contains the configuration information for the Network.
 *         buffer : pointer to a read buffer.
 * @retval length of data sent or SOCKERR code
 */
#if 1   //teddy 200805
int wiz750sr_send(int sn)
#else
int wiz750sr_send(int sn,str_qM4_Data* device)
#endif
{

    
    int ret,i = 0;
    M4_Data* put_data;
//	M4_Data* MessageDatafromA7;
    
    
    char* payload;
    int payload_len;
        //make 750SR message  
#if 1   //teddy 200805 
    cnt = m4_data_dequeue(&Wiz750SR, &put_data, 1);
#else
    cnt = m4_data_dequeue(device, &put_data, 1);
#endif
#if 1   //teddy 200805
    if (cnt <= 0)
        return cnt;
#else
    if(cnt > 0){
#endif
        printf("before payload index : [%02x][%02x] count : %d \r\n", put_data->Index[0], put_data->Index[1], put_data->DataCount);
          for (i=0; i<put_data->DataCount; i++)
        {
            printf("put_data.Data[%d] = %#x\r\n", i, put_data->Data[i]);
            if (put_data->Data[i] != NULL)
            {
                printf("put_data.Data[%d] = %s\r\n", i, put_data->Data[i]);
            }
        }
        payload = malloc(check_F2MsgSize(put_data) + 15); //MAPW Message Size is 15
        payload_len = makeWIZ750SRMSG(put_data, payload);
        printf("payload : %d %d \r\n", check_F2MsgSize(put_data) + 15, payload_len);
#if 1   //teddy 200805
        info.index = put_data->Index[1];
        if (put_data->Data[0] == NULL)
        {
            strcpy(info.destMac, "ff:ff:ff:ff:ff:ff");
        }
        else
        {
            strcpy(info.destMac, put_data->Data[0]);
        }
        FreeM4_Data(put_data);
        free(put_data);
#endif
        for(int i = 0;i<payload_len ;i++)
            printf("[%x] ",payload[i]);
        printf("\r\n");
#if 1   //teddy 200805
        printf("getSn_SR(%d) = %d\r\n", sn, getSn_SR(sn));
        ret = sock_sendto(sn, (uint8_t*)payload, payload_len, destip, destport);
        printf("sendto[%d]\r\n", ret);
#else
        printf("getSn_SR(%d) = %d\r\n",sn,getSn_SR(sn));
        if(getSn_SR(sn) == SOCK_UDP){
	    	ret = sock_sendto(sn, (uint8_t *)payload, payload_len,destip,destport);
        }
#endif
#if 1   //teddy 200805
        free(payload);
#endif
#if 0   //teddy 200805
    }
    else
    {
        return cnt;
    }
#endif
	return ret;
}

// 20210125 taylor
#if 1
int wiz750sr_unicast_search(uint8_t* destmac)
{
#define DBG_WIZ750SR_UNICAST_SEARCH

#ifdef DBG_WIZ750SR_UNICAST_SEARCH
    printf("Start DBG_WIZ750SR_UNICAST_SEARCH\r\n");
#endif
    
    uint8_t sn = UDPSocket;
    int ret;
    uint8_t unicast_search[] = {
            0x4d, 0x41,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd, 0xa,
            0x50, 0x57, 0x20, 0xd, 0xa,
            0x4d, 0x43, 0xd, 0xa,
            0x56, 0x52, 0xd, 0xa,
            0x4d, 0x4e, 0xd, 0xa,
            0x53, 0x54, 0xd, 0xa,
            0x49, 0x4d, 0xd, 0xa,
            0x4f, 0x50, 0xd, 0xa,
            0x4c, 0x49, 0xd, 0xa,
            0x53, 0x4d, 0xd, 0xa,
            0x47, 0x57, 0xd, 0xa
    };
    
    if (getSn_SR(sn) == SOCK_CLOSED)
    {
#ifdef DBG_WIZ750SR_UNICAST_SEARCH
        printf("SOCK_CLOSED\r\n");
        printf("End DBG_WIZ750SR_UNICAST_SEARCH\r\n");
#endif
        return -1;
    }

    if (info.status == Ot_idle)
    {
    
#ifdef DBG_WIZ750SR_UNICAST_SEARCH
        printf("getSn_SR(%d) = %d\r\n", sn, getSn_SR(sn));
#endif
        memcpy(&unicast_search[2], destmac, 6);
        ret = sock_sendto(sn, unicast_search, sizeof(unicast_search), destip, destport);
#ifdef DBG_WIZ750SR_UNICAST_SEARCH
        printf("sock_sendto ret = %d\r\n", ret);
#endif
        if (ret < 0)
        {
#ifdef DBG_WIZ750SR_UNICAST_SEARCH
            printf("error\r\n", ret);
            printf("End DBG_WIZ750SR_UNICAST_SEARCH\r\n");
#endif
            return ret;
        }
        info.status = Ot_start;
    }

#ifdef DBG_WIZ750SR_UNICAST_SEARCH
    printf("End DBG_WIZ750SR_UNICAST_SEARCH\r\n");
#endif

    return ret;
}
#endif

/*
 * @brief initial nfunction
 * @param  n : pointer to a Network structure
 *         that contains the configuration information for the Network.
 */
#if 1 //becky 200803
int wiz750sr_init(int sn_udp , int sn_tcp)
#else 
int wiz750sr_init(int sn)
#endif
{
    int ret = 0;
#if 1 //teddy 200805
    // for wiz750sr
    m4_data_queueinit(&Wiz750SR, Wiz750SR_buf, sizeof(Wiz750SR_buf) / sizeof(M4_Data*));
#endif
#if 1 //becky 200803
    memset(&info, 0, sizeof(info));
    UDPSocket = sn_udp;
    TCPSocket = sn_tcp;
    #else
    if(getSn_SR(sn)== SOCK_CLOSED){
    	ret = wiz_socket(sn,Sn_MR_UDP,myport++,0x00);
        printf("%d:Opened, UDP loopback, port [%d]\r\n", sn, myport);
        return ret;
    }
    #endif
        
}
#if 0   //teddy 200803
int F2_enqueue(M4_Data* M_Data,str_qM4_Data* device, M4_Data* m4_data_p)
{
    u8* temp;
    int i = 0;

    m4_data_p = malloc(sizeof(M4_Data));
    printf("malloc m4_data_p = %#x\r\n", m4_data_p);

    m4_data_p->Index[0] =  M_Data ->Index[0];
    m4_data_p->Index[1] = M_Data -> Index[1];
    m4_data_p->DataCount = M_Data -> DataCount;
    
    for (i=0; i<m4_data_p->DataCount; i++)
    {
        temp = malloc(strlen(M_Data->Data[i]+1));
        printf("malloc m4_data_p->Data[%d] = %#x\r\n", i, temp);
        m4_data_p->Data[i] = temp;
        memcpy(m4_data_p->Data[i], M_Data -> Data[i], strlen(M_Data-> Data[i]+1));
    }

    m4_data_enqueue(device, m4_data_p);
    send_parameter(device);
}
#endif

// 20210122 taylor
#if 1
/*
 * @brief F2 Process for WIZ750SR
 * @param Status : Mbox Send or Recv Status
 *      M_RecvData : 
 */
int F2_Process_750sr_enquee(char* Status, M4_Data* M_RecvData)
{
    // Mbox Data Recv 0x02 0000 0100
    if(*Status & 0x02)
    {
        if(M_RecvData->Index[0]==0x11 || M_RecvData->Index[0]==0x12)
        {
            *Status = *Status &0xFD;
            m4_data_enqueue(&Wiz750SR, M_RecvData);
            
            printf("2 status = %d\r\n",info.status);
        }
// 20210125 taylor
// print M4_Data
#if 0
    printf_M4_Data(M_RecvData);
#endif
    }
}
#endif

/*
 * @brief F2 Process for WIZ750SR
 * @param Status : Mbox Send or Recv Status
 *      M_SendData : 
 */
#if 1 //becky 200803
#if 1   //teddy 200805
int F2_Process_750sr(char* Status, M4_Data* M_SendData, M4_Data* M_RecvData, int time_100ms)
#else
int F2_Process_750sr(char* Status, str_qM4_Data* device, M4_Data* M_SendData, M4_Data* M_RecvData, int time_100ms)
#endif
#else
int F2_Process_750sr(char* Status,int sn,str_qM4_Data* device, M4_Data* M_SendData, M4_Data* M_RecvData,int time_100ms)
#endif
{
// 0x02 : Data Recv
// 0x04 : Data Send       
       // WIZ750SR_INFO *info;
        int ret,count= 0;
        int wiz750sr_st = 0;



        
#if 0   //teddy 200805
        if(( m4_data_queue_getlen(device) != 0)&&(info.status == Ot_idle))
        {
             info.status = Ot_init;
             printf("==============  1 status = %d  \r\n", info.status);
        }
#endif

// 20210122 taylor         
#if 0

        if(*Status & 0x02){ // Mbox Data Recv 0x02 0000 0100
#if 1
// 20201130 taylor
            if(M_RecvData->Index[0]==0x11 || M_RecvData->Index[0]==0x12){
                *Status = *Status &0xFD;
                m4_data_enqueue(&Wiz750SR, M_RecvData);
                
                printf("2 status = %d\r\n",info.status);
            }
#else
            if(M_RecvData->Index[0]==0x11){
                *Status = *Status &0xFD;
#if 1   //teddy 200805
                m4_data_enqueue(&Wiz750SR, M_RecvData);
#else
                m4_data_enqueue(device, M_RecvData);
#endif
                //send_parameter(device);
                
                printf("2 status = %d\r\n",info.status);
            }
#endif
        }
#endif

#if 1 //becky 200803    //teddy 200805 move position add return error 
        uint8_t sn = UDPSocket;
        if (getSn_SR(sn) == SOCK_CLOSED) {
            ret = wiz_socket(sn, Sn_MR_UDP, myport++, 0x00);
            printf("%d:Opened, UDP Config, port [%d]\r\n", sn, myport);
            if (ret < 0)
            {
                printf("UDP Socket Opne Error[%d]!! \r\n", ret);
                return ret;
            }
        }
#endif
        #if 1

        #if 1  //becky 200803
        count = wiz750sr_recv(M_SendData, sn, time_100ms);
        #else
        count = wiz750sr_recv(M_SendData,sn,&info,time_100ms);
        #endif
        // if(count< 0){
        //     printf("RECV ERROR = %d\r\n",count);
        //     return count;
        // }
        // else
        //     ret = count;
        #endif

#if 1       //teddy 200805
        if (info.status == Ot_stop)
        {
            if (count > 0) 
                *Status = 0x04;
            
            info.status = Ot_idle;
        }

        if (info.status == Ot_idle)
        {
#if 1   //teddy 200805
            if (m4_data_queue_getlen(&Wiz750SR) != 0)
#else
            if (m4_data_queue_getlen(device) != 0)
#endif
            {
#if 1   //teddy 200805
                ret = wiz750sr_send(sn);
#else
                ret = wiz750sr_send(sn, device);
#endif
                if (ret < 0) {
                    printf("SEND ERROR = %d\r\n", ret);
                    return ret;
                }
                info.status = Ot_start;
            }
        }
        return count;
#endif


#if 0       //teddy 200805 
        switch (info.status){
            case Ot_init:
                ret = wiz750sr_send(sn, device);
                if(ret< 0){
                    printf("SEND ERROR = %d\r\n",ret);
                    return ret;
                }
                else{
                    printf("M_RecvData->Data[0] = %s\r\n",M_RecvData->Data[0]);
                    if (strcmp(M_RecvData->Data[0], NULL) == 0) {
                        info.destMac =  "ff:ff:ff:ff:ff:ff";
                        printf("test1\r\n");
                    }
                    else{
                        //sprintf(info.destMac, M_RecvData->Data[0]);
                        //memcpy(info->destMac[0],M_RecvData->Data[0],strlen(M_RecvData->Data[0]+1));
                        info.destMac = M_RecvData->Data[0];
                        printf("test2\r\n");
                    }
                   printf("info.destMac = %s\r\n", info.destMac);
                   #if 1 //becky 200803
                   info.index = M_RecvData->Index[1];
                   #endif
                   info.status  = Ot_start;
                    //(*info).status  = 2;  
                }
            
            break;
            case Ot_stop:
                if(count > 0 ){
                    *Status = 0x04;
                }
                
                info.status= Ot_idle;
                //init_flag = 1;
                return count;
            

            break;
            default:
               
            break;

        }
        return ret;
#endif
        
}

#if 1 //becky 200805
int fwupdate_process(uint8_t *destmac, unsigned int time_100ms, uint8_t *fwdata, uint16_t fwsize)
{
    //firmware data, serach list mac
    static int status=0;
    static uint8_t tcp_destip[4];
    static uint16_t tcp_destport;
    static uint8_t RecvUDP_Buff[50];
    char *str_tcp_destport;
    char* temp_p;
    static uint16_t any_port = 50000;
    int ret;

    switch (status) {
    case 0:
        //open udp
        printf("\r\n****************fw updata start*****************\r\n");
        if (getSn_SR(UDPSocket) == SOCK_CLOSED) {
            ret = wiz_socket(UDPSocket, Sn_MR_UDP, myport++, 0x00);
            printf("%d:Opened, UDP Config, port [%d]\r\n", UDPSocket, myport);
        }
        else
        {
            printf("%d:Already Opened, UDP Config\r\n", UDPSocket);
        }
        status++;

        break;

    case 1:
        //send MA PW AB
        // wait recv
        if (sendMAPWAB(destmac, time_100ms) > 0) status++;
        break;

    case 2:
        //send MA PW FW
        //wait recv parsing data
        if (sendMAPWFW(destmac, time_100ms, fwsize, RecvUDP_Buff) > 0) {
            printf("recv data : %s\r\n", RecvUDP_Buff+17);
            temp_p = RecvUDP_Buff+17;
            str_tcp_destport = strstr(temp_p, ":");
            *str_tcp_destport = NULL;
            str_tcp_destport++;

            str2int_ip(temp_p, tcp_destip);
            temp_p = strstr(str_tcp_destport, "\r");
            *temp_p = NULL;
            tcp_destport = str2int(str_tcp_destport);

            close_socket(TCPSocket);
            if ((ret = wiz_socket(TCPSocket, Sn_MR_TCP, any_port++, 0x00)) != TCPSocket) {
                if (any_port == 0xffff) any_port = 50000;
            }
            
            status++;
        }
        break;

    case 3:
        //send firwmware data
        //tcp_destip[3] = 29;
        if ((ret = sendFWData(tcp_destip, tcp_destport, fwdata, fwsize)) > 0) {
            printf("firmware send \r\n");
            status = 0;
            return 1;
        }
        else if (ret == -1) status--;
        break;
    }
    return 0;
}

int sendMAPWAB(uint8_t* destMAC, unsigned int time_100ms)
{
    static int status=0;
    static uint8_t timer_count=0;
    static unsigned int pre_time=0;
    static uint8_t SendUDP_Buff[20] = { 0, };
    uint8_t RecvUDP_Buff[20] = {0,};
    uint16_t RecvUDP_Len=0, SendUDP_Len=0;
    static uint8_t ret=0;

    switch (status) {
    case 0:
        status ++;
        timer_count = 1000;
        break;

    case 1:
        sprintf(SendUDP_Buff, "MA      \r\nPW \r\nAB\r\n");
        SendUDP_Len = strlen(SendUDP_Buff);
        memcpy(SendUDP_Buff + 2, destMAC, sizeof(char) * 6);
        printf("Send : %s\r\n", SendUDP_Buff);
        ret=sock_sendto(UDPSocket, SendUDP_Buff, SendUDP_Len, broad_ip, upd_dest_port);
        if (ret > 0) {
            pre_time = time_100ms;
            status++;
        }
        else printf(" send error%d \r\n", ret);
        break;

    case 2:
        if ((time_100ms - pre_time) > timer_count) { printf("status --- ");  status--; }

        if ((RecvUDP_Len = getSn_RX_RSR(UDPSocket)) > 0)
        {
            // recv data
            RecvUDP_Len = sock_recvfrom(UDPSocket, (uint8_t*)RecvUDP_Buff, (uint16_t)RecvUDP_Len, 0,0);
            if (RecvUDP_Len <= 0)
            {
                printf("%d: recvfrom error. %ld\r\n", UDPSocket, RecvUDP_Len);
            }
            else
            {
                if (memcmp(SendUDP_Buff, RecvUDP_Buff, 15)==0)
                {
                    status = 0;
                    return 1;
                }
                else
                {
                    printf("Compare failed\r\n");
                    printf("Send : %s\r\n", SendUDP_Buff);
                    printf("Recevied : %s\r\n", RecvUDP_Buff);
                }
            }
        }
        break;
    }
    return 0;
}

int sendMAPWFW(uint8_t* destMAC, unsigned int time_100ms, int fwsize, char * RecvUDP_Buff)
{
    static int status = 0;
    static uint8_t timer_count;
    static unsigned int pre_time = 0;
    static uint8_t SendUDP_Buff[30];
    uint16_t RecvUDP_Len, SendUDP_Len;
    static uint8_t ret = 0;

    switch (status) {
    case 0:
        status++;
        timer_count = 30;
        break;
        
    case 1:
        sprintf(SendUDP_Buff, "MA      \r\nPW \r\nFW%d\r\n", fwsize);
        SendUDP_Len = strlen(SendUDP_Buff);
        memcpy(SendUDP_Buff + 2, destMAC, sizeof(char) * 6);
        printf("Send : %s\r\n", SendUDP_Buff);
        sock_sendto(UDPSocket, SendUDP_Buff, SendUDP_Len, broad_ip, upd_dest_port);
        pre_time = time_100ms;
        status++;

        break;
        
    case 2:
        if ((time_100ms - pre_time) > timer_count) { printf(" status--\r\n");  status--; }
        if ((RecvUDP_Len = getSn_RX_RSR(UDPSocket)) > 0)
        {
            // recv data
            RecvUDP_Len = sock_recvfrom(UDPSocket, (uint8_t*)RecvUDP_Buff, (uint16_t)RecvUDP_Len,0,0);
            for (int i = 0; i < RecvUDP_Len; i++) printf("%.2x ", RecvUDP_Buff[i]);
            printf("\r\n");
            if (RecvUDP_Len <= 0)
            {
                printf("%d: recvfrom error. %ld\r\n", UDPSocket, RecvUDP_Len);
            }
            else
            {
                if (memcmp(SendUDP_Buff, RecvUDP_Buff, 15)==0)
                {
                    status = 0;
                    //18~
                    printf("sendMA PW return 1\r\n");
                    return 1;
                }
                else
                {
                    printf("Compare failed\r\n");
                    printf("Send : %s\r\n", SendUDP_Buff);
                    printf("Recevied : %s\r\n", RecvUDP_Buff);
                }
            }
        }
        break;
    }
    return 0;
}

#endif

#if 1 //becky 200806
int sendFWData(uint8_t* tcp_destip, uint16_t tcp_destport, uint8_t* SendTCP_Buff, uint16_t SendTCP_Len)
{
    int ret;
    static uint16_t sentsize=0;
    static uint16_t last_sentsize;
    uint8_t RecvTCP_Buff[20];
    uint16_t RecvTCP_Len;
    switch (getSn_SR(TCPSocket)) {
    case SOCK_ESTABLISHED:
        if (getSn_IR(TCPSocket) & Sn_IR_CON)	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
        {
            if ((SendTCP_Len - sentsize) > FWSENDSIZE) last_sentsize = FWSENDSIZE;
            else last_sentsize = SendTCP_Len - sentsize;
            ret=sock_send(TCPSocket, SendTCP_Buff, last_sentsize);
            if (ret > 0) {
                last_sentsize = ret;
                sentsize += ret;
                setSn_IR(TCPSocket, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
            }
        }
        
        if ((RecvTCP_Len = getSn_RX_RSR(TCPSocket)) > 0)
        {
            // recv data
            RecvTCP_Len = sock_recv(TCPSocket, RecvTCP_Buff, RecvTCP_Len);
            if (((RecvTCP_Buff[0] << 8) | RecvTCP_Buff[1]) == last_sentsize)
            {
                printf("last_sentsize:%d\r\n", last_sentsize);
                if ((SendTCP_Len - sentsize) > FWSENDSIZE) last_sentsize = FWSENDSIZE;
                else last_sentsize = SendTCP_Len - sentsize;

                ret = sock_send(TCPSocket, SendTCP_Buff + sentsize, last_sentsize);
                if (ret > 0) {
                    last_sentsize = ret;
                    sentsize += ret;
                }
                else return ret;
                if (SendTCP_Len == sentsize) {
                    printf("sentsize : %d, last_sentsize : %d\r\n", sentsize, last_sentsize);
// 20210128 taylor
#if 1
                    int8_t ret_discon;
                    ret_discon = sock_disconnect(TCPSocket);
                    printf("sock_disconnect %d\r\n", ret_discon);
#else
                    close_socket(TCPSocket);
#endif
                    return 1;
                }
            }
        }
        break;
        
    case SOCK_INIT : 
        sentsize = 0;
        if ((ret = sock_connect(TCPSocket, tcp_destip, tcp_destport)) != SOCK_OK) {
            printf(" connect error : %d\r\n", ret);
            return ret;
        }
        break;

    case SOCK_CLOSED :
        return -1;
        break;
    }
    return 0;
}
unsigned int str2int(char* str)
{
    int dec = 0;
    for (int i = 0; i < strlen(str); i++) {
        dec = dec * 10 + str[i] - '0';
    }
    return dec;
}

void str2int_ip(char* strip, uint8_t* intip)
{

    char* p = strstr(strip, ".");
    *p = NULL;
    intip[0] = str2int(strip);

    strip = p + 1;
    p = strstr(strip, ".");
    *p = NULL;
    intip[1] = str2int(strip);
    strip = p + 1;

    p = strstr(strip, ".");
    *p = NULL;
    intip[2] = str2int(strip);
    strip = p + 1;

    intip[3] = str2int(strip);
}
#endif


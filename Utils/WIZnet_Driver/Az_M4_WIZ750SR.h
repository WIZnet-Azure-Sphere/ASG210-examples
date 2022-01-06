#ifndef __AZ_M4_WIZ750SR_H
#define __AZ_M4_WIZ750SR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ParseMboxData.h"
#include "AZ_M4_Alarm.h"
#include "Az_M4_Process.h"
#include "m4_data_queue.h"
#include "ioLibrary_Driver/Ethernet/socket.h"
#include "ioLibrary_Driver/Ethernet/wizchip_conf.h"
#include "ioLibrary_Driver/Ethernet/W5500/w5500.h"

//Define Command
// 20201204 taylor
#if 1
typedef enum {CMD_MC, CMD_VR, CMD_MN, CMD_IM, CMD_OP, CMD_DD, CMD_CP, CMD_PO, CMD_DG, CMD_KA,
              CMD_KI, CMD_KE, CMD_RI, CMD_LI, CMD_SM, CMD_GW, CMD_DS, CMD_PI, CMD_PP, CMD_DX,
              CMD_DP, CMD_DI, CMD_DW, CMD_DH, CMD_LP, CMD_RP, CMD_RH, CMD_BR, CMD_DB, CMD_PR, 
              CMD_SB, CMD_FL, CMD_IT, CMD_PT, CMD_PS, CMD_PD, CMD_TE, CMD_SS, CMD_NP, CMD_SP, 
              CMD_LG, CMD_ER, CMD_FW, CMD_MA, CMD_PW, CMD_SV, CMD_EX, CMD_RT, CMD_UN, CMD_ST, 
              CMD_FR, CMD_EC, CMD_K1, CMD_UE, CMD_GA, CMD_GB, CMD_GC, CMD_GD, CMD_CA, CMD_CB,
              CMD_CC, CMD_CD, CMD_SC, CMD_S0, CMD_S1, CMD_RX, CMD_FS, CMD_FC, CMD_FP, CMD_FD,
              CMD_FH, CMD_UI, CMD_AB, CMD_TR, CMD_BU,
              // For WIZ750SR Waton
              #if 1
              CMD_AL, CMD_AR, CMD_AP, CMD_TL, CMD_T0, CMD_T1, CMD_T2, CMD_T3, CMD_T4, CMD_P0,
              CMD_P1, CMD_P2, CMD_P3, CMD_P4,
              #endif
              CMD_UNKNOWN=255
}eCMDNUM;
#else
typedef enum {CMD_MC, CMD_VR, CMD_MN, CMD_IM, CMD_OP, CMD_DD, CMD_CP, CMD_PO, CMD_DG, CMD_KA,
              CMD_KI, CMD_KE, CMD_RI, CMD_LI, CMD_SM, CMD_GW, CMD_DS, CMD_PI, CMD_PP, CMD_DX,
              CMD_DP, CMD_DI, CMD_DW, CMD_DH, CMD_LP, CMD_RP, CMD_RH, CMD_BR, CMD_DB, CMD_PR, 
              CMD_SB, CMD_FL, CMD_IT, CMD_PT, CMD_PS, CMD_PD, CMD_TE, CMD_SS, CMD_NP, CMD_SP, 
              CMD_LG, CMD_ER, CMD_FW, CMD_MA, CMD_PW, CMD_SV, CMD_EX, CMD_RT, CMD_UN, CMD_ST, 
              CMD_FR, CMD_EC, CMD_K1, CMD_UE, CMD_GA, CMD_GB, CMD_GC, CMD_GD, CMD_CA, CMD_CB,
              CMD_CC, CMD_CD, CMD_SC, CMD_S0, CMD_S1, CMD_RX, CMD_FS, CMD_FC, CMD_FP, CMD_FD,
              CMD_FH, CMD_UI, CMD_AB, CMD_TR, CMD_BU, CMD_UNKNOWN=255
}eCMDNUM;
#endif

typedef enum{
    I_Search   = 0x00,
    I_getDev   = 0x01,
    I_getNet   = 0x02,
	I_setNet   = 0x03,	
    I_getUart  = 0x04,
	I_setUart  = 0x05,
	I_getSop   = 0x06,
	I_setSop   = 0x07,
    I_getOp   = 0x08,
    I_setOp   = 0x09,
    I_getSwitch = 0x10,
    I_setSwitch =0x11,
    I_getCtrl = 0x12,
    I_setCtrl = 0x13,
    I_noResponse = 0xff
}IndexTYPE;


typedef enum{
    Ot_idle     = 0,
    Ot_init     = 1,
    Ot_start    = 2, 
    Ot_busy     = 3,
    Ot_stop     = 4,
    Ot_upgrade  = 5
}OtStatus;


typedef struct WIZ750SR_INFO WIZ750SR_INFO;
struct WIZ750SR_INFO
{
    OtStatus status;
    int upgrade_seq;
    int index;
    char destMac[17];

};
char hex2char(unsigned char intmac, char *strmac);
char char2hex(char* data);
void str2hex_mac(char* strmac, unsigned char* intmac);
void hex2str_mac(unsigned char* intmac, char* strmac);
int check_F2MsgSize(M4_Data *M_Data);
int makeWIZ750SRMSG(M4_Data *M_Data, char* pBuf);
#if 1   //teddy
int parseWIZ750SRMSG(M4_Data* M_Data, char* recvBuffer);
#endif
void Sample_R_MakeMessage(char *inData, char *outData);
#if 1 //becky 200803
#if 1   //teddy 200805
int wiz750sr_send(int sn);
#else
int wiz750sr_send(int sn, str_qM4_Data* device);
#endif
int wiz750sr_recv(M4_Data* M_Data, int sn, unsigned int time_100ms);
int wiz750sr_init(int sn_udp, int sn_tcp);
#else
int wiz750sr_send(int sn,str_qM4_Data* device);
int wiz750sr_recv(M4_Data* M_Data, int sn, WIZ750SR_INFO *info, unsigned int time_100ms);
int wiz750sr_init(int sn);
#endif

// 20210122 taylor
#if 1
int F2_Process_750sr_enquee(char* Status, M4_Data* M_RecvData);
#endif

#if 0   //teddy 200803
int F2_enqueue(M4_Data* M_Data,str_qM4_Data* device, M4_Data* m4_data_p);
#endif
#if 1 //becky 200803
#if 1   //teddy 200805
int F2_Process_750sr(char* Status, M4_Data* M_SendData, M4_Data* M_RecvData, int time_100ms);
#else
int F2_Process_750sr(char* Status, str_qM4_Data* device, M4_Data* M_SendData, M4_Data* M_RecvData, int time_100ms);
#endif
# else
int F2_Process_750sr(char* Status, int sn,str_qM4_Data* device, M4_Data* M_SendData, M4_Data* M_RecvData,int time_100ms);
#endif

#if 1 //bekcy 200805
int fwupdate_process(uint8_t *destmac, unsigned int time_100ms, uint8_t *fwdata, uint16_t fwsize);
int sendFWData(uint8_t* tcp_destip, uint16_t tcp_destport, uint8_t* SendTCP_Buff, uint16_t SendTCP_Len);
int sendMAPWAB(uint8_t* destMAC, unsigned int time_100ms);
int sendMAPWFW(uint8_t* destMAC, unsigned int time_100ms, int fwsize, char * RecvUDP_Buff);
unsigned int str2int(char* str);
void str2int_ip(char* strip, uint8_t* intip);
#endif 

#ifdef __cplusplus
}
#endif
#endif
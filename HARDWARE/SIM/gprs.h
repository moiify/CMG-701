#ifndef _GPRS_H
#define _GPRS_H
#include "sdmmc_sdcard.h"
#include "ff.h"
#include "ftl.h"
#include "exfuns.h"

extern u8 Flag_GPRS_R;
//extern u8 GPRS_DATA[100];
extern u8 Server_ip[16];
extern u8 server_port[6];
extern u8 EquipID[5];
extern u8  Password[5];
extern u8 GprsSignalStrength;
extern u8 GprsSignalFlag;


void GPRS_EN_Init(void);
void GPRS_AT_Init(void);
void GPRS_Task(void);

#endif

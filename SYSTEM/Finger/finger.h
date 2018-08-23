#ifndef _FINGER_H
#define _FINGER_H
#include "sys.h"

typedef struct
{
	//u8 addr ;
	u8 	Id[5];
	u8  Name[4];
} people ;


void Add_FR(void);	//Â¼Ö¸ÎÆ
int FingerPrint_Task(void);
void PS_StaGPIO_Init(void);
extern char Text_Info[100];
extern u8 Press_FR_Flag;
extern int Add_FR_Flag;
extern int Del_FR_Flag;
extern char ID_Puts[5];
extern char Name_Puts[4];
#endif

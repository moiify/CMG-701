#ifndef _HOST485_H
#define _HOST485_H
#include "sys.h"
#include "string.h"  
#include "usart.h"
#include "includes.h"
#include "led.h"

typedef struct MODBUS_DataValue{  
float Steam_Mpa;   //ѹ��
float Temp_C;      //�¶�
float MachineSmoke_C;//��¯����
float LengSmoke_C;  //��������
int Eryang_ppm;      //��������
int Danyang_ppm;    //��������
float Oxygen_Null;  //��������ϵ��
float Rexiao;       //��Ч�� 
int Water_Null;    //ˮλ
unsigned char Machine_Status;  //��¯��ͣ
unsigned char Burn_Status;       //ȼ�ջ���ͣ
}Modbus_data;  

extern u8  Pa_Flag;
extern u8 Tem_Flag;
extern u8 Wat_Flag;
extern u8 Mac_Flag;
extern Modbus_data ModbusStrtues;

char Host485_Task(void);

#endif



#ifndef _HOST485_H
#define _HOST485_H
#include "sys.h"
#include "string.h"  
#include "usart.h"
#include "includes.h"
#include "led.h"

typedef struct MODBUS_DataValue{  
float Steam_Mpa;   //压力
float Temp_C;      //温度
float MachineSmoke_C;//锅炉排烟
float LengSmoke_C;  //冷凝排烟
int Eryang_ppm;      //二氧化硫
int Danyang_ppm;    //氮氧化物
float Oxygen_Null;  //过量空气系数
float Rexiao;       //热效率 
int Water_Null;    //水位
unsigned char Machine_Status;  //锅炉启停
unsigned char Burn_Status;       //燃烧机启停
}Modbus_data;  

extern u8  Pa_Flag;
extern u8 Tem_Flag;
extern u8 Wat_Flag;
extern u8 Mac_Flag;
extern Modbus_data ModbusStrtues;

char Host485_Task(void);

#endif



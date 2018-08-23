#ifndef _HOST485_H
#define _HOST485_H
#include "sys.h"
#include "string.h"  
#include "usart.h"
#include "includes.h"
#include "led.h"

typedef struct MODBUS_DataValue{  
float Steam_Mpa;
float MachineSmoke_C;
float LengSmoke_C;
int WarmWater_Mpa;
int WarmWater_C;
int Steam_C;
unsigned char Water_Null;
unsigned char Machine_Status;
unsigned char Burn_Status;
}Modbus_data; 

extern u8  Pa_Flag;
extern u8 Tem_Flag;
extern u8 Wat_Flag;
extern u8 Mac_Flag;
extern Modbus_data ModbusStrtues;

char Host485_Task(void);

#endif



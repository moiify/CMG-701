#include "Host485.h"
#include "stm32f7xx_hal.h"
#include "gprs.h"
#include "checkout.h"
#include "usart.h"

#define Enable 0
#define Disable 1
#define SlaveAddr 0x02
 


u8 Host485_send_data(u8 *data);
unsigned short ModbusCRC(unsigned char *ptr, unsigned char size);
unsigned char checkCRC(unsigned char *ptr, unsigned char size);
void MODBUSDataProcess(void);
u16 Resiger;
extern u8 EquipID[5];
unsigned char Host485Enable_Flag=0;
unsigned char Sendbuff[8]={0x02,0x03,0x00,0x00,0x00,0x1D,0x85,0xF0};
unsigned char Test_Data[]={0x42,0xc8,0x00,0x00};
float LenningqiYan_Temp;
u8 *p;
extern UART_HandleTypeDef husart6; 
extern u8 USART6_RX_BUF[USART6_MAX_RECV_LEN];
extern u16 USART6_RX_STA;
Modbus_data ModbusStrtues;
int len;
char Host485_Task()
{
	OS_ERR err;

	p=Sendbuff;
	while(1)
	{ 
		
		Sendbuff[0]=SlaveAddr;
		if(Host485Enable_Flag==Disable)
			return 1 ;
		else if(Host485Enable_Flag==Enable)
		{ 
			Host485_send_data(p); 
			//OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
			//memset(Salve_Data,0,sizeof(Salve_Data)); 
		}
		while((USART6_RX_STA|=1<<15)==0){OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);}		
    len=(USART6_RX_STA&(~(1<<15)));
		if(len>11&&USART6_RX_BUF[0]==SlaveAddr&&checkCRC((unsigned char *)USART6_RX_BUF,sizeof(USART6_RX_BUF)))     
      {     
            switch(USART6_RX_BUF[1])
        {   
					   
						case 0x03:
                MODBUSDataProcess();
                break;
            default:              
								break;
				}

      } 
			memset(USART6_RX_BUF,0,sizeof(USART6_RX_BUF)); 
			USART6_RX_STA=0;
			OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
			
	}
      return 0 ;
}


void MODBUSDataProcess()
{   
	  u8 Base_Address;
		u8 AddrInt;
//	  unsigned char TestFloatBuff[]={0x02,0x03,0x3A,
//		0x3F,0x1c,0x28,0xf5, 0x43,0x27,0x80,0x00, 0x42,0xF3,0x99,0x99, 0x00,0x03, 0x00,0x01, 0x00,0x01,
//		0x3F,0x17,0x0a,0x3d, 0x42,0xd3,0x80,0x00, 0x42,0x3d,0x33,0x33, 0x00,0x02, 0x00,0x01, 0x00,0x00,
//		0x3F,0x1c,0x28,0xf5, 0x42,0xb6,0xcc,0xcc, 0x42,0x40,0x66,0x66, 0x00,0x01, 0x00,0x00, 0x00,0x01};
		AddrInt=str2int((const char *)EquipID);
	  ModbusStrtues.Steam_Mpa=     GetFloat(&USART6_RX_BUF[(AddrInt-1)*2*9+3]);
		ModbusStrtues.MachineSmoke_C=GetFloat(&USART6_RX_BUF[(AddrInt-1)*2*9+7]);
		ModbusStrtues.LengSmoke_C=   GetFloat(&USART6_RX_BUF[(AddrInt-1)*2*9+11]);
		ModbusStrtues.Water_Null=    USART6_RX_BUF[(AddrInt-1)*2*9+16];
		ModbusStrtues.Machine_Status=    USART6_RX_BUF[(AddrInt-1)*2*9+18];
		ModbusStrtues.Burn_Status=    USART6_RX_BUF[(AddrInt-1)*2*9+20];
		ModbusStrtues.WarmWater_Mpa = -1;
		ModbusStrtues.WarmWater_C =-1;
		ModbusStrtues.Steam_C  =-1;
	  Resiger=USART6_RX_BUF[15]*256+USART6_RX_BUF[16];
//		Salve_Data[0]=Resiger/1000;
//	  Salve_Data[1]=Resiger/100%10;
//		Salve_Data[2]=Resiger/10%10;
//		Salve_Data[3]=Resiger%10;
//		Value.water=Salve_Data[0]*10+Salve_Data[1]+Salve_Data[2]/10+Salve_Data[3]/100;
printf("浓度值:%d \n",Resiger);
}
	



u8 Host485_send_data(u8 *data)
{
	OS_ERR err;
	char result;
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
	OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
	result=HAL_UART_Transmit(&husart6,data,8,0xffff);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
	return result;
}

unsigned short ModbusCRC(unsigned char *ptr, unsigned char size)
{
	unsigned short a, b, tmp, CRC16;
	CRC16 = 0xffff;
	for (a = 0; a < size; a++)
	{
		CRC16 = *ptr^CRC16;
		for (b = 0; b < 8; b++)
		{
			tmp = CRC16 & 0x0001;
			CRC16 = CRC16 >> 1;
			if (tmp)
				CRC16 = CRC16 ^ 0xa001;
		}
		*ptr++;
	}
	return (((CRC16 & 0x00FF) << 8) | ((CRC16 & 0xFF00) >> 8));
}


unsigned char checkCRC(unsigned char *ptr, unsigned char size)
{
	signed short tmp = 0;
	tmp = ModbusCRC(ptr, size - 2);
	if ((*(ptr + size - 1) == (tmp & 0x00ff)) && (*(ptr + size - 2) == (tmp >> 8)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}





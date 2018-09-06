#include "Host485.h"
#include "stm32f7xx_hal.h"
#include "gprs.h"
#include "checkout.h"
#include "usart.h"

#define Enable 0
#define Disable 1
#define GuoluAddr 0x02
#define AnlogAddr 0X01
 


u8 Host485_send_data(u8 *data);
unsigned short ModbusCRC(unsigned char *ptr, unsigned char size);
unsigned char checkCRC(unsigned char *ptr, unsigned char size);
void MODBUSDataProcess1(void);
void MODBUSDataProcess2(void);

extern RS_Cache RS485_Cache;
extern u8 EquipID[5];
unsigned char Host485Enable_Flag=0;
unsigned char GuoluSendbuff[8]={0x02,0x03,0x00,0x00,0x00,0x1D,0x85,0xF0};
unsigned char AnlogSendbuff[8]={0x01,0x03,0x00,0x03,0x00,0x03,0xF5,0xCB};
u8 AddrInt;
u8 *p;
extern UART_HandleTypeDef husart6; 
extern u8 USART6_RX_BUF[USART6_MAX_RECV_LEN];
extern u16 USART6_RX_STA;
Modbus_data ModbusStrtues;

char Host485_Task()
{
    OS_ERR err;
    ModbusRevBuf_t *p;
    while(1)
    { 
    //  printf("modbus \r\n");
      IWDG_Feed();
      USART6->CR1|=1<<2;
      USART6->CR1|=1<<5;
      p=&RS485_Cache.Buf[RS485_Cache.Out];
      printf("Out:%d,Len:%d\r\n",RS485_Cache.Out,RS485_Cache.Buf[RS485_Cache.Out].Len);
	  if(RS485_Cache.Count>0&&RS485_Cache.Buf[RS485_Cache.Out].Len>58)
      {  
         if(p->Buf[0]==2&&p->Buf[1]==3&&p->Buf[2]==0x3a&&checkCRC((unsigned char *)(p->Buf),63))
         {
              switch(p->Buf[0])
                {   
                    case GuoluAddr:
                        
                    MODBUSDataProcess1();
                    break;
                    
                    case AnlogAddr:
                    MODBUSDataProcess2();
                    
                    break;
                    default:              
					break;
				}
         }        
         else if(checkCRC((unsigned char *)(&(p->Buf[8])),(p->Len-8)))     
           {   
               for(uint8_t i=0;i < p->Len-8; i++)
               {
                    p->Buf[i] = p->Buf[8+i];
               }
          
                switch(p->Buf[0])
                {   
                    case GuoluAddr:
                        
                    MODBUSDataProcess1();
                    break;
                    
                    case AnlogAddr:
                    MODBUSDataProcess2();
                    
                    break;
                    default:              
					break;
				}
           }
       }
         else if(RS485_Cache.Count>0&&RS485_Cache.Buf[RS485_Cache.Out].Len>8)
      {  
         
         if(checkCRC((unsigned char *)(p->Buf),(p->Len)))     
           {   
                switch(p->Buf[0])
                {   
                    case GuoluAddr:
                        
                    MODBUSDataProcess1();
                    break;
                    
                    case AnlogAddr:
                    MODBUSDataProcess2();
                    
                    break;
                    default:              
					break;
				}
                OSTimeDlyHMSM(0,0,3,0,OS_OPT_TIME_PERIODIC,&err);
           }
                
       } 
      if(RS485_Cache.Count>0)
      {
           RS485_Cache.Out++;
           RS485_Cache.Out%= sizeof(RS485_Cache.Buf)/sizeof(RS485_Cache.Buf[0]);
           CPU_SR      cpu_sr;
           CPU_CRITICAL_ENTER();
           RS485_Cache.Count--;
           CPU_CRITICAL_EXIT() ;   
           OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err);
      }
      else OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
          
   }
   return 0 ;
}

void GuoLuModbus()
{
   
	
}

void MODBUSDataProcess1()
{   
//	  unsigned char TestFloatBuff[]={0x02,0x03,0x3A,
//		0x3F,0x1c,0x28,0xf5, 0x43,0x27,0x80,0x00, 0x42,0xF3,0x99,0x99, 0x00,0x03, 0x00,0x01, 0x00,0x01,
//		0x3F,0x17,0x0a,0x3d, 0x42,0xd3,0x80,0x00, 0x42,0x3d,0x33,0x33, 0x00,0x02, 0x00,0x01, 0x00,0x00,
//		0x3F,0x1c,0x28,0xf5, 0x42,0xb6,0xcc,0xcc, 0x42,0x40,0x66,0x66, 0x00,0x01, 0x00,0x00, 0x00,0x01};	  
//		u8 AddrInt;

		AddrInt=str2int((const char *)EquipID);
        if(AddrInt==3)
        AddrInt=1;
        else if (AddrInt==1)
        AddrInt=3;
        ModbusStrtues.Steam_Mpa=     GetFloat(&RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2*9+3]);
		ModbusStrtues.MachineSmoke_C=GetFloat(&RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2*9+7]);
		ModbusStrtues.LengSmoke_C=   GetFloat(&RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2*9+11]);
		ModbusStrtues.Water_Null=    RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2*9+15]*256+RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2*9+16];
		ModbusStrtues.Machine_Status=    RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2*9+18];
		ModbusStrtues.Burn_Status=    RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2*9+20];
		ModbusStrtues.Temp_C  =-1;
	  ModbusStrtues.Danyang_ppm  =-1;
		ModbusStrtues.Eryang_ppm  =-1;
		ModbusStrtues.Rexiao  =-1;
//	  Resiger=RS485_Cache.Buf[RS485_Cache.Out].Buf[15]*256+RS485_Cache.Buf[RS485_Cache.Out].Buf[16];
}
void MODBUSDataProcess2()
{   
	  float AnlogDataN;
	  float OxygenN;
		float A,B,C;
	
	  AddrInt=str2int((const char *)EquipID);
//		AnlogDataN= (((RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2+3]*256+RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2+4])/10000)*16+4);
	  A=(RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2+3])*256;
	  B=RS485_Cache.Buf[RS485_Cache.Out].Buf[(AddrInt-1)*2+4];
		C=((A+B)/10000)*16+4;
	  AnlogDataN=C;
	//  AnlogDataN=5.11;  //Test
	  OxygenN = 0.218+(19.5/16)*(AnlogDataN-4);
	  if(OxygenN>20)
		ModbusStrtues.Oxygen_Null=18;
		else
	  ModbusStrtues.Oxygen_Null=(20.9/(20.9-OxygenN));
	
}



u8 Host485_send_data(u8 *data)
{
	OS_ERR err;
	char result;
	OSIntEnter();
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
	result=HAL_UART_Transmit(&husart6,data,8,0xffff);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);	 
	OSIntExit(); 
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





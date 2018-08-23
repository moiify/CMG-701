#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "w25qxx.h"
#include "nand.h"  
#include "24cxx.h"
#include "mpu.h"
#include "sdmmc_sdcard.h"
#include "malloc.h"
#include "ff.h"
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"
#include "touch.h"
#include "includes.h"
#include "AS608.h"
#include "finger.h"
#include "checkout.h"
#include <stdlib.h>
#include "gprs.h"

#define usart2_baund  57600//串口2波特率，根据指纹模块波特率更改

SysPara AS608Para;//指纹模块AS608参数
people People_Info[30];

u16 ValidN;//模块内有效指纹个数
extern u16 USART3_RX_STA;
extern u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				    //接收缓冲,最大USART2_MAX_RECV_LEN个字节.

u8 Press_FR_Flag=0;
int Add_FR_Flag=0;
int Del_FR_Flag=0;
char Text_Info[100];
char ID_Puts[5];
char Name_Puts[4];
extern u8 Server_ip[16];
extern u8 EquipID[5];
extern u8 GPRS_DATA[100];
void Add_FR(void);	//录指纹
void Del_FR(void);	//删除指纹
void press_FR(void);//刷指纹
void ShowErrMessage(u8 ensure);//显示确认码错误信息



void PS_StaGPIO_Init(void)    //检测引脚初始化
{   
  GPIO_InitTypeDef  GPIO_InitStructure;
	__HAL_RCC_GPIOH_CLK_ENABLE();

  GPIO_InitStructure.Pin = GPIO_PIN_8;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT; //输入模式
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;	 //下拉模式
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;//高速模式
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure); //初始化GPIO	
}

int FingerPrint_Task(void)    //指纹识别任务
{ 
	OS_ERR err;
	u8 ensure;
	char *str;
	int m;
	OSTaskSemPend(0,OS_OPT_PEND_BLOCKING,0,&err);		 	//请求内建的信号量
	PS_StaGPIO_Init();
	
	while(PS_HandShake(&AS608Addr))//检测AS608
	{
		OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_PERIODIC,&err);
		printf("未检测到模块!!!");
		OSTimeDlyHMSM(0,0,0,800,OS_OPT_TIME_PERIODIC,&err);
		printf("尝试连接模块...");		  
	}
	sprintf(Text_Info,"欢迎使用");
	str=mymalloc(SRAMIN,30);
	printf("\r\n %s ",(u8*)str);
	
	ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		ShowErrMessage(ensure);//显示确认码错误信息		
	ensure=PS_ReadSysPara(&AS608Para);  							//读参数 
	if(ensure==0x00)
	{
		mymemset(str,0,50);
		sprintf(str,"库容量:%d     对比等级: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
		printf("\r\n%s ",(u8*)str);
	}
	ShowErrMessage(ensure);
	myfree(SRAMIN,str);
	
	for(m=0;m<25;m++)
	{ 
		AT24CXX_Read(m*8,People_Info[m].Id,4);
		AT24CXX_Read(m*8+4,People_Info[m].Name,3);
		OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);							//延时5ms
	//	printf("ID:%s Name%s \r\n",People_Info[m].Id,People_Info[m].Name);
	}	
	
	while(1)
	{ 
    sprintf(Text_Info,"欢迎使用");
		
		if(Del_FR_Flag)
			Del_FR();		//删指纹
				
		if(PS_Sta&&Press_FR_Flag)
			press_FR();//刷指纹	

	 if(Add_FR_Flag)
		{
			Add_FR();		//录指纹
		}	
	OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err); 
}
}
//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
	printf("\r\n %s",(u8*)EnsureMessage(ensure));
	

}
//录指纹
void Add_FR(void)
{ 
	OS_ERR err;
	u8 ensure ,processnum=0;
	u8 i=0 ;
	u8 m=0;
	char Null_ID[7]="\0\0\0\0";
	static u16 Make_Num;
	Add_FR_Flag=0;
	PS_ValidTempleteNum(&ValidN);
//	ID_Num=ValidN;
//  for(m=0;m<50;m++)
//	{ 
//		AT24CXX_Read(m*4,People_Info[m].Id,4);
//		OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);							//延时5ms
//		printf("ID%d:%s \r\n",m,People_Info[m].Id);
//	}
	while(1)
	{ 
		switch (processnum)
		{
			case 0:
				i++;
				printf("\r\n请按指纹");
			  sprintf(Text_Info,"请在指纹区按下手指");
			  ensure=PS_GetImage();
				OSTimeDlyHMSM(0,0,0,1000,OS_OPT_TIME_PERIODIC,&err);
				if(ensure==0x00) 
				{
					//PCF8574_WriteBit(BEEP_IO,0);//打开蜂鸣器
					ensure=PS_GenChar(CharBuffer1);//生成特征
					//PCF8574_WriteBit(BEEP_IO,1);//关闭蜂鸣器
					if(ensure==0x00)
					{
						printf("\r\n指纹正常");
						i=0;
						processnum=1;//跳到第二步						
					}else ShowErrMessage(ensure);				
				} else  ShowErrMessage(ensure);					
				
			break;
			
			case 1:
				i++;
				printf("\r\n 请按再按一次指纹");
			  sprintf(Text_Info,"请按再按一次手指");
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
				//	PCF8574_WriteBit(BEEP_IO,0);//打开蜂鸣器
					ensure=PS_GenChar(CharBuffer2);//生成特征
				//	PCF8574_WriteBit(BEEP_IO,1);//关闭蜂鸣器
					if(ensure==0x00)
					{
						printf("\r\n 指纹正常");
						i=0;
						processnum=2;//跳到第三步
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				break;

			case 2:
				printf("\r\n 对比两次指纹");
				ensure=PS_Match();
				if(ensure==0x00) 
				{

					printf("\r\n 对比成功,两次指纹一样");
					processnum=3;//跳到第四步
				}
				else 
				{
					printf("\r\n 对比失败，请重新录入指纹");
					ShowErrMessage(ensure);
					sprintf(Text_Info,"Contrast failed,Please make Again");
					i=0;
					processnum=0;//跳回第一步		
				}
				OSTimeDlyHMSM(0,0,0,1200,OS_OPT_TIME_PERIODIC,&err);
				break;

			case 3:
				printf("\r\n生成指纹模板");
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					printf("\r\n生成指纹模板成功");
					processnum=4;//跳到第五步
				}else {processnum=0;ShowErrMessage(ensure);}
				OSTimeDlyHMSM(0,0,0,1200,OS_OPT_TIME_PERIODIC,&err);
				break;
				
			case 4:	
				do{ 
					 for(m=0;m<25;m++)
					{
						if(!(strcmp((char*)People_Info[m].Id,Null_ID))) 
						{ 
							
							Make_Num=m;
							AT24CXX_Write(m*8,(u8 *)ID_Puts,4);
							OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
							printf("ID:%s \r\n",ID_Puts);
							AT24CXX_Write(m*8+4,(u8 *)Name_Puts,3);
							sprintf((char *)People_Info[m].Id,"%s",ID_Puts);
							sprintf((char *)People_Info[m].Name,"%s",Name_Puts);
						  printf("MakeNum:%d\r\n",Make_Num);
							break;
						}else printf("-%d-Found no space",m);
				   }
			    }while(!(Make_Num<AS608Para.PS_max));//输入ID必须小于指纹容量的最大值
				ensure=PS_StoreChar(CharBuffer2,Make_Num);//储存模板
				if(ensure==0x00) 
				{	

					printf("\r\n录入指纹成功");
					sprintf(Text_Info,"录入指纹成功,您的ID:%d",Make_Num);
					OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_PERIODIC,&err);
//					PS_ValidTempleteNum(&ValidN);//读库指纹个数
//					printf("\r\n %d",AS608Para.PS_max-ValidN);
					OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}					
				break;				
		}	
		OSTimeDlyHMSM(0,0,0,600,OS_OPT_TIME_PERIODIC,&err);
		if(i==5)//超过5次没有按手指则退出
		{
		printf("\r\n 超时 取消指纹录制");
		sprintf(Text_Info,"超时 取消指纹录制");
		Add_FR_Flag=0;
		break;	
		}			
	}
}
//刷指纹
void press_FR(void)
{ 
	OS_ERR err;
	SearchResult seach;
	u8 ensure;
	char *str;
	char end1[1]={0x1A};					  //发送结尾标志
	char FingerRep[100];	
	
	ensure=PS_GetImage();
	//sprintf(Text_Info,"Ready");
	if(ensure==0x00)//获取图像成功 
	{	 
		 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//打开蜂鸣器			
		 ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//搜索成功
			{					
//				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);	//PB1置0
//				OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET); //关闭蜂鸣器
				printf("\r\n刷指纹成功");				
				str=mymalloc(SRAMIN,100);
				//sprintf(str,"确有此人,ID:%d  匹配得分:%d",seach.pageID,seach.mathscore);
				//sprintf("\r\n%s",(u8*)str);
				printf("Id:%s\r\n",People_Info[seach.pageID].Id);
				sprintf(str,"%s",People_Info[seach.pageID].Name);
				sprintf(Text_Info,"%s 您已成功打卡!",str);   
				myfree(SRAMIN,str);
				sprintf(FingerRep,"{\"FingerID\":\"%s\",\"NAME\":\"%s\",\"EQUIP\":\"%s\",\"Type\":1}\r\n",People_Info[seach.pageID].Id,People_Info[seach.pageID].Name,EquipID);
				GPRS_Sendcom((u8 *)"AT+CIPSEND\r\n");
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);				
				GPRS_Sendcom((u8 *)FingerRep);
				GPRS_Sendcom((u8 *)end1);
				GPRS_Sendcom((u8 *)"\r\n");	
				printf("打卡信息上传成功!\r\n");
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
		    GPRS_Sendcom((u8 *)"AT+CSQ\r\n");				
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
				GprsSignalStrength=(GPRS_DATA[8]-48)*10+(GPRS_DATA[9]-48);
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
			}
			else
			{ 
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
		//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//打开蜂鸣器
		//		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);
				sprintf(Text_Info," 请重按手指 ");				
				OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);		
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//打开蜂鸣器 				
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);				
				
			}
				ShowErrMessage(ensure);
					
	  }
		else
			ShowErrMessage(ensure);	
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);	
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);	
			OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);

	}
		
}

//删除指纹
void Del_FR(void)
{ 
	OS_ERR err;
	u8  ensure;
	u8 m;
	u8 Null_char[8]="\0\0\0\0\0\0\0\0";
	ensure=PS_Empty();//清空指纹库  百度百科
	for(m=0;m<25;m++)
{ 
	AT24CXX_Write(m*8,Null_char,8);
	OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);							//延时5ms
}

//	else 
//		ensure=PS_DeletChar(num,1);//删除单个指纹  
	if(ensure==0)
	{
		printf("\r\n删除指纹成功");	
	  sprintf(Text_Info,"删除成功 ");	
	}
  else
	ShowErrMessage(ensure);	
	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
	PS_ValidTempleteNum(&ValidN);//读库指纹个数
	printf("\r\n%d",AS608Para.PS_max-ValidN);
}


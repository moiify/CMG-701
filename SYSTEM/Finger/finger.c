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

#define usart2_baund  57600//����2�����ʣ�����ָ��ģ�鲨���ʸ���

SysPara AS608Para;//ָ��ģ��AS608����
people People_Info[30];

u16 ValidN;//ģ������Чָ�Ƹ���
extern u16 USART3_RX_STA;
extern u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				    //���ջ���,���USART2_MAX_RECV_LEN���ֽ�.

u8 Press_FR_Flag=0;
int Add_FR_Flag=0;
int Del_FR_Flag=0;
char Text_Info[100];
char ID_Puts[5];
char Name_Puts[4];
extern u8 Server_ip[16];
extern u8 EquipID[5];
extern u8 GPRS_DATA[100];
void Add_FR(void);	//¼ָ��
void Del_FR(void);	//ɾ��ָ��
void press_FR(void);//ˢָ��
void ShowErrMessage(u8 ensure);//��ʾȷ���������Ϣ



void PS_StaGPIO_Init(void)    //������ų�ʼ��
{   
  GPIO_InitTypeDef  GPIO_InitStructure;
	__HAL_RCC_GPIOH_CLK_ENABLE();

  GPIO_InitStructure.Pin = GPIO_PIN_8;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT; //����ģʽ
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;	 //����ģʽ
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;//����ģʽ
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure); //��ʼ��GPIO	
}

int FingerPrint_Task(void)    //ָ��ʶ������
{ 
	OS_ERR err;
	u8 ensure;
	char *str;
	int m;
	OSTaskSemPend(0,OS_OPT_PEND_BLOCKING,0,&err);		 	//�����ڽ����ź���
	PS_StaGPIO_Init();
	
	while(PS_HandShake(&AS608Addr))//���AS608
	{
		OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_PERIODIC,&err);
		printf("δ��⵽ģ��!!!");
		OSTimeDlyHMSM(0,0,0,800,OS_OPT_TIME_PERIODIC,&err);
		printf("��������ģ��...");		  
	}
	sprintf(Text_Info,"��ӭʹ��");
	str=mymalloc(SRAMIN,30);
	printf("\r\n %s ",(u8*)str);
	
	ensure=PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	if(ensure!=0x00)
		ShowErrMessage(ensure);//��ʾȷ���������Ϣ		
	ensure=PS_ReadSysPara(&AS608Para);  							//������ 
	if(ensure==0x00)
	{
		mymemset(str,0,50);
		sprintf(str,"������:%d     �Աȵȼ�: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
		printf("\r\n%s ",(u8*)str);
	}
	ShowErrMessage(ensure);
	myfree(SRAMIN,str);
	
	for(m=0;m<25;m++)
	{ 
		AT24CXX_Read(m*8,People_Info[m].Id,4);
		AT24CXX_Read(m*8+4,People_Info[m].Name,3);
		OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);							//��ʱ5ms
	//	printf("ID:%s Name%s \r\n",People_Info[m].Id,People_Info[m].Name);
	}	
	
	while(1)
	{ 
    sprintf(Text_Info,"��ӭʹ��");
		
		if(Del_FR_Flag)
			Del_FR();		//ɾָ��
				
		if(PS_Sta&&Press_FR_Flag)
			press_FR();//ˢָ��	

	 if(Add_FR_Flag)
		{
			Add_FR();		//¼ָ��
		}	
	OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err); 
}
}
//��ʾȷ���������Ϣ
void ShowErrMessage(u8 ensure)
{
	printf("\r\n %s",(u8*)EnsureMessage(ensure));
	

}
//¼ָ��
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
//		OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);							//��ʱ5ms
//		printf("ID%d:%s \r\n",m,People_Info[m].Id);
//	}
	while(1)
	{ 
		switch (processnum)
		{
			case 0:
				i++;
				printf("\r\n�밴ָ��");
			  sprintf(Text_Info,"����ָ����������ָ");
			  ensure=PS_GetImage();
				OSTimeDlyHMSM(0,0,0,1000,OS_OPT_TIME_PERIODIC,&err);
				if(ensure==0x00) 
				{
					//PCF8574_WriteBit(BEEP_IO,0);//�򿪷�����
					ensure=PS_GenChar(CharBuffer1);//��������
					//PCF8574_WriteBit(BEEP_IO,1);//�رշ�����
					if(ensure==0x00)
					{
						printf("\r\nָ������");
						i=0;
						processnum=1;//�����ڶ���						
					}else ShowErrMessage(ensure);				
				} else  ShowErrMessage(ensure);					
				
			break;
			
			case 1:
				i++;
				printf("\r\n �밴�ٰ�һ��ָ��");
			  sprintf(Text_Info,"�밴�ٰ�һ����ָ");
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
				//	PCF8574_WriteBit(BEEP_IO,0);//�򿪷�����
					ensure=PS_GenChar(CharBuffer2);//��������
				//	PCF8574_WriteBit(BEEP_IO,1);//�رշ�����
					if(ensure==0x00)
					{
						printf("\r\n ָ������");
						i=0;
						processnum=2;//����������
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				break;

			case 2:
				printf("\r\n �Ա�����ָ��");
				ensure=PS_Match();
				if(ensure==0x00) 
				{

					printf("\r\n �Աȳɹ�,����ָ��һ��");
					processnum=3;//�������Ĳ�
				}
				else 
				{
					printf("\r\n �Ա�ʧ�ܣ�������¼��ָ��");
					ShowErrMessage(ensure);
					sprintf(Text_Info,"Contrast failed,Please make Again");
					i=0;
					processnum=0;//���ص�һ��		
				}
				OSTimeDlyHMSM(0,0,0,1200,OS_OPT_TIME_PERIODIC,&err);
				break;

			case 3:
				printf("\r\n����ָ��ģ��");
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					printf("\r\n����ָ��ģ��ɹ�");
					processnum=4;//�������岽
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
			    }while(!(Make_Num<AS608Para.PS_max));//����ID����С��ָ�����������ֵ
				ensure=PS_StoreChar(CharBuffer2,Make_Num);//����ģ��
				if(ensure==0x00) 
				{	

					printf("\r\n¼��ָ�Ƴɹ�");
					sprintf(Text_Info,"¼��ָ�Ƴɹ�,����ID:%d",Make_Num);
					OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_PERIODIC,&err);
//					PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
//					printf("\r\n %d",AS608Para.PS_max-ValidN);
					OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}					
				break;				
		}	
		OSTimeDlyHMSM(0,0,0,600,OS_OPT_TIME_PERIODIC,&err);
		if(i==5)//����5��û�а���ָ���˳�
		{
		printf("\r\n ��ʱ ȡ��ָ��¼��");
		sprintf(Text_Info,"��ʱ ȡ��ָ��¼��");
		Add_FR_Flag=0;
		break;	
		}			
	}
}
//ˢָ��
void press_FR(void)
{ 
	OS_ERR err;
	SearchResult seach;
	u8 ensure;
	char *str;
	char end1[1]={0x1A};					  //���ͽ�β��־
	char FingerRep[100];	
	
	ensure=PS_GetImage();
	//sprintf(Text_Info,"Ready");
	if(ensure==0x00)//��ȡͼ��ɹ� 
	{	 
		 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//�򿪷�����			
		 ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //���������ɹ�
		{		
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//�����ɹ�
			{					
//				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);	//PB1��0
//				OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET); //�رշ�����
				printf("\r\nˢָ�Ƴɹ�");				
				str=mymalloc(SRAMIN,100);
				//sprintf(str,"ȷ�д���,ID:%d  ƥ��÷�:%d",seach.pageID,seach.mathscore);
				//sprintf("\r\n%s",(u8*)str);
				printf("Id:%s\r\n",People_Info[seach.pageID].Id);
				sprintf(str,"%s",People_Info[seach.pageID].Name);
				sprintf(Text_Info,"%s ���ѳɹ���!",str);   
				myfree(SRAMIN,str);
				sprintf(FingerRep,"{\"FingerID\":\"%s\",\"NAME\":\"%s\",\"EQUIP\":\"%s\",\"Type\":1}\r\n",People_Info[seach.pageID].Id,People_Info[seach.pageID].Name,EquipID);
				GPRS_Sendcom((u8 *)"AT+CIPSEND\r\n");
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);				
				GPRS_Sendcom((u8 *)FingerRep);
				GPRS_Sendcom((u8 *)end1);
				GPRS_Sendcom((u8 *)"\r\n");	
				printf("����Ϣ�ϴ��ɹ�!\r\n");
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
		    GPRS_Sendcom((u8 *)"AT+CSQ\r\n");				
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
				GprsSignalStrength=(GPRS_DATA[8]-48)*10+(GPRS_DATA[9]-48);
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
			}
			else
			{ 
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
		//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//�򿪷�����
		//		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);
				sprintf(Text_Info," ���ذ���ָ ");				
				OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);		
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);	//�򿪷����� 				
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

//ɾ��ָ��
void Del_FR(void)
{ 
	OS_ERR err;
	u8  ensure;
	u8 m;
	u8 Null_char[8]="\0\0\0\0\0\0\0\0";
	ensure=PS_Empty();//���ָ�ƿ�  �ٶȰٿ�
	for(m=0;m<25;m++)
{ 
	AT24CXX_Write(m*8,Null_char,8);
	OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);							//��ʱ5ms
}

//	else 
//		ensure=PS_DeletChar(num,1);//ɾ������ָ��  
	if(ensure==0)
	{
		printf("\r\nɾ��ָ�Ƴɹ�");	
	  sprintf(Text_Info,"ɾ���ɹ� ");	
	}
  else
	ShowErrMessage(ensure);	
	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
	PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	printf("\r\n%d",AS608Para.PS_max-ValidN);
}


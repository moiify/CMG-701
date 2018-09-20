#include "gprs.h"
#include "rtc.h"
#include "includes.h"	
#include "sys.h"
#include "led.h"
#include "usart.h"
#include "Host485.h"
#include "24cxx.h"
#include "iwdg.h"
	
RTC_TimeTypeDef RTC_TimeStruct;	//时间句柄
RTC_DateTypeDef RTC_DateStruct;	//日期句柄

u8 Server_ip[16];				//--------------------IP 116.62.102.100
u8 server_port[6];			//--------------------Port  14000
u8 EquipID[5];					//--------------------设备号
u8 Password[5];					//--------------------密码


u8 tbuf[24];									  //时间变量
char end1[1]={0x1A};					  //发送结尾标志
u8 count=0;										  //任务计次
u8 Flag_GPRS_R=0;
u8 Flag_CONNECTSERVER;
u8 GprsSignalStrength=0;
u8 GprsSignalFlag=0;
FIL * FilPtr, FileSystemDst;
u8 GPRS_DATA[100];
u8 Clock[40];
u8 GPRSSEND[50];
extern Modbus_data ModbusStrtues;

void GPRS_EN_Init(void)
{		
	
//		OS_ERR err;
//		OSTimeDlyHMSM(0,0,500,0,OS_OPT_TIME_PERIODIC,&err);	//延时1s
//		printf("Connecting server\r\n");
//		GPRS_en(0);
//		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_PERIODIC,&err);	
//	  printf("En Finish\r\n");
}
//void GPRS_Sendcom( *data)
//{
//HAL_UART_Transmit(&husart6,data,8,0xffff);
//}
void GPRS_AT_Init(void)
{   
		OS_ERR err;	
/*                检测模块是否工作            */
AT:		do
		{
			printf("En Finish\r\n");	 
			memset(GPRS_DATA,0,sizeof(GPRS_DATA));            
			GPRS_Sendcom((u8 *)"AT\r\n");  
            OSTimeDlyHMSM(0,0,3,0,OS_OPT_TIME_PERIODIC,&err);
            count++;
            if(count > 25)
            {
               SCB->AIRCR =0X05FA0000|(u32)0x04;  //系统软复位 
            }
			while(Flag_GPRS_R==0)
			{
				count++; 
				OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_PERIODIC,&err);
				if(count%5==0)
				{
					GPRS_Sendcom((u8 *)"AT\r\n"); 
				}
				if(count > 25)
				{
					count =0;
					printf("GPRS module is not detected\r\n");
					OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);
					goto AT;
				}	
		 }
			if(strstr((const char *)GPRS_DATA,"OK")!=NULL)
			{
					printf("AT: %s \r\n",GPRS_DATA);					
					count=0;
					Flag_GPRS_R=0;
					memset(GPRS_DATA,0,sizeof(GPRS_DATA));
					OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
				  
			}
			else
			{
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
				goto AT;
				//OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
 			}			
		}while(Flag_GPRS_R);

		
/*                查询信号质量             */
CSQ:		do
			{
			printf("AT+CSQ\r\n");
			memset(GPRS_DATA,0,sizeof(GPRS_DATA));
			GPRS_Sendcom((u8 *)"AT+CSQ\r\n");  
			OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
			while(Flag_GPRS_R==0)
			{ 
				count++;
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
				if(count%5==0)
				{
					memset(GPRS_DATA,0,sizeof(GPRS_DATA));
					GPRS_Sendcom((u8 *)"AT+CSQ\r\n");
				}
				if(count>30)
				{
					count =0;
					goto AT;
				}
			}
			if(strstr((const char *)GPRS_DATA,"OK")!=NULL)
			{ 		
			      printf("AT+CSQ : %s\r\n",GPRS_DATA);
		        GprsSignalStrength=(GPRS_DATA[8]-48)*10+(GPRS_DATA[9]-48);	
						count =0;		
						Flag_GPRS_R=0;
						memset(GPRS_DATA,0,sizeof(GPRS_DATA));
						OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_PERIODIC,&err);
			}
			else
			{ 
				printf("CSQ ERROR\r\n");
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
			}
			}while(Flag_GPRS_R);

///*                网络注册信息             */
//			do
//		{ 
//			printf("AT+CPIN?\r\n");
//			GPRS_Sendcom((u8 *)"AT+CPIN?\r\n"); 
//			OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);		
//			while(Flag_GPRS_R==0)
//			{
//				count++;
//				OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_PERIODIC,&err);	
//				if(count%5==0)
//				{
//					memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//					GPRS_Sendcom((u8 *)"AT+CPIN?\r\n");
//				}
//				if(count>25)
//				{
//					count =0;
//					goto AT;
//				}
//			}
//			
//			if(strstr((const char *)GPRS_DATA,"READY")!=NULL)
//			{   
//				  printf("AT+CPIN?: %s\r\n",GPRS_DATA);	
//					Flag_GPRS_R=0;
//					count =0;				
//					memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//					
//			}
//			else
//			{
//				Flag_GPRS_R=0;
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
//			}
//		}while(Flag_GPRS_R);

/*                附着GPRS业务             */
	do
		{   
            IWDG_Feed();
			printf("AT+CGATT\r\n");
			GPRS_Sendcom((u8 *)"AT+CGATT=1\r\n"); 
			OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err);
			while(Flag_GPRS_R==0)
			{
				count++;
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
				OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_PERIODIC,&err);
				if(count%5==0)
				{
					memset(GPRS_DATA,0,sizeof(GPRS_DATA));
					GPRS_Sendcom((u8 *)"AT+CGATT=1\r\n");
				}
				if(count>25)
				{
					count =0;
					goto AT;
				}
			}
	 	if(strstr((const char *)GPRS_DATA,"OK")!=NULL)
			{ 
				
				printf("AT+CGATT=1:%s\r\n",GPRS_DATA);
				count =0;
				Flag_GPRS_R=0;
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
			}
			else
			{
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
				OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_PERIODIC,&err);
				goto CSQ;
			}
		}while(Flag_GPRS_R);

//	/*                启动任务             */
//		do
//		{ 
//			memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//			GPRS_Sendcom((u8 *)"AT+CSTT=1\r\n");
//			OSTimeDlyHMSM(0,0,8,0,OS_OPT_TIME_PERIODIC,&err);
//			while(Flag_GPRS_R==0)
//			{
//				count++;
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_PERIODIC,&err);
//				if(count%5==0)
//				{
//					memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//					GPRS_Sendcom((u8 *)"AT+CSTT=1\r\n");
//				}
//				if(count>25)
//				{
//					count =0;
//				}
//			}
//		if(strstr((const char *)GPRS_DATA,"OK")!=NULL)
//			{
//				Flag_GPRS_R=0;
//				count =0;
//				printf("AT+CSTT=1 : %s\r\n",GPRS_DATA);
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);	
//			}
//			else
//			{ 
//				
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
//				goto AT;
//			}
//		}while(Flag_GPRS_R);
//		
///*                发起GPRS连接             */
//		do
//		{
//			GPRS_Sendcom((u8 *)"AT+CIICR\r\n"); 
//			OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
//			while(Flag_GPRS_R==0)
//			{
//				count++;
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_PERIODIC,&err);
//				if(count%5==0)
//				{
//					memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//					GPRS_Sendcom((u8 *)"AT+CIICR\r\n");
//				}
//				if(count>25)
//				{
//					count =0;
//					goto AT;
//				}			
//			}
//			if(strstr((const char *)GPRS_DATA,"OK")!=NULL)
//			{
//				count=0;
//				Flag_GPRS_R=0;	
//				printf("AT+CIICR : %s\r\n",GPRS_DATA);
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
//			}
//			else
//			{
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
//			}
//		}while(Flag_GPRS_R);
//		/*                获取本地IP地址             */
//		do
//		{
//			GPRS_Sendcom((u8 *)"AT+CIFSR\r\n");
//			OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);//延时500ms
//			while(Flag_GPRS_R==0)
//			{
//				count++;
//				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);//延时500ms
//				if(count>30)
//				{
//					count =0;
//					goto AT;
//				}
//			}

//		  	Flag_GPRS_R=0;
//			  OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
//				printf("AT+CIFSR : %s\r\n",GPRS_DATA);		
//				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//				 OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
//		} while(Flag_GPRS_R);

//	

/*                建立连接             */
		do
		{ 
			printf("Server_ip %s\r\n",Server_ip);	
			AT24CXX_Read(232,Server_ip,16);
			OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err);
			OSIntEnter();
			GPRS_Sendcom((u8 *)"AT+CIPSTART=\"TCP\",\"");
			GPRS_Sendcom(Server_ip);
			GPRS_Sendcom((u8 *)"\",\"");
			GPRS_Sendcom(server_port);
			GPRS_Sendcom((u8 *)"\"\r\n");
			OSIntExit(); 
			OSTimeDlyHMSM(0,0,3,0,OS_OPT_TIME_PERIODIC,&err);
			while(Flag_GPRS_R==0)
			{
				count++;
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
				if(count>20)
				{
					count = 0;
					goto AT;
				}
			}
			if(strstr((const char *)GPRS_DATA,"CONNECT OK")!=NULL)
			{
				Flag_GPRS_R=0;
                count =0;
				printf("CONNECTSERVER_OK : %s\r\n",GPRS_DATA);
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
			}
			else if(strstr((const char *)GPRS_DATA,"ALREADY")!=NULL)
			{ 
				printf("CONNECTSERVER : %s\r\n",GPRS_DATA);
				Flag_GPRS_R=0;
                count =0;
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
			}
			else
			{   
                GprsSignalFlag=2;
				printf("CONNECTSERVER : %s\r\n",GPRS_DATA);
				memset(GPRS_DATA,0,sizeof(GPRS_DATA));
				OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
				goto CSQ;
			}
		}while(Flag_GPRS_R);
		
		Flag_CONNECTSERVER=1;

}

	

void GPRS_Task(void)
	{ 
    
//		RTC_TimeTypeDef RTC_TimeStruct;	//时间句柄
//		RTC_DateTypeDef RTC_DateStruct;	//日期句柄
		OS_ERR err;
		unsigned int bw;
		u8 res,i;
		char sendsbuf[120];	
		char a[] = "0:Log/1234-56-78.txt";
		char Date[10];
//		char Clock_Flag[3];
		char clockcheck[]="{\"YL\":-1,\"WD\":-1,\"SW\":-1,\"GLPY\":-1,\"LNQPY\":-1,\"GLQD\":-1,\"RSJQD\":-1,\"DYHW\":-1,\"EYHL\":-1,\"KQXS\":-1,\"RXL\":-1,\"EQUIP\":\"-1\",\"Clock\":0,\"Type\":0}\r\n";
		static u8 His_Date;
        //OSTaskSemPend(0,OS_OPT_PEND_BLOCKING,0,&err);		 	//请求内建的信号量
	
		//GPRS_EN_Init();
		OSTimeDlyHMSM(0,0,3,0,OS_OPT_TIME_PERIODIC,&err);
		GPRS_AT_Init();
		GPRS_Sendcom((u8 *)"ATE0\r\n");				//关闭回显
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
		memset(GPRS_DATA,0,sizeof(GPRS_DATA));
		GPRS_Sendcom((u8 *)"AT+CIPSEND\r\n");
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
		GPRS_Sendcom((u8 *)clockcheck);
		GPRS_Sendcom((u8 *)end1);
		GPRS_Sendcom((u8 *)"\r\n");	
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_PERIODIC,&err);
		sprintf((char*)Clock,"%s",GPRS_DATA);
		printf("Clock1:%s\r\n",Clock);
		if(strstr((const char *)Clock,"OK")==NULL)
		{
             GprsSignalFlag=2;
             memset(GPRS_DATA,0,sizeof(GPRS_DATA));
             GPRS_Task();
		}
        else GprsSignalFlag=1;
		memset(GPRS_DATA,0,sizeof(GPRS_DATA));
        RTC_DateStruct.Year = ((Clock[18]-48)*10 + (Clock[19]-48));
		RTC_DateStruct.Month = ((Clock[20]-48)*10 + (Clock[21]-48));
		RTC_DateStruct.Date = ((Clock[22]-48)*10 + (Clock[23]-48));
		RTC_Set_Date(RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, 4);		
		RTC_TimeStruct.Hours = ((Clock[24]-48)*10 + (Clock[25]-48));
		RTC_TimeStruct.Minutes = ((Clock[26]-48)*10 + (Clock[27]-48));
		RTC_TimeStruct.Seconds = ((Clock[28]-48)*10 + (Clock[29]-48));
		HAL_RTC_SetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);
        His_Date=RTC_DateStruct.Date;
	  while(1)
		{
			GPRS_Sendcom((u8 *)"AT+CIPSTART=\"TCP\",\"");
			AT24CXX_Read(232,Server_ip,16);
			//printf("Server_ip %s\r\n",Server_ip);
			GPRS_Sendcom(Server_ip);
			GPRS_Sendcom((u8 *)"\",\"");
			GPRS_Sendcom(server_port);
			GPRS_Sendcom((u8 *)"\"\r\n");
			OSTimeDlyHMSM(0,0,3,0,OS_OPT_TIME_PERIODIC,&err);
//		RTC_DateStruct.Year = ((GPRS_DATA[2]-48)*10 + (GPRS_DATA[3]-48));
//		RTC_DateStruct.Month = ((GPRS_DATA[4]-48)*10 + (GPRS_DATA[5]-48));
//		RTC_DateStruct.Date = ((GPRS_DATA[6]-48)*10 + (GPRS_DATA[7]-48));
//		RTC_Set_Date(RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, 4);		
//		RTC_TimeStruct.Hours = ((GPRS_DATA[8]-48)*10 + (GPRS_DATA[9]-48))-1;
//		RTC_TimeStruct.Minutes = ((GPRS_DATA[10]-48)*10 + (GPRS_DATA[11]-48));
//		RTC_TimeStruct.Seconds = ((GPRS_DATA[12]-48)*10 + (GPRS_DATA[13]-48));
//		HAL_RTC_SetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);
//		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
		FilPtr = &FileSystemDst;
			
//		HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);  	//获得系统时间
//		temp=sprintf((char*)tbuf,"Time:%02d%02d%02d",RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds); 		
//		HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);   //获得系统日期
//		sprintf((char*)tbuf+temp,"20%02d%02d%02d%s",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date,EquipID); 
		sprintf((char *)Date,"20%02d-%02d-%02d.txt",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date); 
//		memset(GPRS_DATA,0,sizeof(GPRS_DATA));
//		GPRS_Sendcom((u8 *)"AT+CIPSEND\r\n");  //发送数据
//		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
//		GPRS_Sendcom(tbuf);
//		GPRS_Sendcom((u8 *)end1);
//		GPRS_Sendcom((u8 *)"\r\n");	
  
		if(Flag_CONNECTSERVER==1)       //判断是否连接服务器
{
        printf("Send to server...\r\n");
        memset(GPRS_DATA,0,sizeof(GPRS_DATA));			
//						sprintf(&sendsbuf[0],"Data:%02d:%02d:%02d",RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds); 
//						sprintf(sendsbuf+13,"20%02d-%02d-%02d",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date);						
//						sprintf(sendsbuf+23,":%send", EquipID);
        sprintf(sendsbuf,"{\"YL\":%.2f,\"WD\":%f,\"SW\":%d,\"GLPY\":%.2f,\"LNQPY\":%.2f,\"GLQD\":%d,\"RSJQD\":%d,\"DYHW\":%d,\"EYHL\":%d,\"KQXS\":%f,\"RXL\":%f,\"EQUIP\":\"%s\",\"Clock\":1,\"Type\":0}\r\n",
                                                                                                                                                                    ModbusStrtues.Steam_Mpa,
                                                                                                                                                                    ModbusStrtues.Temp_C,
                                                                                                                                                                    ModbusStrtues.Water_Null,
                                                                                                                                                                    ModbusStrtues.MachineSmoke_C,
                                                                                                                                                                    ModbusStrtues.LengSmoke_C,
                                                                                                                                                                    ModbusStrtues.Machine_Status,
                                                                                                                                                                  ModbusStrtues.Burn_Status,
                                                                                                                                                                  ModbusStrtues.Danyang_ppm,                                               
                                                                                                                                                                  ModbusStrtues.Eryang_ppm,
                                                                                                                                                                    ModbusStrtues.Oxygen_Null,                                               
                                                                                                                                                                  ModbusStrtues.Rexiao,
                                                                                                                                                                    EquipID			);
        printf("GPRS:%s\r\n",sendsbuf);
        memset(GPRS_DATA,0,sizeof(GPRS_DATA));
        GPRS_Sendcom((u8 *)"AT+CIPSEND\r\n");
        OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err)
        ;
        GPRS_Sendcom((u8 *)sendsbuf);
        GPRS_Sendcom((u8 *)end1);
        GPRS_Sendcom((u8 *)"\r\n");	
        u16 waittime=0;
        while(strstr((const char *)GPRS_DATA,"OK")==NULL)
        {   
            waittime++;                           
            if(waittime==25)
            {   
                printf("发送失败");
                GprsSignalFlag=2;
                GPRS_Sendcom((u8 *)end1);
                GPRS_Sendcom((u8 *)"\r\n");	
                memset(GPRS_DATA,0,sizeof(GPRS_DATA));
                GPRS_Task();
            }
            OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
                
        }						
        printf("Send to server success!\r\n"); 
        
            
						
}					
    for(i=0;i<sizeof(Date);i++)
    {
        a[6+i] = Date[i];
    }   
      USART3->CR1&=~(1<<2);
      USART3->CR1&=~(1<<5);
      USART6->CR1&=~(1<<2);
      USART6->CR1&=~(1<<5);
      OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
      res = f_open(FilPtr, (const TCHAR*)a, FA_OPEN_ALWAYS | FA_WRITE);
      OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
        if( res == FR_OK )
        {
            
            f_lseek(FilPtr,f_size(FilPtr));
            res = f_write(FilPtr, sendsbuf,sizeof(sendsbuf),&bw);
            OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);
            bw=0;
            if(res)
            {
                printf("File  Write ERROR! \r\n");
            }
            else
            {   
                printf("File Write SUCCESS! \r\n");
                f_close(FilPtr);
                FilPtr=NULL;
                USART3->CR1|=1<<2;
                USART3->CR1|=1<<5;
                USART6->CR1|=1<<2;
                USART6->CR1|=1<<5;
                printf("File close SUCCESS! \r\n");
                OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);
            }
                
            
        }
        else if(res == FR_EXIST)
        {
            printf("File is already exist \r\n");
        }
        else
        {
            printf("Don't know the error! \r\n");
        }
        USART3->CR1|=1<<2;
        USART3->CR1|=1<<5;
        USART6->CR1|=1<<2;  
        USART6->CR1|=1<<5;
        memset(GPRS_DATA,0,sizeof(GPRS_DATA));
        GPRS_Sendcom((u8 *)"AT+CSQ\r\n");				
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);
        GprsSignalStrength=(GPRS_DATA[8]-48)*10+(GPRS_DATA[9]-48);
        memset(GPRS_DATA,0,sizeof(GPRS_DATA));
        u8 Now_Date;
        HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);
        Now_Date=RTC_DateStruct.Date;
        if(Now_Date!=His_Date)
        {
            SCB->AIRCR =0X05FA0000|(u32)0x04;  //系统软复位
        }
        OSTimeDlyHMSM(0,1,0,0,OS_OPT_TIME_PERIODIC,&err);
	}
}

#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "sdram.h"
#include "tftlcd.h"
#include "ltdc.h"
#include "mpu.h"
#include "usmart.h"
#include "malloc.h"
#include "touch.h"
#include "sdmmc_sdcard.h"
#include "ff.h"
#include "ftl.h" 
#include "exfuns.h"
#include "w25qxx.h"
#include "fontupd.h"
#include "GUI.h"
#include "WM.h"
#include "EmWinHZFont.h"
#include "includes.h"
#include "DIALOG.h"
#include "Hzfontupdata.h"
#include "rtc.h"
#include "24cxx.h"
#include "task.h"
#include "checkout.h"
#include "iwdg.h"
#include "Host485.h"
#include "AS608.h"
#include "finger.h"
#include "gprs.h"
#include "iwdg.h"
#include "dma.h"
/*
*********************************************************************
*                     静态全局变量
*********************************************************************
*/ 



OS_TMR 	MESH_tmr;				//--------------------定时器MESH_tmr
OS_TMR 	MESH_OVER_tmr;	//--------------------定时器MESH超时定时器


extern u8 server_port[6];
extern u8 EquipID[5];
extern u8  Password[5];
extern u8 Server_ip[16];

/*
*********************************************************************
*                            start任务 
*********************************************************************
*/ 
/*         任务优先级          */
#define START_TASK_PRIO				3
/*        任务堆栈大小         */	
#define START_STK_SIZE 				512
/*         任务控制块          */	
OS_TCB StartTaskTCB;
/*         任务堆栈	           */
CPU_STK START_TASK_STK[START_STK_SIZE];

/*
*********************************************************************
*                            触摸任务
*********************************************************************
*/ 
/*         任务优先级          */
#define TOUCH_TASK_PRIO				4
/*        任务堆栈大小         */	
#define TOUCH_STK_SIZE				512
/*         任务控制块          */	
OS_TCB TouchTaskTCB;
/*         任务堆栈	           */
CPU_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];
 
/*
*********************************************************************
*                          暂无任务
*********************************************************************
*/
/*         任务优先级          */
#define LED0_TASK_PRIO 				11
/*        任务堆栈大小         */	
#define LED0_STK_SIZE					512
/*         任务控制块          */
OS_TCB Led0TaskTCB;
/*         任务堆栈	           */
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];
 
/*
*********************************************************************
*                           读取存储任务
*********************************************************************
*/
/*         任务优先级          */
#define Init_TASK_PRIO			5
/*        任务堆栈大小         */	
#define Init_STK_SIZE			512
/*         任务控制块          */
OS_TCB InitTaskTCB;
/*         任务堆栈	           */
CPU_STK Init_TASK_STK[Init_STK_SIZE];
 
/*
*********************************************************************
*                           GUI任务
*********************************************************************
*/
/*         任务优先级          */
#define EMWINDEMO_TASK_PRIO			7
/*        任务堆栈大小         */	
#define EMWINDEMO_STK_SIZE			512
/*         任务控制块          */
OS_TCB EmwindemoTaskTCB;
/*         任务堆栈	           */
CPU_STK EMWINDEMO_TASK_STK[EMWINDEMO_STK_SIZE];
 
/*
*********************************************************************
*                           连接服务器任务
*********************************************************************
*/
/*         任务优先级          */
#define ConnectServer_TASK_PRIO			9
/*        任务堆栈大小         */	
#define ConnectServer_STK_SIZE			512
/*         任务控制块          */
OS_TCB ConnectServerTaskTCB;
/*         任务堆栈	           */
CPU_STK ConnectServer_TASK_STK[ConnectServer_STK_SIZE];
 
/*
*********************************************************************
*                           指纹识别任务
*********************************************************************
*/
/*         任务优先级          */
#define Slavetalk_TASK_PRIO			6
/*        任务堆栈大小         */	
#define Slavetalk_STK_SIZE			512
/*         任务控制块          */
OS_TCB SlavetalkTaskTCB;
/*         任务堆栈	           */
CPU_STK Slavetalk_TASK_STK[Slavetalk_STK_SIZE];

/*
*********************************************************************
*                            看门狗任务
*********************************************************************
*/
/*         任务优先级          */
#define Systemcheckstack_TASK_PRIO			10
/*        任务堆栈大小         */	
#define Systemcheckstack_STK_SIZE			512
/*         任务控制块          */
OS_TCB SystemcheckstackTaskTCB;   
/*         任务堆栈	           */
CPU_STK Systemcheckstack_TASK_STK[Systemcheckstack_STK_SIZE];

/*
*********************************************************************
*                           RS485任务
*********************************************************************
*/
/*         任务优先级          */
#define WIFI_TASK_PRIO			8
/*        任务堆栈大小         */	
#define WIFI_STK_SIZE			512
/*         任务控制块          */
OS_TCB WIFITaskTCB;
/*         任务堆栈	           */
CPU_STK WIFI_TASK_STK[WIFI_STK_SIZE];

/*
************************************************************************************
*                                      函数声明
************************************************************************************
*/
void start_task(void *p_arg);                           //开始任务函数
void touch_task(void *p_arg);														//触屏任务函数
void led0_task(void *p_arg);														//LED任务函数
void Init_task(void *p_arg);                            //读取存储任务函数
void emwindemo_task(void *p_arg);                       //屏幕任务函数
void IWATCH_Task(void *p_arg);                   //连接服务器任务函数
void Finger_Task(void *p_arg);                       //主从机交互任务函数
void Systemcheckstack_task(void *p_arg);                //堆栈扫描任务函数
void ModBus_Task(void *p_arg);                            //WIFI任务函数
//void MESH_tmr_callback(void *p_tmr, void *p_arg); 			//定时器MESH_tmr回调函数
void MESH_OVER_tmr_callback(void *p_tmr, void *p_arg); 	//定时器MESH超时回调函数

/*
******************************************************************************
*	函 数 名: start_task
*	功能说明: 创建应用任务 			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：3
******************************************************************************
*/
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	CPU_Init();
	
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  					 //---------------统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN							 //---------------如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  						 //---------------当使用时间片轮转的时候
	
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  //---------------使能时间片轮转调度功能,设置默认的时间片长度
#endif		
	__HAL_RCC_CRC_CLK_ENABLE();								 //---------------使能CRC时钟
	WM_SetCreateFlags(WM_CF_MEMDEV);					 //---------------开启STemWin存储设备
	GUI_Init();  															 //---------------STemWin初始化
	
	while(AT24CXX_Check())										 //---------------检测存储单元
	{
		printf("check At24cxx error\r\n");
		OSTimeDlyHMSM(0,0,30,0,OS_OPT_TIME_PERIODIC,&err);//------延时30s
	}
	printf("check at24cxx ok\r\n");
	
	WM_MULTIBUF_Enable(1);  									 //---------------开启STemWin多缓冲,RGB屏可能会用到	
		
	OS_CRITICAL_ENTER();											 //---------------进入临界区	

	/*
	*************************************************************
	*                        STemWin Demo任务
	*************************************************************
	*/								
	OSTaskCreate((OS_TCB*     )&EmwindemoTaskTCB,		
				 (CPU_CHAR*   )"Emwindemo task", 		
                 (OS_TASK_PTR )emwindemo_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )EMWINDEMO_TASK_PRIO,     
                 (CPU_STK*    )&EMWINDEMO_TASK_STK[0],	
                 (CPU_STK_SIZE)EMWINDEMO_STK_SIZE/10,	
                 (CPU_STK_SIZE)EMWINDEMO_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);

	/*
	*************************************************************
	*                        触摸屏任务
	*************************************************************
	*/
	OSTaskCreate((OS_TCB*     )&TouchTaskTCB,		
				 (CPU_CHAR*   )"Touch task", 		
                 (OS_TASK_PTR )touch_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )TOUCH_TASK_PRIO,     
                 (CPU_STK*    )&TOUCH_TASK_STK[0],	
                 (CPU_STK_SIZE)TOUCH_STK_SIZE/10,	
                 (CPU_STK_SIZE)TOUCH_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);			 

	/*
	*************************************************************
	*                        LED0任务
	*************************************************************
	*/
	OSTaskCreate((OS_TCB*     )&Led0TaskTCB,		
				 (CPU_CHAR*   )"Led0 task", 		
                 (OS_TASK_PTR )led0_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )LED0_TASK_PRIO,     
                 (CPU_STK*    )&LED0_TASK_STK[0],	
                 (CPU_STK_SIZE)LED0_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED0_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);	   

	/*
	*************************************************************
	*                        读取存储任务
	*************************************************************
	*/
	OSTaskCreate((OS_TCB*     )&InitTaskTCB,		
				 (CPU_CHAR*   )"Init task", 		
                 (OS_TASK_PTR )Init_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )Init_TASK_PRIO,     
                 (CPU_STK*    )&Init_TASK_STK[0],	
                 (CPU_STK_SIZE)Init_STK_SIZE/10,	
                 (CPU_STK_SIZE)Init_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);	 		

	/*
	*************************************************************
	*                        连接服务器任务
	*************************************************************
	*/
	OSTaskCreate((OS_TCB*     )&ConnectServerTaskTCB,		
				 (CPU_CHAR*   )"ConnectServer task", 		
                 (OS_TASK_PTR )IWATCH_Task, 			
                 (void*       )0,					
                 (OS_PRIO	  )ConnectServer_TASK_PRIO,     
                 (CPU_STK*    )&ConnectServer_TASK_STK[0],	
                 (CPU_STK_SIZE)ConnectServer_STK_SIZE/10,	
                 (CPU_STK_SIZE)ConnectServer_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);	 		

	/*
	*************************************************************
	*                        Systemcheckstack任务
	*************************************************************
	*/								 
	OSTaskCreate((OS_TCB*     )&SystemcheckstackTaskTCB,		
				 (CPU_CHAR*   )"Systemcheakstack task", 		
                 (OS_TASK_PTR )Systemcheckstack_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )Systemcheckstack_TASK_PRIO,     
                 (CPU_STK*    )&Systemcheckstack_TASK_STK[0],	
                 (CPU_STK_SIZE)Systemcheckstack_STK_SIZE/10,	
                 (CPU_STK_SIZE)Systemcheckstack_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);								 

	/*
	*************************************************************
	*                        Finger_Task任务
	*************************************************************
	*/
	OSTaskCreate((OS_TCB*     )&SlavetalkTaskTCB,		
				 (CPU_CHAR*   )"Finger_Task", 		
                 (OS_TASK_PTR )Finger_Task, 			
                 (void*       )0,					
                 (OS_PRIO	  )Slavetalk_TASK_PRIO,     
                 (CPU_STK*    )&Slavetalk_TASK_STK[0],	
                 (CPU_STK_SIZE)Slavetalk_STK_SIZE/10,	
                 (CPU_STK_SIZE)Slavetalk_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);										 

  /*
	*************************************************************
	*                        WIFI任务
	*************************************************************
	*/
	OSTaskCreate((OS_TCB*     )&WIFITaskTCB,		
				 (CPU_CHAR*   )"WIFI task", 		
                 (OS_TASK_PTR )ModBus_Task, 			
                 (void*       )0,					
                 (OS_PRIO	  )WIFI_TASK_PRIO,     
                 (CPU_STK*    )&WIFI_TASK_STK[0],	
                 (CPU_STK_SIZE)WIFI_STK_SIZE/10,	
                 (CPU_STK_SIZE)WIFI_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);
						 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			
  OS_CRITICAL_EXIT();	//退出临界区
}

/*
******************************************************************************
*	函 数 名: emwindemo_task
*	功能说明: 屏幕显示 			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：7
******************************************************************************
*/
void emwindemo_task(void *p_arg)
{	
	CreateWindow();
	while(1)
	{
		GUI_Delay(100);
	}
}

/*
******************************************************************************
*	函 数 名: touch_task
*	功能说明: 触屏任务 			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：4
******************************************************************************
*/
void touch_task(void *p_arg)
{
	OS_ERR err;
	while(1)
	{
		GUI_TOUCH_Exec();	 //触屏执行
		OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);//延时5ms
	}
}

/*
******************************************************************************
*	函 数 名: ModBus_Task
*	功能说明: WIFI任务 			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：11
******************************************************************************
*/
void ModBus_Task(void *p_arg)
{
	OS_ERR err;
	WIFI_USART6_init(9600);
	while(1)
	{   
       
		Host485_Task();
		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);//延时100ms
	}
}

/*
******************************************************************************
*	函 数 名: 截屏功能
*	功能说明: LED任务及恢复出厂设置 			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：11
******************************************************************************
*/
void led0_task(void *p_arg)
{
	//	OS_ERR err;

//	char  buf[100];
//	u8 Pic_Name=1;
//	p_arg=p_arg;
//	while(1)
//	{ 
//		//OSSchedLock();
//		printf("ScreenShot Start\r\n");
//    sprintf(buf,"0:/SCREENSHORT_%d.bmp",Pic_Name);
//    emWin_CreateBMPPicture((unsigned char *)buf, 0, 0, 1024, 600);
//    Pic_Name++;
//		printf("ScreenShot  OK\r\n");
//		//OSSchedUnlock(); 
//		OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_PERIODIC,&err);
//  }
	
	
//    MYDMA_Config(DMA2_Stream2,DMA_CHANNEL_5);//初始化DMA
//	  while(1)
//		{
//		 if(USART6_RX_STA==0)
//		 HAL_UART_Transmit_DMA(&husart6,USART6_RX_BUF,USART6_MAX_RECV_LEN);//开启DMA传输
//		 while(1)
//			 {
//		 if(USART6_RX_STA==1)
//        {
//           __HAL_DMA_CLEAR_FLAG(&UART6RxDMA_Handler,DMA_FLAG_FEIF1_5);//清除DMA2_Steam7传输完成标志
//						//DMA_CHANNEL_5->CNDTR=32;
//           HAL_UART_DMAStop(&husart6);      //传输完成以后关闭串口DMA
//				   break; 
//        }
//			 }
//		}
			
}
/*
******************************************************************************
*	函 数 名: Init_task
*	功能说明: 读取存储器任务 			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：6
******************************************************************************
*/
void Init_task(void *p_arg)
{
	OS_ERR err;
	u8 count;
	u8 Init_Flag[1]={0x00};
	u8 Init_Finish_Flag[1]="3";
	u8 Null_char[8]="\0\0\0\0\0\0\0\0";

 
//		AT24CXX_Read(253,Init_Flag,1);
//		if(!(strcmp((char*)Init_Flag,"0")))
//		{	
//			
//			AT24CXX_Write(253,Init_Finish_Flag,1);
//			SCB->AIRCR =0X05FA0000|(u32)0x04;  //系统软复位
//		  
//		}
//		AT24CXX_Write(253,"0",1);
		AT24CXX_Read(232,Server_ip,16);
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
		printf("IP:%s  %d\r\n",Server_ip,sizeof(Server_ip));   
		AT24CXX_Read(216,server_port,6);
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
		printf("Port:%s  %d\r\n",server_port,sizeof(server_port));		
		AT24CXX_Read(222,EquipID,5);
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
		printf("EquipID:%s  %d\r\n",EquipID,sizeof(EquipID));	
		AT24CXX_Read(227,Password,5);
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
		printf("Password:%s  %d\r\n",Password,sizeof(Password));	
		
    AT24CXX_Read(254,Init_Flag,1);
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);
		if(strcmp((char*)Init_Flag,"3"))
		{ 
	  for(count=0;count<32;count++)
	  { 
	  	 AT24CXX_Write(count*8,Null_char,8);
	  	 OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);							//延时5ms

	  }
		  AT24CXX_Write(254,Init_Finish_Flag,1);
		  OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);		
		  printf("AT24C02 Init Finish \r\n");
	  }else printf("AT24C02 Already Init  \r\n");
		
		
		  OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_PERIODIC,&err); 			
			OSTaskSemPost(&SlavetalkTaskTCB,OS_OPT_POST_NONE,&err);		//发送信号量connectServer任务
		 // OSTaskSemPost(&ConnectServerTaskTCB,OS_OPT_POST_NONE,&err);		//发送信号量connectServer任务
 	  	OSTaskSuspend((OS_TCB*)&InitTaskTCB,&err);				   					//任务挂起
		  OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err); 				  //延时500ms
//	}
}

/*
******************************************************************************
*	函 数 名: IWATCH_Task
*	功能说明: 连接服务器任务 			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：8
******************************************************************************
*/
void IWATCH_Task(void *p_arg)
{		
		OS_ERR err;
    GPRS_Task();
		OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_PERIODIC,&err);//延时5ms
}

/*
******************************************************************************
*	函 数 名: Systemcheckstack_task
*	功能说明: 看门狗	  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：10
******************************************************************************
*/
void Systemcheckstack_task(void *p_arg)
{
//	u16 memused=0;
//	u8 paddr[20];
	OS_ERR err;
//	CPU_STK_SIZE free,used;
	(void) p_arg;
  IWDG_Init(4, 30000);
	
	while(1)
{
   
 	IWDG_Feed();
	OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_PERIODIC,&err);//延时5ms
	
}
	
	
	
	
	
	

//	while(1)
//	{
//		/***************打印主机相关内存使用情况**************/
//		OSTimeDlyHMSM(0,0,0,80,OS_OPT_TIME_PERIODIC,&err);//延时80ms
//		OSTaskStkChk(&SystemcheckstackTaskTCB,&free,&used,&err);
//		printf("Systemcheckstack:     used/free:%d/%d    usage:%d%%\r\n",used,free,(used*100)/(used+free));
//	
//		OSTaskStkChk (&StartTaskTCB,&free,&used,&err);  
//    printf("StartTaskTCB          used/free:%d/%d       usage:%d%%\r\n",used,free,(used*100)/(used+free));  
//		
//		OSTaskStkChk (&TouchTaskTCB,&free,&used,&err);  
//    printf("TouchTaskTCB          used/free:%d/%d     usage:%d%%\r\n",used,free,(used*100)/(used+free));  
//				
//		OSTaskStkChk (&Led0TaskTCB,&free,&used,&err);  
//    printf("Led0TaskTCB           used/free:%d/%d     usage:%d%%\r\n",used,free,(used*100)/(used+free));  
//		
//		OSTaskStkChk (&EmwindemoTaskTCB,&free,&used,&err);  
//    printf("EmwindemoTaskTCB      used/free:%d/%d  usage:%d%%\r\n",used,free,(used*100)/(used+free));  
//				
//		OSTaskStkChk (&InitTaskTCB,&free,&used,&err);  
//    printf("InitTaskTCB           used/free:%d/%d     usage:%d%%\r\n",used,free,(used*100)/(used+free));  
//		
//		OSTaskStkChk (&ConnectServerTaskTCB,&free,&used,&err);  
//    printf("ConnectServerTaskTCB  used/free:%d/%d    usage:%d%%\r\n",used,free,(used*100)/(used+free));  
//		
//		OSTaskStkChk (&SlavetalkTaskTCB,&free,&used,&err);  
//    printf("SlavetalkTaskTCB      used/free:%d/%d   usage:%d%%\r\n",used,free,(used*100)/(used+free));  

//		memused=my_mem_perused(SRAMIN);
//		sprintf((char *)paddr,"%d.%01d%%",memused/10,memused%10);
//		printf("SRAMIN:%s\r\n",paddr);

//		memused=my_mem_perused(SRAMEX);
//		sprintf((char *)paddr,"%d.%01d%%",memused/10,memused%10);
//		printf("SRAMEX:%s\r\n",paddr);
//		
//		memused=my_mem_perused(SRAMDTCM);
//		sprintf((char *)paddr,"%d.%01d%%",memused/10,memused%10);
//		printf("SRAMDTCM:%s\r\n",paddr);


//		printf("\r\n\r\n");
//		OSTimeDlyHMSM(0,1,0,0,OS_OPT_TIME_PERIODIC,&err);//延时60s
//	}
}

/*
******************************************************************************
*	函 数 名: Finger_Task
*	功能说明: 指纹任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：9
******************************************************************************
*/
void Finger_Task(void *p_arg)
{

	OS_ERR err;
	FingerPrint_Task();
	OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);
	
}
		 
/*
******************************************************************************
*	函 数 名: Sys_Init
*	功能说明: 系统初始化
*	形    参：无
*	返 回 值: 无
* 优 先 级：9
******************************************************************************
*/
void Sys_Init(void)
{   
	  OS_ERR err;
	  u8 Font_Init[1]={0x00}; 
		u8 Font_Init_Finish_Flag[1]=".";
		Write_Through();                //Cahce强制透写
    MPU_Memory_Protection();        //保护相关存储区域
    Cache_Enable();                 //打开L1-Cache
   	Stm32_Clock_Init(432,25,2,9);   //设置时钟,216Mhz 
		HAL_Init();				       			  //初始化HAL库
    delay_init(216);                //延时初始化
		uart_init(115200);		        	//串口初始化
		GPRS_UART2_init(115200);				//GPRS串口2初始化
		WIFI_USART6_init(9600);  		  //初始化串口7波特率为115200
		MESH_UART3_init(57600);
    LED_Init();                     //初始化LED
		KEY_Init();											//初始化KEY
    SDRAM_Init();                   //SDRAM初始化
		TFTLCD_Init();                  //初始化LCD
    TP_Init();				              //触摸屏初始化
		RTC_Init();                     //初始化RTC 
		AT24CXX_Init();
//	  MYDMA_Config(DMA2_Stream2,DMA_CHANNEL_5);//初始化DMA
    my_mem_init(SRAMIN);		        //初始化内部内存池
		my_mem_init(SRAMEX);		        //初始化外部内存池
		my_mem_init(SRAMDTCM);		      //初始化DTCM内存池
		exfuns_init();			            //为fatfs相关变量申请内存				 
  	f_mount(fs[0],"0:",1); 		      //挂载SD卡
		f_mount(fs[1],"1:",1); 	        //挂载FLASH.	
		AT24CXX_Read(253,Font_Init,1);
		if(font_init()||strcmp((char*)Font_Init,"."))		//初始化字库
//  	if(font_init())		//初始化字库
		{
		AT24CXX_Write(253,Font_Init_Finish_Flag,1);
		printf("Font Error \r\n");
		SD_Init();
	  update_font(30,90,16,"0:");	//如果字库不存在就更新字库
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_PERIODIC,&err);//延时5ms
    }else printf("Font OK");
}

/*
******************************************************************************
*	函 数 名: start_task_init
*	功能说明: 创建开始任务
*	形    参：无
*	返 回 值: 无
******************************************************************************
*/
void start_task_init(void)
{
	OS_ERR err;
		//创建开始任务
		OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
}

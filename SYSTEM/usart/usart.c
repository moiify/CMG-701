/*
*********************************************************************************************************
*	                                  
*	模块名称 : 串口初始化
*	文件名称 : usart.c
*	说    明 : 串口初始化，中断函数
*
*********************************************************************************************************
*/
#include "usart.h"
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif

#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕  
	USART1->TDR=(u8)ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   //如果使能了接收
/*
*********************************************************************
*                            静态全局变量
*********************************************************************
*/
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART6_RX_BUF[USART6_MAX_RECV_LEN];
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				    //接收缓冲,最大USART2_MAX_RECV_LEN个字节.

//串口发送缓存区 	
__align(8) u8 USART6_TX_BUF[USART6_MAX_SEND_LEN]; 	//发送缓冲,最大USART6_MAX_SEND_LEN字节 
__align(8) u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; //发送缓冲,最大USART2_MAX_SEND_LEN字节


u16 USART_RX_STA=0;       //接收状态标记
u16 USART6_RX_STA=0; 
u16 USART3_RX_STA=0;
//------------------------------------------------------------------------------------定义变量
u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲

u8 time;
u8 USART6_re;
u8 USART3_re;
static u8 num = 0;
u8 flag = 0;
u8 Gprs_res[200];
u8 Finger_res[200];
u8 Modbus_res[200];
UART_HandleTypeDef UART1_Handler;       //UART句柄
UART_HandleTypeDef GPRS_UART2_Handler;  //串口2接GPRS
UART_HandleTypeDef MESH_UART3_Handler;  //串口3接MESH
UART_HandleTypeDef husart6; //串口6接WIFI
TIM_HandleTypeDef TIM7_Handler;         //定时器句柄 
TIM_HandleTypeDef TIM5_Handler;         //定时器句柄 
TIM_HandleTypeDef TIM3_Handler; 


extern u8 Flag_GPRS_R;
extern u8 MESH_DATA[120];
extern u8 Flag_MESH_R;
extern u8 MESH_R_COUNT;
extern OS_TMR 	tmr1;		//定时器1
extern OS_TMR 	MESH_tmr;		//MESH定时器
extern u8 GPRS_DATA[100];

RS_Cache RS485_Cache = 
{
    .Size = 20,
    .In = 0,
    .Out = 0,
    .Count = 0, 
};

/*
****************************************************************************
*	函 数 名: WIFI_USART6_init
*	功能说明: 初始化IO 串口6
*	形    参：bound			波特率     
*	返 回 值: 无
****************************************************************************
*/
 void WIFI_USART6_init(u32 bound)                          
{
	husart6.Instance=USART6;					    			  //USART6
	husart6.Init.BaudRate=9600;				    		 	//波特率
	husart6.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	husart6.Init.StopBits=UART_STOPBITS_1;	      //一个停止位
	husart6.Init.Parity=UART_PARITY_NONE;		      //无奇偶校验位
	husart6.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	husart6.Init.Mode=UART_MODE_TX_RX;		        //收发模式
	HAL_UART_Init(&husart6);					    				//HAL_UART_Init()会使能USART6
	HAL_UART_Receive_IT(&husart6, (u8 *)Modbus_res, 1);
}

/*
****************************************************************************
*	函 数 名: MESH_UART3_init
*	功能说明: 初始化IO 串口3
*	形    参：bound			波特率     
*	返 回 值: 无
****************************************************************************
*/
 void MESH_UART3_init(u32 bound)                          
{
	MESH_UART3_Handler.Instance=USART3;					    				 //USART3
	MESH_UART3_Handler.Init.BaudRate=57600;				    			 //波特率
	MESH_UART3_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	MESH_UART3_Handler.Init.StopBits=UART_STOPBITS_1;	    	 //一个停止位
	MESH_UART3_Handler.Init.Parity=UART_PARITY_NONE;		     //无奇偶校验位
	MESH_UART3_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	MESH_UART3_Handler.Init.Mode=UART_MODE_TX_RX;		  		   //收发模式
	HAL_UART_Init(&MESH_UART3_Handler);					   					 //HAL_UART_Init()会使能UART3
	
	USART3->CR1|=1<<2;
	USART3->CR1|=1<<5;
   // HAL_UART_Receive_IT(&MESH_UART3_Handler, (u8 *)Mesh_res, 1);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
}

/*
****************************************************************************
*	函 数 名: GPRS_UART2_init
*	功能说明: 初始化IO 串口2
*	形    参：bound			波特率     
*	返 回 值: 无
****************************************************************************
*/
 void GPRS_UART2_init(u32 bound)                          
{
	GPRS_UART2_Handler.Instance=USART2;					   					 //USART2
	GPRS_UART2_Handler.Init.BaudRate=bound;				    			 //波特率
	GPRS_UART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	GPRS_UART2_Handler.Init.StopBits=UART_STOPBITS_1;	    	 //一个停止位
	GPRS_UART2_Handler.Init.Parity=UART_PARITY_NONE;		     //无奇偶校验位
	GPRS_UART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	GPRS_UART2_Handler.Init.Mode=UART_MODE_TX_RX;		   		   //收发模式
	HAL_UART_Init(&GPRS_UART2_Handler);					   					 //HAL_UART_Init()会使能UART2
	
	HAL_UART_Receive_IT(&GPRS_UART2_Handler, (u8 *)Gprs_res, 1);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
}

/*
****************************************************************************
*	函 数 名: uart_init
*	功能说明: 初始化IO 串口1
*	形    参：bound			波特率     
*	返 回 值: 无
****************************************************************************
*/
void uart_init(u32 bound)
{	
	//UART 初始化设置
	UART1_Handler.Instance=USART1;					   					//USART1
	UART1_Handler.Init.BaudRate=bound;				   				//波特率
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    	//一个停止位
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    	//无奇偶校验位
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    		//收发模式
	HAL_UART_Init(&UART1_Handler);					    				//HAL_UART_Init()会使能UART1
	
	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
}

/*
****************************************************************************
*	函 数 名: HAL_UART_MspInit
*	功能说明: UART底层初始化，时钟使能，引脚配置，中断配置
						此函数会被HAL_UART_Init()调用
*	形    参：huart		串口句柄  
*	返 回 值: 无
****************************************************************************
*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//如果是串口1，进行串口1 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();						//使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			 		//使能USART1时钟
	
		GPIO_Initure.Pin=GPIO_PIN_9;						//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;			//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;					//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FAST;			//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART1;	//复用为USART1
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA9

		GPIO_Initure.Pin=GPIO_PIN_10;						//PA10
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA10
		
#if EN_USART1_RX
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//使能USART1中断通道
		HAL_NVIC_SetPriority(USART1_IRQn,3,3);	//抢占优先级3，子优先级3
#endif	
	}
	
	if(huart->Instance==USART2)//如果是串口2，进行串口2 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();						//使能GPIOA时钟			
		__HAL_RCC_USART2_CLK_ENABLE();					//使能USART2时钟
	
		GPIO_Initure.Pin=GPIO_PIN_2;						//PA2
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;			//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;					//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FAST;			//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART2;	//复用为USART2
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA2

		GPIO_Initure.Pin=GPIO_PIN_3;						//PA3
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA3
		

		HAL_NVIC_EnableIRQ(USART2_IRQn);				//使能USART2中断通道
		HAL_NVIC_SetPriority(USART2_IRQn,3,2);	//抢占优先级3，子优先级2
	}
	
	if(huart->Instance==USART3)//如果是串口3，进行串口3 MSP初始化
	{
		__HAL_RCC_GPIOB_CLK_ENABLE();						//使能GPIOB时钟			
		__HAL_RCC_USART3_CLK_ENABLE();					//使能USART3时钟
	
		GPIO_Initure.Pin=GPIO_PIN_10;						//PB10
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;			//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;					//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FAST;			//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART3;	//复用为USART3
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//初始化PB10

		GPIO_Initure.Pin=GPIO_PIN_11;						//PB11
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//初始化PB11
		__HAL_UART_ENABLE_IT(&MESH_UART3_Handler,UART_IT_RXNE);//开启接收中断
		HAL_NVIC_EnableIRQ(USART3_IRQn);				//使能USART3中断通道
		HAL_NVIC_SetPriority(USART3_IRQn,2,2);	//抢占优先级3，子优先级1
		TIM5_Int_Init(1000-1,9000-1);						//100ms中断
		USART3_RX_STA=0;												//清零
		TIM5->CR1&=~(1<<0);      							  //关闭定时器7
	}
	
	if(huart->Instance==USART6)//如果是串口6，进行串口6 MSP初始化
	{
		__HAL_RCC_GPIOC_CLK_ENABLE();			//使能GPIOC时钟		
		__HAL_RCC_USART6_CLK_ENABLE();		//使能USART6时钟
	
		GPIO_Initure.Pin=GPIO_PIN_6;			      //PC6
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		  //复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;			    //上拉
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		  //高速
		GPIO_Initure.Alternate=GPIO_AF8_USART6;	//复用为USART6
		HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	//初始化PC6

		GPIO_Initure.Pin=GPIO_PIN_7;			      //PC7
		HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	//初始化PC7
		
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);//开启接收中断
		HAL_NVIC_EnableIRQ(USART6_IRQn);				//使能USART6中断通道
		HAL_NVIC_SetPriority(USART6_IRQn,2,3);	//抢占优先级2，子优先级3
		TIM7_Int_Init(50-1,10800-1);						//8ms中断
		TIM7->CR1&=~(1<<0);      							  //关闭定时器7
		//HAL_UART_Receive_IT(&husart6, (u8 *)Modbus_res, 1) != HAL_OK);
	}
}

/*
****************************************************************************
*	函 数 名: HAL_UART_RxCpltCallback
*	功能说明: 中断回调函数
*	形    参：huart		串口句柄  
*	返 回 值: 无
****************************************************************************
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//如果是串口1
	{
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}
	}
	
	if(huart->Instance==USART2)							//---------------------------------如果是GPRS的中断USART2
	{
		strcat((char *)GPRS_DATA,(char *)Gprs_res);
		Flag_GPRS_R=1;
		
	}
	
	
	if(huart->Instance==USART3)	
	{	
	 
		if((USART3_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM5->CNT=0;         				//计数器清空	
				if(USART3_RX_STA==0) 				//使能定时器7的中断 
				{
				TIM5->CR1|=1<<0;     			//使能定时器7
				}
				USART3_RX_BUF[USART3_RX_STA++] = Finger_res[0];	//记录接收到的值	 
			}
			else 
			{
				USART3_RX_STA|=1<<15;				//强制标记接收完成
			} 
		
		}
	}	
	if(huart->Instance==USART6)//接收到数据
	{
	
			TIM7->CNT=0;         				//计数器清空	

			TIM7->CR1|=1<<0;     			//使能定时器7					

			USART6_RX_BUF[USART6_RX_STA++] = Modbus_res[0];;	//记录接收到的值	 
      
			HAL_UART_Receive_IT(&husart6, (u8 *)Modbus_res, 1);
	}  	
}


/*
****************************************************************************
*	函 数 名: USART1_IRQHandler
*	功能说明: 串口1中断服务程序
*	形    参：无
*	返 回 值: 无
****************************************************************************
*/
void USART1_IRQHandler(void)                	
{ 
	u32 timeout=0;
    u32 maxDelay=0x1FFFF;
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntEnter();    
#endif
	
	HAL_UART_IRQHandler(&UART1_Handler);	//调用HAL库中断处理公用函数
	
	timeout=0;
  while (HAL_UART_GetState(&UART1_Handler)!=HAL_UART_STATE_READY)//等待就绪
	{
    timeout++;////超时处理
    if(timeout>maxDelay) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler,(u8 *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
		timeout++; //超时处理
		if(timeout>maxDelay) break;	
	}
#if SYSTEM_SUPPORT_OS	 	//使用OS
	OSIntExit();  											 
#endif
} 

/*
****************************************************************************
*	函 数 名: USART2_IRQHandler
*	功能说明: 串口2中断服务程序
*	形    参：无
*	返 回 值: 无
****************************************************************************
*/
void USART2_IRQHandler(void)
{ 

		u32 timeout=0;
    u32 maxDelay=0x1FFFF;
		OSIntEnter();    
		HAL_UART_IRQHandler(&GPRS_UART2_Handler);	//调用HAL库中断处理公用函数
	
	timeout=0;
  while (HAL_UART_GetState(&GPRS_UART2_Handler)!=HAL_UART_STATE_READY)//等待就绪
	{
    timeout++;////超时处理
    if(timeout>maxDelay) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&GPRS_UART2_Handler,(u8 *)Gprs_res, 1)!=HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
		timeout++; //超时处理
		if(timeout>maxDelay) break;	
	}
	OSIntExit();  		
}

/*
****************************************************************************
*	函 数 名: USART3_IRQHandler
*	功能说明: 串口3中断服务程序
*	形    参：无
*	返 回 值: 无
****************************************************************************
*/
void USART3_IRQHandler(void)
{
	
  OSIntEnter(); 
	HAL_UART_IRQHandler(&MESH_UART3_Handler);	//调用HAL库中断处理公用函数
	while (HAL_UART_GetState(&MESH_UART3_Handler) != HAL_UART_STATE_READY){;}//等待就绪
	while(HAL_UART_Receive_IT(&MESH_UART3_Handler, (u8 *)Finger_res, 1) != HAL_OK){;}	
	OSIntExit();
} 

/*
****************************************************************************
*	函 数 名: USART6_IRQHandler
*	功能说明: 串口6中断服务程序
*	形    参：无
*	返 回 值: 无
****************************************************************************
*/
void USART6_IRQHandler(void)
{
//	u8 res;	
//	OSIntEnter(); 
//	if(USART6->ISR&(1<<5))//接收到数据
//	{	 
//		USART6->ISR&=~(1<<5);
//		res=USART6->RDR; 			 
//		if((USART6_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
//		{ 
//			if(USART6_RX_STA<USART6_MAX_RECV_LEN)	//还可以接收数据
//			{
//				TIM7->CNT=0;         				//计数器清空	
//				if(USART6_RX_STA==0) 				//使能定时器7的中断 
//				{
//					TIM7->CR1|=1<<0;     			//使能定时器7					
//				}
//				USART6_RX_BUF[USART6_RX_STA++] = res;	//记录接收到的值	 
//			}
//			else 
//			{
//				USART6_RX_STA|=1<<15;				//强制标记接收完成
//			} 
//		}
//		OSIntExit();
//	}  
	OSIntEnter(); 
	HAL_UART_IRQHandler(&husart6);	//调用HAL库中断处理公用函数
	OSIntExit();	
} 	
/*
****************************************************************************
*	函 数 名: GPRS_Sendcom
*	功能说明: 串口2发送数据
*	形    参：Gprs_com 		发送的数据指针
*	返 回 值: 无
****************************************************************************
*/
void GPRS_Sendcom(u8 * Gprs_com)
{
	HAL_UART_Transmit(&GPRS_UART2_Handler,Gprs_com,strlen((char *)Gprs_com),0xffff);
	while(__HAL_UART_GET_FLAG(&GPRS_UART2_Handler,UART_FLAG_TC)!=SET); 
}

/*
****************************************************************************
*	函 数 名: MESH_Sendcom
*	功能说明: 串口3发送数据
*	形    参：MESH_com 		发送的数据指针  
						len					发送的长度
*	返 回 值: 无
****************************************************************************
*/
void MESH_Sendcom(u8 * MESH_com,u8 len)
{
	u8 i;
	for(i=0;i<len;i++)
	{
		HAL_UART_Transmit(&MESH_UART3_Handler,MESH_com+i,1,1000); 
	}
}

/*
****************************************************************************
*	函 数 名: TIM7_Int_Init
*	功能说明: 基本定时器7中断初始化
*	形    参：arr			自动重装值。
						psc			时钟预分频数
*	返 回 值: 无
****************************************************************************
*/
void TIM7_Int_Init(u16 arr,u16 psc)
{
		TIM7_Handler.Instance=TIM7;                          //通用定时器7
    TIM7_Handler.Init.Prescaler=psc;                     //分频系数
    TIM7_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM7_Handler.Init.Period=arr;                        //自动装载值
    TIM7_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM7_Handler);
    HAL_TIM_Base_Start_IT(&TIM7_Handler); //使能和定时器7更新中断：TIM_IT_UPDATE									 
}
void TIM5_Int_Init(u16 arr,u16 psc)
{
		TIM5_Handler.Instance=TIM5;                          //通用定时器5
    TIM5_Handler.Init.Prescaler=psc;                     //分频系数
    TIM5_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM5_Handler.Init.Period=arr;                        //自动装载值
    TIM5_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM5_Handler);
    HAL_TIM_Base_Start_IT(&TIM5_Handler); //使能和定时器5更新中断：TIM_IT_UPDATE									 
}

/*
****************************************************************************
*	函 数 名: HAL_TIM_Base_MspInit
*	功能说明: 定时器底册驱动，开启时钟，设置中断优先级
*	形    参：htim  定时器的种类
*	返 回 值: 无
****************************************************************************
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM7)
	{
		__HAL_RCC_TIM7_CLK_ENABLE();            //使能TIM7时钟
		HAL_NVIC_SetPriority(TIM7_IRQn,0,1);    //设置中断优先级，抢占优先级0，子优先级1
		HAL_NVIC_EnableIRQ(TIM7_IRQn);          //开启ITM7中断   
	}
	else if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);
		HAL_NVIC_EnableIRQ(TIM3_IRQn); 
	}
	else if(htim->Instance==TIM5)
	{
		__HAL_RCC_TIM5_CLK_ENABLE();            //使能TIM7时钟
		HAL_NVIC_SetPriority(TIM5_IRQn,0,2);    //设置中断优先级，抢占优先级0，子优先级1
		HAL_NVIC_EnableIRQ(TIM5_IRQn);          //开启ITM7中断   
	}
}

/*
****************************************************************************
*	函 数 名: TIM7_IRQHandler
*	功能说明: 定时器7中断函数
*	形    参：无
*	返 回 值: 无
****************************************************************************
*/	    
void TIM7_IRQHandler(void)
{ 	
	static u8 CopyPoint;
	 
	__HAL_TIM_CLEAR_FLAG(&TIM7_Handler,TIM_EventSource_Update );       //清除TIM7更新中断标志  
	TIM7->CR1&=~(1<<0);     			//关闭定时器7
	if(USART6_RX_STA>0)
	{
        if(RS485_Cache.Count < RS485_Cache.Size)
        {
            ModbusRevBuf_t *p;
            if(USART6_RX_STA>100)
            USART6_RX_STA=100;
            p = &RS485_Cache.Buf[RS485_Cache.In];
             p->Len = USART6_RX_STA;
            USART6_RX_STA=0;	
            memcpy(p->Buf,USART6_RX_BUF,p->Len);
            memset(USART6_RX_BUF,0,sizeof(USART6_RX_BUF));
            RS485_Cache.In++;
            RS485_Cache.In %= sizeof(RS485_Cache.Buf)/sizeof(RS485_Cache.Buf[0]);
            RS485_Cache.Count++;
        }
    }		 
} 
void TIM5_IRQHandler(void)
{ 	

		USART3_RX_STA |= 1<<15;	//标记接收完成
		__HAL_TIM_CLEAR_FLAG(&TIM5_Handler,TIM_EventSource_Update );       //清除TIM7更新中断标志  
		TIM5->CR1&=~(1<<0);     			//关闭定时器7   		
  			
} 

/*
****************************************************************************
*	函 数 名: TIM3_Int_Init
*	功能说明: 基本定时器3中断初始化
*	形    参：arr			自动重装值。
						psc			时钟预分频数
*	返 回 值: 无
****************************************************************************
*/
void TIM3_Init(u16 arr,u16 psc)
{
	num = 0;
	TIM3_Handler.Instance=TIM3; 
	TIM3_Handler.Init.Prescaler=psc; 
	TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP; 
	TIM3_Handler.Init.Period=arr; 
	TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1; 
	HAL_TIM_Base_Init(&TIM3_Handler);  
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
}

/*
****************************************************************************
*	函 数 名: TIM3_IRQHandler
*	功能说明: 定时器3中断函数
*	形    参：无
*	返 回 值: 无
****************************************************************************
*/
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TIM3_Handler);
}

/*
****************************************************************************
*	函 数 名: HAL_TIM_PeriodElapsedCallback
*	功能说明: 定时器中断回调函数
*	形    参：htim		定时器中断的种类
*	返 回 值: 无
****************************************************************************
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==(&TIM3_Handler))
	{
		num++;
//		printf("%d\r\n", num);

		if(num%time == 0)
		{
			num = 0;
			flag = 1;
		}
	}
}

/*
****************************************************************************
*	函 数 名: TIM3_stop
*	功能说明: 定时器3关闭函数
*	形    参：arr			自动重装值。
						psc			时钟预分频书
*	返 回 值: 无
****************************************************************************
*/
void TIM3_stop(u16 arr,u16 psc)
{
	TIM3_Handler.Instance=TIM3; 
	TIM3_Handler.Init.Prescaler=psc; 
	TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP; 
	TIM3_Handler.Init.Period=arr; 
	TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1; 
	HAL_TIM_Base_Init(&TIM3_Handler);  
	HAL_TIM_Base_Stop_IT(&TIM3_Handler);
}


#endif

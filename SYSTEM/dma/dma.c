#include "dma.h"
#include "usart.h"


DMA_HandleTypeDef  UART6RxDMA_Handler;      //DMA句柄

//DMAx的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMA通道选择,@ref DMA_channel DMA_CHANNEL_0~DMA_CHANNEL_7
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx)
{ 
	if((u32)DMA_Streamx>(u32)DMA2)//得到当前stream是属于DMA2还是DMA1
	{
        __HAL_RCC_DMA2_CLK_ENABLE();//DMA2时钟使能	
	}else 
	{
        __HAL_RCC_DMA1_CLK_ENABLE();//DMA1时钟使能 
	}
    
    __HAL_LINKDMA(&husart6,hdmarx,UART6RxDMA_Handler);   
    
    //Tx DMA配置
    UART6RxDMA_Handler.Instance=DMA_Streamx;                            //数据流选择
    UART6RxDMA_Handler.Init.Channel=chx;                                //通道选择
    UART6RxDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;             //存储器到外
    UART6RxDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;                 //外设自增模式关闭
    UART6RxDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                     //存储器增量模式
    UART6RxDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;    //外设数据长度:8位
    UART6RxDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;       //存储器数据长度:8位
    UART6RxDMA_Handler.Init.Mode=DMA_CIRCULAR;                            //外设流控模式
    UART6RxDMA_Handler.Init.Priority=DMA_PRIORITY_MEDIUM;               //中等优先级
    UART6RxDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;              
    UART6RxDMA_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;      
    UART6RxDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;                 //存储器突发单次传输
    UART6RxDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;              //外设突发单次传输
    
    HAL_DMA_DeInit(&UART6RxDMA_Handler);   
    HAL_DMA_Init(&UART6RxDMA_Handler);
} 


 
 

#include "dma.h"
#include "usart.h"


DMA_HandleTypeDef  UART6RxDMA_Handler;      //DMA���

//DMAx�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMAͨ��ѡ��,@ref DMA_channel DMA_CHANNEL_0~DMA_CHANNEL_7
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx)
{ 
	if((u32)DMA_Streamx>(u32)DMA2)//�õ���ǰstream������DMA2����DMA1
	{
        __HAL_RCC_DMA2_CLK_ENABLE();//DMA2ʱ��ʹ��	
	}else 
	{
        __HAL_RCC_DMA1_CLK_ENABLE();//DMA1ʱ��ʹ�� 
	}
    
    __HAL_LINKDMA(&husart6,hdmarx,UART6RxDMA_Handler);   
    
    //Tx DMA����
    UART6RxDMA_Handler.Instance=DMA_Streamx;                            //������ѡ��
    UART6RxDMA_Handler.Init.Channel=chx;                                //ͨ��ѡ��
    UART6RxDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;             //�洢������
    UART6RxDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;                 //��������ģʽ�ر�
    UART6RxDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                     //�洢������ģʽ
    UART6RxDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;    //�������ݳ���:8λ
    UART6RxDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;       //�洢�����ݳ���:8λ
    UART6RxDMA_Handler.Init.Mode=DMA_CIRCULAR;                            //��������ģʽ
    UART6RxDMA_Handler.Init.Priority=DMA_PRIORITY_MEDIUM;               //�е����ȼ�
    UART6RxDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;              
    UART6RxDMA_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;      
    UART6RxDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;                 //�洢��ͻ�����δ���
    UART6RxDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;              //����ͻ�����δ���
    
    HAL_DMA_DeInit(&UART6RxDMA_Handler);   
    HAL_DMA_Init(&UART6RxDMA_Handler);
} 


 
 

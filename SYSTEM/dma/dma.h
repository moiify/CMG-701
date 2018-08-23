#ifndef __DMA_H
#define __DMA_H
#include "sys.h"


extern DMA_HandleTypeDef  UART6RxDMA_Handler;      //DMA¾ä±ú

void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx);
 
#endif

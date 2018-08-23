#include "iwdg.h"
#include "sys.h"

IWDG_HandleTypeDef IWDG_Handler;

void IWDG_Init(u8 prer, u16 rlr)
{
	IWDG_Handler.Instance = IWDG;
	IWDG_Handler.Init.Prescaler = prer;
	IWDG_Handler.Init.Reload = rlr;
	IWDG_Handler.Init.Window = IWDG_WINDOW_DISABLE;
	HAL_IWDG_Init(&IWDG_Handler);
}

void IWDG_Feed(void)
{
	HAL_IWDG_Refresh(&IWDG_Handler);
}

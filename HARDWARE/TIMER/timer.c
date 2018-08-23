#include "timer.h"
#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//��ʱ���ж���������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
//�޸�˵��
//V1.1 20160711
//����TIM3_PWM_Init����,����PWM���
////////////////////////////////////////////////////////////////////////////////// 	 

TIM_HandleTypeDef TIM3_Handler;         //��ʱ����� 
TIM_OC_InitTypeDef TIM3_CH4Handler;     //��ʱ��3ͨ��4���
TIM_OC_InitTypeDef TIM3_CH3Handler;	    //��ʱ��3ͨ��3���

//TIM3 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM3_PWM_Init(u16 arr,u16 psc)
{ 
    TIM3_Handler.Instance=TIM3;            //��ʱ��3
    TIM3_Handler.Init.Prescaler=psc;       //��ʱ����Ƶ
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;//���ϼ���ģʽ
    TIM3_Handler.Init.Period=arr;          //�Զ���װ��ֵ
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIM3_Handler);       //��ʼ��PWM
    
    TIM3_CH3Handler.OCMode=TIM_OCMODE_PWM1; //ģʽѡ��PWM1
    TIM3_CH3Handler.Pulse=arr/2;            //���ñȽ�ֵ,��ֵ����ȷ��ռ�ձȣ�
                                            //Ĭ�ϱȽ�ֵΪ�Զ���װ��ֵ��һ��,��ռ�ձ�Ϊ50%
    TIM3_CH3Handler.OCPolarity=TIM_OCPOLARITY_LOW; //����Ƚϼ���Ϊ�� 
    HAL_TIM_PWM_ConfigChannel(&TIM3_Handler,&TIM3_CH3Handler,TIM_CHANNEL_3);//����TIM3ͨ��3
    HAL_TIM_PWM_Start(&TIM3_Handler,TIM_CHANNEL_3);//����PWMͨ��3
}

//��ʱ���ײ�������ʱ��ʹ�ܣ���������
//�˺����ᱻHAL_TIM_PWM_Init()����
//htim:��ʱ�����
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_TIM3_CLK_ENABLE();			//ʹ�ܶ�ʱ��3
    __HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��
	
    GPIO_Initure.Pin=GPIO_PIN_0;           	//PB0
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;  	//�����������
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	GPIO_Initure.Alternate= GPIO_AF2_TIM3;	//PB1����ΪTIM3_CH4
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}

//����TIMͨ��3��ռ�ձ�
//TIM_TypeDef:��ʱ��
//compare:�Ƚ�ֵ
void TIM_SetCompare3(TIM_TypeDef *TIMx,u32 compare)
{
	TIMx->CCR3=compare;
}

//����TIMͨ��4��ռ�ձ�
//compare:�Ƚ�ֵ
void TIM_SetTIM3Compare4(u32 compare)
{
	TIM3->CCR4=compare; 
}

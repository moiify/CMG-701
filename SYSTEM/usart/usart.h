#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
#include "delay.h"

#define USART_REC_LEN  			200  	//¶¨Òå×î´ó½ÓÊÕ×Ö½ÚÊý 200
#define EN_USART1_RX 			1		//Ê¹ÄÜ£¨1£©/½ûÖ¹£¨0£©´®¿Ú1½ÓÊÕ
#define RXBUFFERSIZE   200 //»º´æ´óÐ¡
//´®¿Ú6Ïà¹Øºê¶¨Òå
#define USART6_MAX_RECV_LEN 400 //×î´ó½ÓÊÕ»º´æ×Ö½ÚÊý
#define USART6_MAX_SEND_LEN	400	//×î´ó·¢ËÍ»º´æ×Ö½ÚÊý
#define USART6_RX_EN 1         //0 ²»½ÓÊÕ  1 ½ÓÊÕ	  
#define USART3_MAX_RECV_LEN		200					//×î´ó½ÓÊÕ»º´æ×Ö½ÚÊý
#define USART3_MAX_SEND_LEN		400					//×î´ó·¢ËÍ»º´æ×Ö½ÚÊý
#define USART3_RX_EN 			1								//0,²»½ÓÊÕ;1,½ÓÊÕ.
/*
************************************************************************************
*	                              	     È«¾Ö±äÁ¿
************************************************************************************
*/

typedef struct
{
	uint8_t Buf[100];
	uint8_t Len;
}ModbusRevBuf_t;

typedef struct 
{ 
  ModbusRevBuf_t Buf[20];
  u8 In;	
  u8 Out;
  u8 Count;
  uint8_t Size;  
}RS_Cache;


extern u8  USART_RX_BUF[USART_REC_LEN]; //½ÓÊÕ»º³å,×î´óUSART_REC_LEN¸ö×Ö½Ú.Ä©×Ö½ÚÎª»»ÐÐ·û
extern u8 USART6_RX_BUF[USART6_MAX_RECV_LEN];
extern u16 USART_RX_STA;         		//½ÓÊÕ×´Ì¬±ê¼Ç	
extern UART_HandleTypeDef UART1_Handler; //UART¾ä±ú
extern UART_HandleTypeDef husart6;

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//½ÓÊÕ»º³å,×î´óUSART2_MAX_RECV_LEN×Ö½Ú
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//·¢ËÍ»º³å,×î´óUSART2_MAX_SEND_LEN×Ö½Ú
extern u16 USART3_RX_STA;   				 //½ÓÊÕÊý¾Ý×´Ì¬


extern u8 aRxBuffer[RXBUFFERSIZE];//HAL¿âUSART½ÓÊÕBuffer

extern RS_Cache RS485_Cache;



extern u8  USART6_RX_BUF[USART6_MAX_RECV_LEN]; 		//½ÓÊÕ»º³å,×î´óUSART6_MAX_RECV_LEN×Ö½Ú
extern u8  USART6_TX_BUF[USART6_MAX_SEND_LEN]; 		//·¢ËÍ»º³å,×î´óUSART6_MAX_SEND_LEN×Ö½Ú
extern u16 USART6_RX_STA;   						//½ÓÊÕÊý¾Ý×´Ì¬¡¢
extern u8 GPRS_SHOW;







/*
************************************************************************************
*	                              	     º¯ÊýÉùÃ÷
************************************************************************************
*/
void uart_init(u32 bound);
void WIFI_USART6_init(u32 bound);			
void TIM7_Int_Init(u16 arr,u16 psc);
void TIM5_Int_Init(u16 arr,u16 psc);
void u6_printf(char* fmt, ...);
void GPRS_UART2_init(u32 bound);
void GPRS_Sendcom(u8 * Gprs_com);
void MESH_UART3_init(u32 bound);
void MESH_Sendcom(u8 * MESH_com,u8 len);
void TIM3_Init(u16 arr,u16 psc);
void TIM3_stop(u16 arr,u16 psc);
void RS485_Receive_Data(u8 *buf);
extern u8 constate;
extern u8 slave_num;

#endif

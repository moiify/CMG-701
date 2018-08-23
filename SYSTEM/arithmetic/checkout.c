/*
*******************************************************************************************

**************
*	                                  
*	模块名称 :功能函数
*	文件名称 : checkout.c
*	说    明 : 校验，判断，置换
*
*******************************************************************************************

**************
*/
#include "checkout.h"

/*
****************************************************************************
*	函 数 名: Get_xor
*	功能说明: XOR校验
*	形    参：Data			校验数据
						len       数据长度
*	返 回 值: 校验值
****************************************************************************
*/
u8 Get_xor(u8 *Data,u8 len)
{
	u8 temp=0x00;
	u8 i=0;
	
	for(;i<len;i++)
	{
		temp^=*(Data+i);
	}
	return temp;
}

/*
****************************************************************************
*	函 数 名: Check_have
*	功能说明: 检查数组是否包含charactor
*	形    参：Data			检查的数组
						len       检查的长度
						charactor 检查的目标
*	返 回 值: 0 没有检查目标
						1	有检查目标
****************************************************************************
*/
u8 Check_have(u8 * Data,u8 len ,u8 charactor)
{
	u8 i=0;
	for(;i<len;i++)
	{
		if(*(Data+i)==charactor)
			return 1;
	}
	return 0;
}

/*
****************************************************************************
*	函 数 名: Charto16
*	功能说明: 字符串转16进制"01 00 "-->0x00 0x01
*	形    参：charactor 转化的目标
						len       转化的长度
						system16  转化后的数据
*	返 回 值: 无
****************************************************************************
*/
void Charto16(u8 * charactor, u8 len, u8 * system16)
{
	u8 i = 0;
	u8 temp1[4];
	
	if (len % 2>0)
		return ;
	if (len == 4)
	{
		temp1[0] = *(charactor + 2);
		temp1[1] = *(charactor + 3);
		temp1[2] = *(charactor + 0);
		temp1[3] = *(charactor + 1);
		for (i = 0; i < 4; i++)
		{
			*(charactor + i) = temp1[i];
		}
	}
	for (i=0; i<(len/2); i ++)
	{
		*(system16 + i) = (*(charactor + i*2) - '0') * 16 + (*(charactor+i*2 + 1) - 

'0');
	}
}

/*
****************************************************************************
*	函 数 名: Exchange
*	功能说明: 字符串转16进制"01 00 "-->0x01 0x00
*	形    参：charactor 转化的目标
						len       转化的长度
						system16  转化后的数据
*	返 回 值: 无
****************************************************************************
*/
void Exchange(u8 * charactor, u8 len, u8 * system16)
{
	u8 i = 0;
	u8 temp1[4];
	u8 temp2[2];
	u8 temp3[16];
	
	if (len == 4)
	{
		temp1[0] = *(charactor + 0);
		temp1[1] = *(charactor + 1);
		temp1[2] = *(charactor + 2);
		temp1[3] = *(charactor + 3);
		for (i = 0; i < 4; i++)
		{
			*(charactor + i) = temp1[i];
		}
	}
	if (len == 2)
	{
		temp2[0] = *(charactor + 0);
		temp2[1] = *(charactor + 1);
		
		for (i = 0; i < 2; i++)
		{
			*(charactor + i) = temp2[i];
		}
	}
	if (len == 16)
	{
		for (i = 0; i < 16; i++)
		{
			temp3[i] = *(charactor + i);
		}
		for (i = 0; i < 16; i++)
		{
			*(charactor + i) = temp3[i];
		}
	}
	
	for (i=0; i<(len/2); i ++)
	{
		*(system16 + i) = (*(charactor + i*2) - '0') * 16 + (*(charactor+i*2 + 1) - 

'0');
	}
}

int str2int(const char* str)  
{  
    int temp = 0;  
    const char* p = str;  
    if(str == NULL) return 0;  
    if(*str == '-' || *str == '+')  
    {  
        str ++;  
    }  
    while( *str != 0)  
    {  
        if( *str < '0' || *str > '9')  
        {  
            break;  
        }  
        temp = temp*10 +(*str -'0');  
        str ++;  
    }  
    if(*p == '-')  
    {  
        temp = -temp;  
    }  
    return temp;  
} 

int strmatch(char *a,char *b)
{
int i,flag=0;
for(i=0;a[i]!='\0'&&b[i]!='\0';)
if(a[i]==b[i])
i++;
else
{
flag=0;
break;
}
if(a[i]=='\0'&&b[i]=='\0')
flag=1;
return flag;
}
float GetFloat(u8 * buf)
{   
	  int a=* buf;
	  int b=*(buf+1);
	  int c=*(buf+2);
	  int d=*(buf+3);
	  int temp =a<< 24 |b<< 16|c<< 8|d;   
	  return *(float*)&temp;
}

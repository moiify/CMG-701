#ifndef _CHECKOUT_H
#define _CHECKOUT_H
#include "sys.h"


u8 Get_xor(u8 *Data,u8 len);
u8 Check_have(u8 * Data,u8 len ,u8 charactor);
void Charto16(u8 * charactor, u8 len, u8 * system16);
void Exchange(u8 * charactor, u8 len, u8 * system16);
int str2int(const char* str);
int strmatch(char *a,char *b);
float GetFloat(u8 * buf);

#endif




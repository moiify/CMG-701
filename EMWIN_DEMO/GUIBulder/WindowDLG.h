#ifndef __WINDOWDLG_H
#define __WINDOWDLG_H
#include "sys.h"
#include "WM.h"
#include "GUI.h"

#ifndef GUI_CONST_STORAGE
#define GUI_CONST_STORAGE const
#endif

extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_set;
extern GUI_CONST_STORAGE GUI_BITMAP bmbtn_control;
extern GUI_CONST_STORAGE GUI_BITMAP bmon;
extern GUI_CONST_STORAGE GUI_BITMAP bmoff;
/*
******************************************************************************************
*	                              	      º¯ÊýÉùÃ÷
******************************************************************************************
*/
WM_HWIN CreateWindow(void);


#endif

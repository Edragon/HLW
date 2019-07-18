#ifndef __STM32_DELAY_H
#define __STM32_DELAY_H 			   
#include "sys.h"

//V1.4修改说明 20110929
//修改了使用ucos,但是ucos未启动的时候,delay_ms中中断无法响应的bug.
//V1.5修改说明 20120902
//在delay_us加入ucos上锁，防止由于ucos打断delay_us的执行，可能导致的延时不准。
////////////////////////////////////////////////////////////////////////////////// 	 
void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif






























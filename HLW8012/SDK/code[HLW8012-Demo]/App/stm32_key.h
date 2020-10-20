
#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY_LEFT                GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)
#define KEY_RIGHT               GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)
#define KEY_UP                  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)
#define KEY_DOWN                GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)
#define KEY_MID                 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)

#define KEY_STATE_NULL          0
#define KEY_STATE_LEFT          1
#define KEY_STATE_RIGHT         2
#define KEY_STATE_UP            3
#define KEY_STATE_DOWN          4
#define KEY_STATE_MID           5

void keyInit(void);
u8 KEY_Scan (u8 mode);


#endif

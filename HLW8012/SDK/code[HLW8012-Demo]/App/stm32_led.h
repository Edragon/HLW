#ifndef __STM32_LED_H
#define __STM32_LED_H

#include "sys.h"

#define LED                                PBout(15)
#define LED_ON                             (LED = 0)
#define LED_OFF                            (LED = 1)

void LED_Init(void);//≥ı ºªØ

#endif

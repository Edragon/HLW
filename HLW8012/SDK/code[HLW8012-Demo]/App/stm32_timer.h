#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x_tim.h"
#include "sys.h"


#define REC_BUF_MAX     2300

#define TIM1_FREQ           72000000
#define TIM1_PSC            ((72000000 / TIM1_FREQ) - 1)

typedef struct _recType {
    u16 cnt[REC_BUF_MAX];
    u16 overCnt[REC_BUF_MAX];
    u16 size;
    u16 currOverCnt;
}recType;

#define SEND_BUF_MAX    252
//Program Size: Code=12348 RO-data=644 RW-data=132 ZI-data=20348  
typedef struct _sendType {
    u8  info[4];
    u32 freq;                           // freq = 38000Hz
    u32 timer_arr;                      // timer_arr = (72000000 + freq / 2) / freq
    u32 time_us[SEND_BUF_MAX];
    u32 cnt;
}sendType;

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
void TIM1_Cap_Init(u16 arr,u16 psc);
#endif

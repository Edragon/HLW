
#include "stm32_key.h"
#include "sys.h" 
#include "stm32_delay.h"

void keyInit (void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

u8 KEY_Scan (u8 mode)
{
    u8 ret = KEY_STATE_NULL;
	static u8 key_up = 1;                       //按键按松开标志
	if (mode) {
        key_up=1;                               //支持连按
	}
    if (key_up) {
        if ((KEY_LEFT == 0) || (KEY_RIGHT == 0) || (KEY_UP == 0) || (KEY_DOWN == 0) || (KEY_MID == 0)) {
            delay_ms(10);                           //去抖动
            key_up=0;
            if (KEY_LEFT == 0) {
                ret = KEY_STATE_LEFT;
            } else if (KEY_RIGHT == 0) {
                ret = KEY_STATE_RIGHT;
            } else if (KEY_UP == 0) {
                ret = KEY_STATE_UP;
            } else if (KEY_DOWN == 0) {
                ret = KEY_STATE_DOWN;
            } else if (KEY_MID == 0) {
                ret = KEY_STATE_MID;
            }
        }
	} else if ((KEY_LEFT == 1) && (KEY_RIGHT == 1) && (KEY_UP == 1) && (KEY_DOWN == 1) && (KEY_MID == 1)) {
        key_up = 1;
    }
 	return ret;
}


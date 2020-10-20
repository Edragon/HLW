#include "stm32_usart.h"
#include "stm32_delay.h"
#include "stm32_led.h"
#include "stm32_key.h"
#include "sys.h"
#include "stm32_oled.h"
#include "stm32_timer.h"

char tempStr[30];
int tempStrLen;
int tempI;

float hlw8012_f;
float hlw8012_P;
float hlw8012_Pp = 103.5 / 21.6;

extern  recType     tRecType;
        u32         countTotal;
        float       hlw8012_f;
        u32         freq_arr;

#define CNT_TO_US(cnt)      (u32)((cnt +         (TIM4_FREQ / 2000000)) / (TIM4_FREQ / 1000000) )   // 获取周期     ((cnt + 36) / 72)   (cnt / 72)          四舍五入    u16 or u32
#define CNT_TO_FREQ(cnt)    (u32)((TIM1_FREQ +   (cnt       / 2      )) / cnt                   )   // 获取频率                         (72000000 / cnt)    四舍五入    u32
#define FREQ_TO_CNT(freq)   (u32)((TIM1_FREQ +   (freq      / 2      )) / freq                  )   // 获取计数值                       (72000000 / freq)   四舍五入    u32
#define US_TO_CNT(us)       (u32)((TIM1_FREQ / 1000000) * us                                    )   // 获取技术值                       (72 * us)                       u32

#define U16_TO_U32(H, L)    (u32)((u32)(H << 16) + L)

unsigned char data_check (recType *pRec, float *modePow);

int main(void)
{
    float Pow_mode;
    delay_init();	        //延时初始化
    NVIC_Configuration();   //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);
    
    keyInit();
    OLED_Init();
    
    LED_Init();
    TIM3_Int_Init(5000, 7200-1);
    
    OLED_Clear();
    //OLED_ShowString(10 * 8,6," KEY  ");
    OLED_ShowCHinese(28 + 0,0,0);
    OLED_ShowCHinese(28 + 16,0,1);
    OLED_ShowCHinese(28 + 32,0,2);
    OLED_ShowCHinese(28 + 48,0,3);
    
    //OLED_ShowString(0,3," www.szlcsc.com");
    //OLED_ShowString(0,6,"0.96'OLED");
    
    TIM1_Cap_Init(65536 - 1, TIM1_PSC);
    
    
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    printf("SYSCLK_Frequency: %d Hz\r\n", RCC_ClocksStatus.SYSCLK_Frequency);
    printf("HCLK_Frequency:   %d Hz\r\n", RCC_ClocksStatus.HCLK_Frequency);
    printf("PCLK1_Frequency:  %d Hz\r\n", RCC_ClocksStatus.PCLK1_Frequency);
    printf("PCLK2_Frequency:  %d Hz\r\n", RCC_ClocksStatus.PCLK2_Frequency);
    
    
	while(1) {
 		
        TIM1->SR = (uint16_t)~TIM_IT_CC1;                       // 清中断
        TIM1->SR = (uint16_t)~TIM_IT_Update;                    // 清中断
        TIM_ITConfig(TIM1, TIM_IT_Update | TIM_IT_CC1, ENABLE);
        TIM_Cmd(TIM1, ENABLE);
        
        // 捕捉超过3秒
        if ((tRecType.currOverCnt > (TIM1_FREQ / 65536) * 3) || (tRecType.size > 10)) {
            TIM_ITConfig(TIM1, TIM_IT_Update | TIM_IT_CC1, DISABLE);
            TIM_Cmd(TIM1, DISABLE);
            
            data_check(&tRecType, &Pow_mode);
            tRecType.size = 0;
            tRecType.currOverCnt = 0;
            
            if (Pow_mode > 5) 
                showPower(1, Pow_mode);
            else 
                showPower(1, 20000);
        }
	}
    
    return 1;
}

//u32 mYverage (u)          递归法：假设前n个数的平均数为avg, 那么前n+1个数的平均数avg = ((avg * n) + X) / (n + 1) = avg - avg / (n + 1) + X / (n + 1) = avg + (X - avg) / (n + 1)
float mYverage (float avg, float X, u32 n)
{
    if (n == 0) {
        return 0;
    } else if (n == 1) {
        return X;
    } else if (n > 1) {
        if (X > avg) {
            return (avg + (X - avg) / n);
        } else {
            return (avg - (avg - X) / n);
        }
    } else {
        return 0;
    }
}

unsigned char data_check (recType *pRec, float *modePow)
{
    unsigned char ret = 1;
    float fAvg;
    u32 u32Temp1, u32Temp2;
    u16 i = 0, j = 0;
    
    if (pRec->size == 0 || pRec->size == 1) {   // 不够1Hz
        *modePow = 0;
        ret = 0;
        goto end;
    }
    
    // 矫正 数据中 溢出中断 与 捕获中断 并发 造成的 溢出数据(overCnt) 顺序混乱的情况
    for (i = 0; i < pRec->size; i++) {
        if (pRec->overCnt[i] == pRec->overCnt[i + 1]) {
            if (pRec->cnt[i] > pRec->cnt[i + 1]) {                      // 后面的数据小于前面的数据--这种情况不应该发生
                pRec->overCnt[i + 1]++;
            }
        }
    }
    
    // 将 数据 由 绝对值 换算为 相对值
    for (i = 0; i < pRec->size - 1; i++) {
        u32Temp1 = U16_TO_U32(pRec->overCnt[i + 0], pRec->cnt[i + 0]);
        u32Temp2 = U16_TO_U32(pRec->overCnt[i + 1], pRec->cnt[i + 1]);
        
        pRec->overCnt[i]    = (u16)((u32Temp2 - u32Temp1) >> 16);
        pRec->cnt[i]        = (u16)((u32Temp2 - u32Temp1) >> 0);
    }
    pRec->size -= 1;                                                    // 数量减一
    
    // 删除 波形中 的高频毛刺
    for (i = 0; i < pRec->size; i++) {
        u32Temp1 = U16_TO_U32(pRec->overCnt[i], pRec->cnt[i]);
        if (CNT_TO_FREQ(u32Temp1) > 100000) {                   // TIM1_FREQ / 100KHz = 720 过滤掉>100KHz的波形
            for (j = i; j < pRec->size - 1; j++) {
                pRec->overCnt[j]    = pRec->overCnt[j + 1];
                pRec->cnt[j]        = pRec->cnt[j + 1];
            }
            pRec->size--;
        }
    }
    
    // 将 高频数据 整合在一起 求平均
    fAvg = 0;
    for (i = 0; i < pRec->size; i++) {
        u32Temp1 = U16_TO_U32(pRec->overCnt[i], pRec->cnt[i]);
        fAvg = mYverage(fAvg, (float)u32Temp1, i + 1);
    }
    *modePow = hlw8012_Pp * CNT_TO_FREQ((u32)fAvg);                     //  计算功率
    
    /********************************打印数据************************************************/
    tempStrLen = sprintf(tempStr,"%f", *modePow);
    txNByte(USART1, tempStr, tempStrLen);
    txStr(USART1, "w\r\n");
    /********************************打印数据************************************************/
end:
    return ret;
}

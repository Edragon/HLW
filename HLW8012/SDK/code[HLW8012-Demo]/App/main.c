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

#define CNT_TO_US(cnt)      (u32)((cnt +         (TIM4_FREQ / 2000000)) / (TIM4_FREQ / 1000000) )   // ��ȡ����     ((cnt + 36) / 72)   (cnt / 72)          ��������    u16 or u32
#define CNT_TO_FREQ(cnt)    (u32)((TIM1_FREQ +   (cnt       / 2      )) / cnt                   )   // ��ȡƵ��                         (72000000 / cnt)    ��������    u32
#define FREQ_TO_CNT(freq)   (u32)((TIM1_FREQ +   (freq      / 2      )) / freq                  )   // ��ȡ����ֵ                       (72000000 / freq)   ��������    u32
#define US_TO_CNT(us)       (u32)((TIM1_FREQ / 1000000) * us                                    )   // ��ȡ����ֵ                       (72 * us)                       u32

#define U16_TO_U32(H, L)    (u32)((u32)(H << 16) + L)

unsigned char data_check (recType *pRec, float *modePow);

int main(void)
{
    float Pow_mode;
    delay_init();	        //��ʱ��ʼ��
    NVIC_Configuration();   //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
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
 		
        TIM1->SR = (uint16_t)~TIM_IT_CC1;                       // ���ж�
        TIM1->SR = (uint16_t)~TIM_IT_Update;                    // ���ж�
        TIM_ITConfig(TIM1, TIM_IT_Update | TIM_IT_CC1, ENABLE);
        TIM_Cmd(TIM1, ENABLE);
        
        // ��׽����3��
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

//u32 mYverage (u)          �ݹ鷨������ǰn������ƽ����Ϊavg, ��ôǰn+1������ƽ����avg = ((avg * n) + X) / (n + 1) = avg - avg / (n + 1) + X / (n + 1) = avg + (X - avg) / (n + 1)
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
    
    if (pRec->size == 0 || pRec->size == 1) {   // ����1Hz
        *modePow = 0;
        ret = 0;
        goto end;
    }
    
    // ���� ������ ����ж� �� �����ж� ���� ��ɵ� �������(overCnt) ˳����ҵ����
    for (i = 0; i < pRec->size; i++) {
        if (pRec->overCnt[i] == pRec->overCnt[i + 1]) {
            if (pRec->cnt[i] > pRec->cnt[i + 1]) {                      // ���������С��ǰ�������--���������Ӧ�÷���
                pRec->overCnt[i + 1]++;
            }
        }
    }
    
    // �� ���� �� ����ֵ ����Ϊ ���ֵ
    for (i = 0; i < pRec->size - 1; i++) {
        u32Temp1 = U16_TO_U32(pRec->overCnt[i + 0], pRec->cnt[i + 0]);
        u32Temp2 = U16_TO_U32(pRec->overCnt[i + 1], pRec->cnt[i + 1]);
        
        pRec->overCnt[i]    = (u16)((u32Temp2 - u32Temp1) >> 16);
        pRec->cnt[i]        = (u16)((u32Temp2 - u32Temp1) >> 0);
    }
    pRec->size -= 1;                                                    // ������һ
    
    // ɾ�� ������ �ĸ�Ƶë��
    for (i = 0; i < pRec->size; i++) {
        u32Temp1 = U16_TO_U32(pRec->overCnt[i], pRec->cnt[i]);
        if (CNT_TO_FREQ(u32Temp1) > 100000) {                   // TIM1_FREQ / 100KHz = 720 ���˵�>100KHz�Ĳ���
            for (j = i; j < pRec->size - 1; j++) {
                pRec->overCnt[j]    = pRec->overCnt[j + 1];
                pRec->cnt[j]        = pRec->cnt[j + 1];
            }
            pRec->size--;
        }
    }
    
    // �� ��Ƶ���� ������һ�� ��ƽ��
    fAvg = 0;
    for (i = 0; i < pRec->size; i++) {
        u32Temp1 = U16_TO_U32(pRec->overCnt[i], pRec->cnt[i]);
        fAvg = mYverage(fAvg, (float)u32Temp1, i + 1);
    }
    *modePow = hlw8012_Pp * CNT_TO_FREQ((u32)fAvg);                     //  ���㹦��
    
    /********************************��ӡ����************************************************/
    tempStrLen = sprintf(tempStr,"%f", *modePow);
    txNByte(USART1, tempStr, tempStrLen);
    txStr(USART1, "w\r\n");
    /********************************��ӡ����************************************************/
end:
    return ret;
}

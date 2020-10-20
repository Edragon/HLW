#include "stm32_timer.h"
#include "stm32_led.h"
#include "stm32_usart.h"
#include "stm32_oled.h"

recType tRecType;					  

//��ʱ��1ͨ��1���벶������
void TIM1_Cap_Init(u16 arr,u16 psc)
{	 
    TIM_ICInitTypeDef  TIM1_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��TIM1ʱ�� ʹ��GPIOAʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 ����  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);						 //PA8 ����
    
	//��ʼ����ʱ��1 TIM1	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
  
	//��ʼ��TIM1���벶�����
	TIM1_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM1_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;	//�����ز���
  	TIM1_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM1_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM1_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM1, &TIM1_ICInitStructure);
    
    /***************************************************ʹ��˫���ؼ��******************************************************/
    // http://www.openedv.com/thread-12644-1-1.html     http://blog.csdn.net/smallmount123/article/details/52401946
//    TIM1->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC1E);
//    TIM1->CCMR1 |= 0x03;
//    TIM1->SMCR |= (1 << 6);
//    TIM1->SMCR &= ~((1 << 5) | (1 << 4));
//    TIM1->CCER |= ((uint16_t)TIM_CCER_CC1E);
    /***************************************************ʹ��˫���ؼ��******************************************************/
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;      //TIM�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);                         //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
    
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;      //TIM�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);                         //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
    tRecType.size = 0;
    tRecType.currOverCnt = 0;
    
    TIM1->SR = (uint16_t)~TIM_IT_CC1;                       // ���ж�
    TIM1->SR = (uint16_t)~TIM_IT_Update;                    // ���ж�
//    
//	TIM_ITConfig(TIM1, TIM_IT_Update | TIM_IT_CC1,ENABLE);  //��������ж� ,����CC1IE�����ж�	
//   	TIM_Cmd(TIM1,ENABLE ); 	                                //ʹ�ܶ�ʱ��
    

//    TIM1->DIER &= (uint16_t)(~((uint16_t)TIM_IT_CC1));
//    TIM1->CR1  &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
//    
//    TIM1->DIER &= (uint16_t)(~((uint16_t)TIM_IT_CC1));
//    TIM1->CR1  &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
}

void data_check (recType *pRecType);
void TIM1_CC_IRQHandler(void)
{
    if (TIM1->SR & TIM_IT_CC1) {
        tRecType.overCnt[tRecType.size]    = tRecType.currOverCnt;
        tRecType.cnt[tRecType.size]        = TIM1->CCR1;
        tRecType.size++;
        
        if (tRecType.size >= REC_BUF_MAX) {
            TIM_ITConfig(TIM1, TIM_IT_Update | TIM_IT_CC1, DISABLE);
            TIM_Cmd(TIM1, DISABLE);
            
            //data_check(&tRecType);
            //ledSetting(&tLed1Para, 50, 50, 0, 0);
            
            tRecType.size = 0;
            tRecType.currOverCnt = 0;
        }
        
        TIM1->SR = (uint16_t)~TIM_IT_CC1;                       //����жϱ�־λ
    }
}

void TIM1_UP_IRQHandler(void)
{
    if (TIM1->SR & TIM_IT_Update) {
        //if (tRecType.size) {
            tRecType.currOverCnt++;
        //}
        TIM1->SR = (uint16_t)~TIM_IT_Update;                    //����жϱ�־λ
    }
}

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
							 
}

    u8 hour = 12, minute = 0, second = 0;
    u8 secondFlag = 0;
//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ 
		LED = !LED;
        
        if (secondFlag) {
            second++;
            if (second == 60) {
                second = 0;
                minute++;
                if (minute == 60) {
                    minute = 0;
                    hour++;
                    if (hour == 24) {
                        hour = 0;
                    }
                }
            }
            //OLED_ShowChar(128 - 8 * 3, 0, ':');
            secondFlag = 0;
        } else {
            //OLED_ShowChar(128 - 8 * 3, 0, ' ');
            secondFlag = 1;
        }
//        OLED_ShowNum(128 - 8 * 5, 0, (hour / 10), 1, 16);
//        OLED_ShowNum(128 - 8 * 4, 0, (hour % 10), 1, 16);
//        
//        OLED_ShowNum(128 - 8 * 2, 0, (minute / 10), 1, 16);
//        OLED_ShowNum(128 - 8 * 1, 0, (minute % 10), 1, 16);
    }
}


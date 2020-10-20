#include "stm32_timer.h"
#include "stm32_led.h"
#include "stm32_usart.h"
#include "stm32_oled.h"

recType tRecType;					  

//定时器1通道1输入捕获配置
void TIM1_Cap_Init(u16 arr,u16 psc)
{	 
    TIM_ICInitTypeDef  TIM1_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);	//使能TIM1时钟 使能GPIOA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);						 //PA8 下拉
    
	//初始化定时器1 TIM1	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM1输入捕获参数
	TIM1_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM1_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;	//上升沿捕获
  	TIM1_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM1_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM1_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM1, &TIM1_ICInitStructure);
    
    /***************************************************使能双边沿检测******************************************************/
    // http://www.openedv.com/thread-12644-1-1.html     http://blog.csdn.net/smallmount123/article/details/52401946
//    TIM1->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC1E);
//    TIM1->CCMR1 |= 0x03;
//    TIM1->SMCR |= (1 << 6);
//    TIM1->SMCR &= ~((1 << 5) | (1 << 4));
//    TIM1->CCER |= ((uint16_t)TIM_CCER_CC1E);
    /***************************************************使能双边沿检测******************************************************/
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;      //TIM中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);                         //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
    
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;      //TIM中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);                         //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
    tRecType.size = 0;
    tRecType.currOverCnt = 0;
    
    TIM1->SR = (uint16_t)~TIM_IT_CC1;                       // 清中断
    TIM1->SR = (uint16_t)~TIM_IT_Update;                    // 清中断
//    
//	TIM_ITConfig(TIM1, TIM_IT_Update | TIM_IT_CC1,ENABLE);  //允许更新中断 ,允许CC1IE捕获中断	
//   	TIM_Cmd(TIM1,ENABLE ); 	                                //使能定时器
    

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
        
        TIM1->SR = (uint16_t)~TIM_IT_CC1;                       //清除中断标志位
    }
}

void TIM1_UP_IRQHandler(void)
{
    if (TIM1->SR & TIM_IT_Update) {
        //if (tRecType.size) {
            tRecType.currOverCnt++;
        //}
        TIM1->SR = (uint16_t)~TIM_IT_Update;                    //清除中断标志位
    }
}

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}

    u8 hour = 12, minute = 0, second = 0;
    u8 secondFlag = 0;
//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
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


#ifndef __STM32_DELAY_H
#define __STM32_DELAY_H 			   
#include "sys.h"

//V1.4�޸�˵�� 20110929
//�޸���ʹ��ucos,����ucosδ������ʱ��,delay_ms���ж��޷���Ӧ��bug.
//V1.5�޸�˵�� 20120902
//��delay_us����ucos��������ֹ����ucos���delay_us��ִ�У����ܵ��µ���ʱ��׼��
////////////////////////////////////////////////////////////////////////////////// 	 
void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif






























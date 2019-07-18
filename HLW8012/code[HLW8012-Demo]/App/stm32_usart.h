#ifndef __STM32_USART_H
#define __STM32_USART_H
#include "stdio.h"	
#include "sys.h" 
	
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
void txByte (USART_TypeDef* USARTx, unsigned char byte);
void txNByte(USART_TypeDef* USARTx, char *str, unsigned int len);
void txStr(USART_TypeDef* USARTx, char *str);
void uart4_init(u32 bound);
#endif


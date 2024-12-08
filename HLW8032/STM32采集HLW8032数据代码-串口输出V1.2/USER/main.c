#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart2.h"

void Data_Processing(void);
u8 k=0;
u16 old_reg=0;

//��ѹϵ���͵���ϵ����ģ��ʹ�õĵ�����ֵ�й�
float Ue=1.88;//ģ��ĵ�ѹϵ��

float Ce=0.333;//10A����ģ��ĵ���ϵ��
//float Ce=1.0;//30A����ģ��ĵ���ϵ��

 int main(void)
 {	
	u8 len;
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	uart_init(115200);	 //����1��ʼ��Ϊ9600
	USART2_Init(4800);   //����2��ʼ��Ϊ4800
	printf("this is uart1\r\n");
	while(1)
	{
		if(USART2_RX_STA&0x8000)
		{					   
			len=USART2_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			if(len==24&&USART2_RX_BUF[1]==0x5a)//�ж������Ƿ�ΪHLW8032����
			{
				Data_Processing();//���ݴ���
			}
			USART2_RX_STA=0;//������ձ�־
		}
	}	 
}
void Data_Processing()
{
	u32 VP_REG=0,V_REG=0,CP_REG=0,C_REG=0,PP_REG=0,P_REG=0,PF_COUNT=0,PF=0;
	double V=0,C=0,P=0,E_con=0;
	if(USART2_RX_BUF[0]!=0xaa)//оƬ�������������������������
	{
		VP_REG=USART2_RX_BUF[2]*65536+USART2_RX_BUF[3]*256+USART2_RX_BUF[4];//�����ѹ�����Ĵ���
		V_REG=USART2_RX_BUF[5]*65536+USART2_RX_BUF[6]*256+USART2_RX_BUF[7];//�����ѹ�Ĵ���
		V=(VP_REG/V_REG)*Ue;//�����ѹֵ��1.88Ϊ��ѹϵ�������������õķ�ѹ�����С��ȷ��
		printf("��ѹֵ��%0.2fV; ",V);
		
		CP_REG=USART2_RX_BUF[8]*65536+USART2_RX_BUF[9]*256+USART2_RX_BUF[10];//������������Ĵ���
		C_REG=USART2_RX_BUF[11]*65536+USART2_RX_BUF[12]*256+USART2_RX_BUF[13];//��������Ĵ���
		C=((CP_REG*Ce*100)/C_REG)/100.0;//�������ֵ
		printf("����ֵ��%0.3fA; ",C);
		//printf(" %X ",USART2_RX_BUF[0]);
		
		if(USART2_RX_BUF[0]>0xf0)//�ж�ʵʱ�����Ƿ�δ���
		{
			printf("δ���õ��豸!");
		}
		else
		{
			PP_REG=USART2_RX_BUF[14]*65536+USART2_RX_BUF[15]*256+USART2_RX_BUF[16];//���㹦�ʲ����Ĵ�
			P_REG=USART2_RX_BUF[17]*65536+USART2_RX_BUF[18]*256+USART2_RX_BUF[19];//���㹦�ʼĴ���
			P=(PP_REG/P_REG)*Ue*Ce;//������Ч����
			printf("��Ч���ʣ�%0.2fW; ",P);
		}
		
		if((USART2_RX_BUF[20]&0x80)!=old_reg)//�ж����ݸ��¼Ĵ������λ��û�з�ת
		{
			k++;
			old_reg=USART2_RX_BUF[20]&0x80;
		}
		PF=(k*65536)+(USART2_RX_BUF[21]*256)+USART2_RX_BUF[22];//�������õ���������
		PF_COUNT=((100000*3600)/(PP_REG*Ue*Ce))*10000;//����1�ȵ��Ӧ����������
		E_con=((PF*10000)/PF_COUNT)/10000.0;//�������õ���
		//printf(" %d %d ",PF,PF_COUNT);
		printf("���õ�����%0.4f��\r\n",E_con);
	}
	else//оƬ�����������ʧЧ
	{
		printf("data error\r\n");
	}
}

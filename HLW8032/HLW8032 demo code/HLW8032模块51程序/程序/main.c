#include "reg52.h"
#include "LCD1602.h"
#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long
	
u8 ReceiveData[24]={0};
u8 DisplayData1[16]={"Voltage:   .  v"};
u8 DisplayData2[16]={"Power :   .   w"};
u16 i=0; 

u32 VoltageParameter = 0;		//��ѹ���� 
float CurrentParameter = 0;		//��������
u32 PowerParameters  = 0;		//���ʲ���

u32 RegisterVoltage  = 0;		//�Ĵ�����ѹ
float RegisterCurrent  = 0;		//�Ĵ�������
u32 RegisterPower    = 0;		//�Ĵ�������

u16 Voltage = 0;		//��ѹ
u32 Current = 0;		//����
u32 Power   = 0;		//����

/*******************************************************************************
* ������         :UsartInit()
* ��������		   :���ô���
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void UsartInit()
{
	SCON=0X50;			//����Ϊ������ʽ1
	TMOD=0X20;			//���ü�����������ʽ2
	//TH1 = 0xFD;			//�����ʣ�9600
	TH1 = 0xFA;		//�����ʣ�4800
	TL1 = TH1;
	ES=1;						//�򿪽����ж�
	EA=1;						//�����ж�
	TR1=1;					//�򿪼�����
}

/*******************************************************************************
* �� �� ��       : main
* ��������		 : ������
* ��    ��       : ��
* ��    ��    	 : ��
*******************************************************************************/
void main()
{	
	UsartInit();  //	���ڳ�ʼ��
	init();
	while(1)
	{
		DisplayData1[8]  = Voltage%100000/10000+0x30;
		DisplayData1[9]  = Voltage%10000/1000+0x30;
		DisplayData1[10] = Voltage%1000/100+0x30;
		DisplayData1[12] = Voltage%100/10+0x30;
		DisplayData1[13] = Voltage%10+0x30;
		DisplayData2[7]  = Voltage%1000000/100000+0x30;
		DisplayData2[8]  = Power%100000/10000+0x30;
		DisplayData2[9]  = Power%10000/1000+0x30;
		DisplayData2[11] = Power%1000/100+0x30;
		DisplayData2[12] = Power%100/10+0x30;
		DisplayData2[13] = Power%10+0x30;
		LCD_Write_String(0,0,DisplayData1);
		LCD_Write_String(0,1,DisplayData2);
	}		
}

/*******************************************************************************
* ������         : Usart() interrupt 4
* ��������		  : ����ͨ���жϺ���
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void Usart() interrupt 4
{
	u8 receiveData;

	receiveData=SBUF;//��ȥ���յ�������
	RI = 0;//��������жϱ�־λ
	ReceiveData[i]=receiveData;
	i++;
	if(i>=24)
	{
		i=0;
		VoltageParameter = ReceiveData[2]*65535+ReceiveData[3]*256+ReceiveData[4];		//��ѹ��������
		CurrentParameter = ReceiveData[8]*65535+ReceiveData[9]*256+ReceiveData[10];		//������������
		PowerParameters  = ReceiveData[14]*65535+ReceiveData[15]*256+ReceiveData[16];	//���ʲ�������
		RegisterVoltage  = ReceiveData[5]*65535+ReceiveData[6]*256+ReceiveData[7];		//�Ĵ�����ѹ����
		RegisterCurrent  = ReceiveData[11]*65535+ReceiveData[12]*256+ReceiveData[13];	//�Ĵ�����ѹ����
		RegisterPower    = ReceiveData[17]*65535+ReceiveData[18]*256+ReceiveData[19];	//�Ĵ�����ѹ����
		Voltage = (u16)(VoltageParameter/RegisterVoltage)*1.88*100;
		Power   = (PowerParameters/RegisterPower)*1.0*1.88*1000;
	}
		
	SBUF=receiveData;//�����յ������ݷ��뵽���ͼĴ���
	while(!TI);			 //�ȴ������������
	TI=0;						 //���������ɱ�־λ
}
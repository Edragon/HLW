#include "reg52.h"
#include "LCD1602.h"
#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long
	
u8 ReceiveData[24]={0};
u8 DisplayData1[16]={"Voltage:   .  v"};
u8 DisplayData2[16]={"Power :   .   w"};
u16 i=0; 

u32 VoltageParameter = 0;		//电压参数 
float CurrentParameter = 0;		//电流参数
u32 PowerParameters  = 0;		//功率参数

u32 RegisterVoltage  = 0;		//寄存器电压
float RegisterCurrent  = 0;		//寄存器电流
u32 RegisterPower    = 0;		//寄存器功率

u16 Voltage = 0;		//电压
u32 Current = 0;		//电流
u32 Power   = 0;		//功率

/*******************************************************************************
* 函数名         :UsartInit()
* 函数功能		   :设置串口
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/
void UsartInit()
{
	SCON=0X50;			//设置为工作方式1
	TMOD=0X20;			//设置计数器工作方式2
	//TH1 = 0xFD;			//波特率：9600
	TH1 = 0xFA;		//波特率：4800
	TL1 = TH1;
	ES=1;						//打开接收中断
	EA=1;						//打开总中断
	TR1=1;					//打开计数器
}

/*******************************************************************************
* 函 数 名       : main
* 函数功能		 : 主函数
* 输    入       : 无
* 输    出    	 : 无
*******************************************************************************/
void main()
{	
	UsartInit();  //	串口初始化
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
* 函数名         : Usart() interrupt 4
* 函数功能		  : 串口通信中断函数
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/
void Usart() interrupt 4
{
	u8 receiveData;

	receiveData=SBUF;//出去接收到的数据
	RI = 0;//清除接收中断标志位
	ReceiveData[i]=receiveData;
	i++;
	if(i>=24)
	{
		i=0;
		VoltageParameter = ReceiveData[2]*65535+ReceiveData[3]*256+ReceiveData[4];		//电压参数计算
		CurrentParameter = ReceiveData[8]*65535+ReceiveData[9]*256+ReceiveData[10];		//电流参数计算
		PowerParameters  = ReceiveData[14]*65535+ReceiveData[15]*256+ReceiveData[16];	//功率参数计算
		RegisterVoltage  = ReceiveData[5]*65535+ReceiveData[6]*256+ReceiveData[7];		//寄存器电压计算
		RegisterCurrent  = ReceiveData[11]*65535+ReceiveData[12]*256+ReceiveData[13];	//寄存器电压计算
		RegisterPower    = ReceiveData[17]*65535+ReceiveData[18]*256+ReceiveData[19];	//寄存器电压计算
		Voltage = (u16)(VoltageParameter/RegisterVoltage)*1.88*100;
		Power   = (PowerParameters/RegisterPower)*1.0*1.88*1000;
	}
		
	SBUF=receiveData;//将接收到的数据放入到发送寄存器
	while(!TI);			 //等待发送数据完成
	TI=0;						 //清除发送完成标志位
}
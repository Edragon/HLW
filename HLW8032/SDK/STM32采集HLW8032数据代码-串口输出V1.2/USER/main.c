#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart2.h"

void Data_Processing(void);
u8 k=0;
u16 old_reg=0;

//电压系数和电流系数和模块使用的电阻阻值有关
float Ue=1.88;//模块的电压系数

float Ce=0.333;//10A量程模块的电流系数
//float Ce=1.0;//30A量程模块的电流系数

 int main(void)
 {	
	u8 len;
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(115200);	 //串口1初始化为9600
	USART2_Init(4800);   //串口2初始化为4800
	printf("this is uart1\r\n");
	while(1)
	{
		if(USART2_RX_STA&0x8000)
		{					   
			len=USART2_RX_STA&0x3fff;//得到此次接收到的数据长度
			if(len==24&&USART2_RX_BUF[1]==0x5a)//判断数据是否为HLW8032数据
			{
				Data_Processing();//数据处理
			}
			USART2_RX_STA=0;//清零接收标志
		}
	}	 
}
void Data_Processing()
{
	u32 VP_REG=0,V_REG=0,CP_REG=0,C_REG=0,PP_REG=0,P_REG=0,PF_COUNT=0,PF=0;
	double V=0,C=0,P=0,E_con=0;
	if(USART2_RX_BUF[0]!=0xaa)//芯片误差修正功能正常，参数正常
	{
		VP_REG=USART2_RX_BUF[2]*65536+USART2_RX_BUF[3]*256+USART2_RX_BUF[4];//计算电压参数寄存器
		V_REG=USART2_RX_BUF[5]*65536+USART2_RX_BUF[6]*256+USART2_RX_BUF[7];//计算电压寄存器
		V=(VP_REG/V_REG)*Ue;//计算电压值，1.88为电压系数，根据所采用的分压电阻大小来确定
		printf("电压值：%0.2fV; ",V);
		
		CP_REG=USART2_RX_BUF[8]*65536+USART2_RX_BUF[9]*256+USART2_RX_BUF[10];//计算电流参数寄存器
		C_REG=USART2_RX_BUF[11]*65536+USART2_RX_BUF[12]*256+USART2_RX_BUF[13];//计算电流寄存器
		C=((CP_REG*Ce*100)/C_REG)/100.0;//计算电流值
		printf("电流值：%0.3fA; ",C);
		//printf(" %X ",USART2_RX_BUF[0]);
		
		if(USART2_RX_BUF[0]>0xf0)//判断实时功率是否未溢出
		{
			printf("未接用电设备!");
		}
		else
		{
			PP_REG=USART2_RX_BUF[14]*65536+USART2_RX_BUF[15]*256+USART2_RX_BUF[16];//计算功率参数寄存
			P_REG=USART2_RX_BUF[17]*65536+USART2_RX_BUF[18]*256+USART2_RX_BUF[19];//计算功率寄存器
			P=(PP_REG/P_REG)*Ue*Ce;//计算有效功率
			printf("有效功率：%0.2fW; ",P);
		}
		
		if((USART2_RX_BUF[20]&0x80)!=old_reg)//判断数据更新寄存器最高位有没有翻转
		{
			k++;
			old_reg=USART2_RX_BUF[20]&0x80;
		}
		PF=(k*65536)+(USART2_RX_BUF[21]*256)+USART2_RX_BUF[22];//计算已用电量脉冲数
		PF_COUNT=((100000*3600)/(PP_REG*Ue*Ce))*10000;//计算1度电对应的脉冲数量
		E_con=((PF*10000)/PF_COUNT)/10000.0;//计算已用电量
		//printf(" %d %d ",PF,PF_COUNT);
		printf("已用电量：%0.4f°\r\n",E_con);
	}
	else//芯片误差修正功能失效
	{
		printf("data error\r\n");
	}
}

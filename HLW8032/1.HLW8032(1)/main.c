/*------------------------------------------------------------------*/
/* --- Knight20        ---------------------------------------------*/
/* --- 淘宝店铺：http://t.cn/A6wYQcoq--------------- ----------------*/
/* --- Mobile: (86)13535549442 -------------------------------------*/


#include "reg51.h"
#include "intrins.h"
#include "stdio.h"
typedef unsigned char BYTE;
typedef unsigned int WORD;

#define FOSC 11059200L      //System frequency
#define BAUD 4800           //UART baudrate

/*Define UART parity mode*/
#define NONE_PARITY     0   //None parity
#define ODD_PARITY      1   //Odd parity
#define EVEN_PARITY     2   //Even parity
#define MARK_PARITY     3   //Mark parity
#define SPACE_PARITY    4   //Space parity

#define PARITYBIT EVEN_PARITY   //Testing even parity

sbit bit9 = P2^2;           //P2.2 show UART data bit9
bit busy;

bit start=0;//用于串口启动接收状态
unsigned char count=0,buf[100],time=0;//用于串口计算，串口数据，及超时

void SendData(BYTE dat);
void SendString(char *s);
void Data_Processing();//数据处理
void delay1ms(void);
//电压系数和电流系数和模块使用的电阻阻值有关
float Ue=1.88;//模块的电压系数

float Ce=0.333;//10A量程模块的电流系数
//float Ce=1.0;//30A量程模块的电流系数
void main()
{
	BYTE i=0;
#if (PARITYBIT == NONE_PARITY)
    SCON = 0x50;            //8-bit variable UART
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xda;            //9-bit variable UART, parity bit initial to 1
#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xd2;            //9-bit variable UART, parity bit initial to 0
#endif

    TMOD = 0x20;            //Set Timer1 as 8-bit auto reload mode
    TH1 = TL1 = -(FOSC/12/32/BAUD); //Set auto-reload vaule
    TR1 = 1;                //Timer1 start run
    ES = 1;                 //Enable UART interrupt
    EA = 1;                 //Open master interrupt switch

    SendString("HLW8032 Text!\r\n");
    while(1)
	{
		if(start)
		{
			time++;
			if(time>5)
			{
				time=0;
				if(buf[1]==0x5a&&count==24)
				{
					Data_Processing();//电量数据处理
					for(i=0;i<count;i++)
						buf[i]=0;
				}
				count=0;
				start=0;
			}
			delay1ms();
		}
	}
}
void Data_Processing()//数据处理
{
    long VP_REG=0,V_REG=0,CP_REG=0,C_REG=0,PP_REG=0,P_REG=0;
	unsigned int PF=0,PF_COUNT=0;
	double V=0,C=0,P=0,E_con=0;
	unsigned char temp[80];
	if(buf[0]!=0xaa)//芯片误差修正功能正常，参数正常
	{
		VP_REG=buf[2]*65536+buf[3]*256+buf[4];//计算电压参数寄存器
		V_REG=buf[5]*65536+buf[6]*256+buf[7];//计算电压寄存器
		V=(VP_REG/V_REG)*Ue;//计算电压值，1.88为电压系数，根据所采用的分压电阻大小来确定
		sprintf(temp,"电压值：%0.2fV; ",V);
		SendString(temp);
		
		CP_REG=buf[8]*65536+buf[9]*256+buf[10];//计算电流参数寄存器
		C_REG=buf[11]*65536+buf[12]*256+buf[13];//计算电流寄存器
		C=((CP_REG*Ce*100)/C_REG)/100.0;//计算电流值

		sprintf(temp,"电流值：%0.3fA; ",C);
		SendString(temp);
		//printf(" %X ",USART2_RX_BUF[0]);
		
		if(buf[0]>0xf0)//判断实时功率是否未溢出
		{
			SendString("No Device!");//功耗过低，未接用电设备
			
		}
		else
		{
			PP_REG=buf[14]*65536+buf[15]*256+buf[16];//计算功率参数寄存
			P_REG=buf[17]*65536+buf[18]*256+buf[19];//计算功率寄存器
			P=(PP_REG/P_REG)*Ue*Ce;//计算有效功率
			sprintf(temp,"有效功率：%0.2fW; ",P);
			SendString(temp);
		}
		

		PF=(buf[21]*256)+buf[22];//计算已用电量脉冲数
		//因为51系列单片机比较难运算超过十万的数据，所以这里不计算用电量，只显示用电脉冲
		//1度电对应的脉冲数量一般大于10万
		//PF_COUNT=((100000*3600)/(PP_REG*Ue*Ce))*10000;//计算1度电对应的脉冲数量
		//E_con=((PF*10000)/PF_COUNT)/10000.0;//计算已用电量
		//printf(" %d %d ",PF,PF_COUNT);

		sprintf(temp,"已用脉冲：%d\r\n",PF);
		SendString(temp);
	}
	else//芯片误差修正功能失效
	{
		SendString("data error\r\n");
	}
}
void delay1ms(void)   //误差 -0.651041666667us
{
    unsigned char a,b;
    for(b=4;b>0;b--)
        for(a=113;a>0;a--);
}
/*----------------------------
UART interrupt service routine
----------------------------*/
void Uart_Isr() interrupt 4
{
    if (RI)
    {
		time=0;
		if(!start) start=1;
        RI = 0;             //Clear receive interrupt flag
        buf[count++] = SBUF;          //P0 show UART data
        bit9 = RB8;         //P2.2 show parity bit
    }
    if (TI)
    {
        TI = 0;             //Clear transmit interrupt flag
        busy = 0;           //Clear transmit busy flag
    }
}

/*----------------------------
Send a byte data to UART
Input: dat (data to be sent)
Output:None
----------------------------*/
void SendData(BYTE dat)
{
    while (busy);           //Wait for the completion of the previous data is sent
    ACC = dat;              //Calculate the even parity bit P (PSW.0)
    if (P)                  //Set the parity bit according to P
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 0;            //Set parity bit to 0
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 1;            //Set parity bit to 1
#endif
    }
    else
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 1;            //Set parity bit to 1
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 0;            //Set parity bit to 0
#endif
    }
    busy = 1;
    SBUF = ACC;             //Send data to UART buffer
}

/*----------------------------
Send a string to UART
Input: s (address of string)
Output:None
----------------------------*/
void SendString(char *s)
{
    while (*s)              //Check the end of the string
    {
        SendData(*s++);     //Send current char and increment string ptr
    }
}


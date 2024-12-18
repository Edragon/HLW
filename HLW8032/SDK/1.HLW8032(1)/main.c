/*------------------------------------------------------------------*/
/* --- Knight20        ---------------------------------------------*/
/* --- �Ա����̣�http://t.cn/A6wYQcoq--------------- ----------------*/
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

bit start=0;//���ڴ�����������״̬
unsigned char count=0,buf[100],time=0;//���ڴ��ڼ��㣬�������ݣ�����ʱ

void SendData(BYTE dat);
void SendString(char *s);
void Data_Processing();//���ݴ���
void delay1ms(void);
//��ѹϵ���͵���ϵ����ģ��ʹ�õĵ�����ֵ�й�
float Ue=1.88;//ģ��ĵ�ѹϵ��

float Ce=0.333;//10A����ģ��ĵ���ϵ��
//float Ce=1.0;//30A����ģ��ĵ���ϵ��
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
					Data_Processing();//�������ݴ���
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
void Data_Processing()//���ݴ���
{
    long VP_REG=0,V_REG=0,CP_REG=0,C_REG=0,PP_REG=0,P_REG=0;
	unsigned int PF=0,PF_COUNT=0;
	double V=0,C=0,P=0,E_con=0;
	unsigned char temp[80];
	if(buf[0]!=0xaa)//оƬ�������������������������
	{
		VP_REG=buf[2]*65536+buf[3]*256+buf[4];//�����ѹ�����Ĵ���
		V_REG=buf[5]*65536+buf[6]*256+buf[7];//�����ѹ�Ĵ���
		V=(VP_REG/V_REG)*Ue;//�����ѹֵ��1.88Ϊ��ѹϵ�������������õķ�ѹ�����С��ȷ��
		sprintf(temp,"��ѹֵ��%0.2fV; ",V);
		SendString(temp);
		
		CP_REG=buf[8]*65536+buf[9]*256+buf[10];//������������Ĵ���
		C_REG=buf[11]*65536+buf[12]*256+buf[13];//��������Ĵ���
		C=((CP_REG*Ce*100)/C_REG)/100.0;//�������ֵ

		sprintf(temp,"����ֵ��%0.3fA; ",C);
		SendString(temp);
		//printf(" %X ",USART2_RX_BUF[0]);
		
		if(buf[0]>0xf0)//�ж�ʵʱ�����Ƿ�δ���
		{
			SendString("No Device!");//���Ĺ��ͣ�δ���õ��豸
			
		}
		else
		{
			PP_REG=buf[14]*65536+buf[15]*256+buf[16];//���㹦�ʲ����Ĵ�
			P_REG=buf[17]*65536+buf[18]*256+buf[19];//���㹦�ʼĴ���
			P=(PP_REG/P_REG)*Ue*Ce;//������Ч����
			sprintf(temp,"��Ч���ʣ�%0.2fW; ",P);
			SendString(temp);
		}
		

		PF=(buf[21]*256)+buf[22];//�������õ���������
		//��Ϊ51ϵ�е�Ƭ���Ƚ������㳬��ʮ������ݣ��������ﲻ�����õ�����ֻ��ʾ�õ�����
		//1�ȵ��Ӧ����������һ�����10��
		//PF_COUNT=((100000*3600)/(PP_REG*Ue*Ce))*10000;//����1�ȵ��Ӧ����������
		//E_con=((PF*10000)/PF_COUNT)/10000.0;//�������õ���
		//printf(" %d %d ",PF,PF_COUNT);

		sprintf(temp,"�������壺%d\r\n",PF);
		SendString(temp);
	}
	else//оƬ�����������ʧЧ
	{
		SendString("data error\r\n");
	}
}
void delay1ms(void)   //��� -0.651041666667us
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


#ifndef _LCD1602_H_
#define _LCD1602_H_

sbit RS = P2^6;   //定义端口 
sbit RW = P2^5;
sbit EN = P2^7;

#define RS_CLR RS=0 
#define RS_SET RS=1

#define RW_CLR RW=0 
#define RW_SET RW=1

#define EN_CLR EN=0
#define EN_SET EN=1

/*------------------------------------------------*/
void DelayUs2x(unsigned char t)
{   
 	while(--t);
}

/*------------------------------------------------
 mS延时函数，含有输入参数 unsigned char t，无返回值
 unsigned char 是定义无符号字符变量，其值的范围是
 0~255 这里使用晶振12M，精确延时请使用汇编
------------------------------------------------*/
void DelayMs(unsigned char t)
{
  	while(t--) 		//大致延时1mS
 	{
     	DelayUs2x(245);
	 	DelayUs2x(245);
 	}
}


void write_com(unsigned char com) //写命令
{
	RS_CLR;
	RW_CLR;
	P0=com;
	DelayMs(5);
	EN_SET;
	DelayMs(5);
	EN_CLR;
}

void write_data(unsigned char date)	 //写一个字符
{
	RS_SET;
	RW_CLR;
	P0=date;
	DelayMs(5);
	EN_SET;
	DelayMs(5);
	EN_CLR;
}

void init()			//初始化
{
	write_com(0x38);
	write_com(0x0c);
	write_com(0x06);
	write_com(0x01);
}

/*------------------------------------------------ 
              写入字符串函数
------------------------------------------------*/
void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s) 
{     
 	if (y == 0) 
 	{     
	 	write_com(0x80 + x);     
 	}
 	else 
 	{     
 		write_com(0xC0 + x);     
 	}        
 	while (*s) 
 	{     
 		write_data( *s);     
 		s ++;     
 	}
}

#endif
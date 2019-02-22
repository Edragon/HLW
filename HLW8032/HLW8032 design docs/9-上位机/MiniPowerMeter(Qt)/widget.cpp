#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    SerialPort=new QSerialPort(this);
    connect(SerialPort,SIGNAL(readyRead()),this,SLOT(serialPortreadyRead()));
    SerialPort->setPortName("COM28");
    if (SerialPort->open(QIODevice::ReadWrite))
    {
        qDebug()<<"open ok";
        SerialPort->setBaudRate(4800);
        SerialPort->setDataBits(QSerialPort::Data8);
        SerialPort->setParity(QSerialPort::NoParity);
        SerialPort->setStopBits(QSerialPort::OneStop);
        SerialPort->setFlowControl(QSerialPort::NoFlowControl);

    }
    else
    {
        //QMessageBox::information(this,"Error","Open Serial Port fail");
        qDebug()<<SerialPort->error();
    }
}

Widget::~Widget()
{
    delete ui;
}

static float Vk=1.88;
static float Ik=0.25;
static uint32_t VParam=0;
static uint32_t VReg=0;
static float V=0;
static uint32_t IParam=0;
static uint32_t IReg=0;
static float I=0;

static uint32_t PParam=0;
static uint32_t PReg=0;
static float P=0;
static float PV=0;
static float PF=0;
static float PBuf[20];
static float PMin=0;
static uint8_t Windex=0;

void Widget::WLW8032_DecodProtocol(uint8_t Data)
{
    static uint8_t status=0;
    static uint8_t index=0;
    static uint8_t sum=0;
    static uint8_t tempBuf[24];
    static uint8_t i;
    switch (status)
    {
    case 0:
        if(Data==0x55)
        {
            tempBuf[0]=Data;
            status=1;
        }
        break;
    case 1:
        if(Data==0x5A)
        {
            tempBuf[1]=Data;
            status=2;
        }
        else status=0;
        index=2;
        sum=0;
        break;
    case 2:

        tempBuf[index]=Data;
        index++;
        if(index==24)
        {
            if(sum==Data)
            {
                //Tools_Printf("check sum OK\r\n");
                if(tempBuf[20]&(0x03<<4))
                {
                    VParam=(tempBuf[2]<<24)|(tempBuf[3]<<16)|tempBuf[4];
                    VReg=(tempBuf[5]<<24)|(tempBuf[6]<<16)|tempBuf[7];
                    IParam=(tempBuf[8]<<24)|(tempBuf[9]<<16)|tempBuf[10];
                    IReg=(tempBuf[11]<<24)|(tempBuf[12]<<16)|tempBuf[13];
                    PParam=(tempBuf[14]<<24)|(tempBuf[15]<<16)|tempBuf[16];
                    PReg=(tempBuf[17]<<24)|(tempBuf[18]<<16)|tempBuf[19];

                    V=(float)VParam/(float)VReg*Vk;
                    I=(float)IParam/(float)IReg*Ik;
                    P=(float)PParam/(float)PReg*Vk*Ik;
                   /* PBuf[Windex]=P;
                    Windex++;
                    if(Windex==20)Windex=0;
                    if(I<0.08)
                    {
                        //取最小值
                        PMin=P;
                        for(i=0;i<20;i++)
                        {
                            if(PBuf[i]<PMin)PMin=PBuf[i];
                        }
                        P=PMin;
                    }*/
                    PV=V*I;
                    PF=P/PV;
                    //Tools_Printf("V:%f\tI:%f\tP:%f\tPV:%f\r\n",V,I,P,PV);
                    qDebug()<<"V:"<<V<<"I:"<<I<<"P:"<<P<<"PV:"<<PV<<"PF:"<<PF;
                }
            }
            //else Tools_Printf("check sum Fail\r\n");
            status=0;
        }
        else
        {
            sum+=Data;
        }
        break;
    default:
        break;
    }
}
void Widget::serialPortreadyRead()
{
    QByteArray revArray;
    uint16_t i;
    bool ret;
    if(SerialPort->isOpen())
    {
        revArray=SerialPort->readAll();
        //qDebug()<<revArray;
        for(i=0;i<revArray.length();i++)
        {
            WLW8032_DecodProtocol(revArray.at(i));
            //ret=MCP_ReceiveByte(revArray.at(i));
            //if(!ret)qDebug()<<"push fail";
        }
    }
}

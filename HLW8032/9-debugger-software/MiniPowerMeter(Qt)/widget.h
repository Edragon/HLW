#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    QSerialPort *SerialPort;
    void WLW8032_DecodProtocol(uint8_t Data);
private slots:
    void serialPortreadyRead();
private:
    Ui::Widget *ui;
};

#endif // WIDGET_H

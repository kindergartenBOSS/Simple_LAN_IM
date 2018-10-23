#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <QHostInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>


Widget::Widget(QWidget *parent, QString username) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //创建UDP套接字并进行初始化，端口默认设为23232，使用connect语句将其与processPendingDatagrams()槽函数绑定，随时接受来自其他用户的UDP广播消息。
    uName = username;
    udpSocket = new QUdpSocket(this);
    port = 23232;
    udpSocket->bind(port,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    sendMsg(NewUser);//使用这个函数来广播用户登录信息（发送各种UDP数据）
}


void Widget::sendMsg(MsgType type, QString srvaddr)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);               //
    QString localHostName = QHostInfo::localHostName();         //获取主机名
    QString address = getIP();                                  //获取IP地址
    out << type << getUser() << localHostName;                                    //

    switch (type)
    {
    case Msg:

        break;
    case NewUser:

        break;
    case UserLeft:

        break;
    case FileName:

        break;
    case Refuse:

        break;
    }

    udpSocket->writeDatagram(data, data.length(), QHostAddress::Broadcast, post);
}



Widget::~Widget()
{
    delete ui;
}

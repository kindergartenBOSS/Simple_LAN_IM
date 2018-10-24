#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <QHostInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>


Widget::Widget(QWidget *parent/*, QString username*/) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //创建UDP套接字并进行初始化，端口默认设为23232，使用connect语句将其与processPendingDatagrams()槽函数绑定，随时接受来自其他用户的UDP广播消息。
    //uName = username;
    udpSocket = new QUdpSocket(this);
    port = 23232;
    udpSocket->bind(port,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    sendMsg(NewUser);//使用这个函数来广播用户登录信息（发送各种UDP数据）
}

//发送UDP广播
void Widget::sendMsg(MsgType type, QString srvaddr)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);               //
    QString localHostName = QHostInfo::localHostName();         //获取主机名
    QString address = getIP();                                  //获取IP地址
    out << type << getUser() << localHostName;                  //向要发送的数据中写入信息类型type、用户名（getUser（）获取）、主机名；
                                                                //type用于接收端区分信息类型，从而可以对不同类型的信息进行不同的处理

    switch (type)
    {
    //对于不同的聊天信息Msg，先判断发送的消息是否为空，若空则警告；然后向发送的数据中写入本机的IP地址和用户输入的消息（getMsg（）获取）
    case Msg:
        if (ui->messageTxtEdit->toPlainText() == "")
        {
            QMessageBox::warning(0, tr("警告"), tr("发送内容不能为空"), QMessageBox::Ok);
            return;
        }
        out << address << getMsg();
        ui->messageBrowser->verticalScrollBar()->setValue(ui->messageBrowser->verticalScrollBar()->maximum());
        break;

    //新用户加入，获取后，向数据中写入IP地址
    case NewUser:
        out << address;
        break;

    //用户离开，不需要任何操作
    case UserLeft:
        break;

    //发送文件名
    case FileName:
        break;

    //拒绝接收文件
    case Refuse:
        break;
    }

    udpSocket->writeDatagram(data, data.length(), QHostAddress::Broadcast, port);
}

//接收UDP消息
void Widget::processPendingDatagrams()
{
    //首先调用QUdpSocket类的成员函数hasPendingDatagrams()来判断是否有可以读取的数据
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        //如果有可以读取的数据，则通过pendingDatagramSize()获取UDP报文大小，并根据大小分配接收缓冲区datagram
        datagram.resize(udpSocket->pendingDatagramSize());
        //使用QUdpSocket类的成员函数readDatagram()读取相应数据
        udpSocket->readDatagram(datagram.data(), datagram.size());
        QDataStream in (&datagram, QIODevice::ReadOnly);
        int msgType;
        //获取信息类型，再用switch()对不同类型进行不同的操作
        in >> msgType;
        QString userName,localHostName,ipAddr,msg;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        switch (msgType)
        {
        //若是普通的消息Msg，则获取其中的用户名、主机名、IP地址、消息内容；
        //并将用户名和消息内容显示在界面左上角的信息浏览器messageBrowser中，同时显示系统的时间（利用QDateTime::currentDateTime()获取）
        case Msg:
            in >> userName >> localHostName >> ipAddr >> msg;
            ui->messageBrowser->setTextColor(Qt::blue);
            ui->messageBrowser->setCurrentFont(QFont("Times New Roman",12));
            ui->messageBrowser->append("[" + userName + "]" + time);
            ui->messageBrowser->append(msg);
            break;

        //若是新用户加入，则获取其中的用户名、主机名、IP地址信息，然后使用newUser()函数进行新用户登录处理
        case NewUser:
            in >> userName >> localHostName >> ipAddr;
            newUser(userName,localHostName,ipAddr);
            break;

        //若是用户退出，则获取其中的用户名、主机名，然后使用userLeft()函数进行用户退出处理
        case UserLeft:
            in >> userName >> localHostName;
            userLeft(userName,localHostName,time);
            break;

        //若是文件传输，则
        case FileName:
            break;

        //若是拒绝文件接收，则
        case Refuse:
            break;
        }
    }
}

//newUser()函数
void Widget::newUser(QString userName, QString localHostName, QString ipaddr)
{
    //使用主机名来判断用户是否已经加入用户列表中
    //若没有，则1.向界面右侧的用户列表（userTblWidget）中添加新的用户信息，然后2.在信息浏览器（messageBrowser）中显示新用户加入的信息
    bool isEmpty = ui->userTblWidget->findItems(localHostName, Qt::MatchExactly).isEmpty();
    if (isEmpty)
    {
        QTableWidgetItem *user = new QTableWidgetItem(userName);
        QTableWidgetItem *host = new QTableWidgetItem(localHostName);
        QTableWidgetItem *ip = new QTableWidgetItem(ipaddr);

        //1.
        ui->userTblWidget->insertRow(0);
        ui->userTblWidget->setItem(0,0,user);
        ui->userTblWidget->setItem(0,1,host);
        ui->userTblWidget->setItem(0,2,ip);

        //2.
        ui->messageBrowser->setTextColor(Qt::gray);
        ui->messageBrowser->setCurrentFont(QFont("Times New Roman",10));
        ui->messageBrowser->append(tr("%1 在线").arg(userName));

        ui->userNumLbl->setText(tr("在线人数：%1").arg(ui->userTblWidget->rowCount()));

        //再次调用sendMsg()，告知已经在线的用户加入了新用户
        sendMsg(NewUser);
    }
}

//userLeft()函数
void Widget::userLeft(QString userName, QString localHostName, QString time)
{
    int rowNum = ui->userTblWidget->findItems(localHostName,Qt::MatchExactly).first()->row();

    ui->userTblWidget->removeRow(rowNum);

    ui->messageBrowser->setTextColor(Qt::gray);
    ui->messageBrowser->setCurrentFont(QFont("Times New Roman",10));
    ui->messageBrowser->append(tr("%1于%2离开!").arg(userName).arg(time));

    ui->userNumLbl->setText(tr("在线人数：%1").arg(ui->userTblWidget->rowCount()));
}

//getIP()函数
QString Widget::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress addr, list) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol)
        {
            return addr.toString();
        }
    }
    return 0;
}

//getUser()函数
QString Widget::getUser()
{
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*" << "HOATNAME.*" << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables)
    {
        int index = environment.indexOf(QRegExp(string));
        if(index != -1)
        {
            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2)
            {
                return stringList.at(1);
                break;
            }
        }
    }
    return "unknown";
}

//getMsg()函数
QString Widget::getMsg()
{
    QString msg = ui->messageTxtEdit->toHtml();
    ui->messageTxtEdit->clear();
    ui->messageTxtEdit->setFocus();
    return msg;
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_sendBtn_clicked()
{
    sendMsg(Msg);
}

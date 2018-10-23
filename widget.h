#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class QUdpSocket;

namespace Ui {
class Widget;
}

//枚举变量标志信息类型，分别为、聊天信息、新用户加入、用户退出、文件名、拒绝接收文件
enum MsgType{Msg, NewUser, UserLeft, FileName, Refuse};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();  

protected:
    void newUser(QString username, QString ipaddr);     //处理新用户加入
    void userLeft(QString username, QString time);      //处理用户离开
    void sendMsg(MsgType type, QString srvaddr="");     //广播UDP消息

    QString getIP();                                    //获取IP地址
    QString getUser();                                  //获取用户名
    QString getMsg();                                   //获取聊天消息


private:
    Ui::Widget *ui;
    QUdpSocket *udpSocket;
    qint16 port;
    QString uName;//


private slots:
    void processPendingDatagrams();                     //接收UDP消息
};

#endif // WIDGET_H

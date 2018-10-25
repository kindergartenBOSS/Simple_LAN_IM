#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QDialog>
#include <QTime>

class QFile;
class QTcpServer;       //在TCP服务器类中，要创建一个发送对话框以供用户选择文件发送，通过这个来实现
class QTcpSocket;

namespace Ui {
class TcpServer;
}

class TcpServer : public QDialog
{
    Q_OBJECT

public:
    explicit TcpServer(QWidget *parent = nullptr);
    ~TcpServer();

    void initSrv();         //初始化服务器
    void refused();         //关闭服务器

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::TcpServer *ui;

    qint16 tPort;
    QTcpServer *tSrv;
    QString fileName;
    QString theFileName;
    QFile *localFile;                           //待发送的文件

    qint64 totalBytes;                          //总共需要发送的字节数
    qint64 bytesWritten;                        //已发送的字节数
    qint64 bytesTobeWrite;                      //待发送的字节数
    qint64 payloadsize;                         //被初始化为一个常量

    QByteArray outBlock;                        //缓存一次发送的数据

    QTcpSocket *clientConnection;               //客户端连接的套接字

    QTime time;

private slots:
    void sendMsg();                             //发送数据
    void updateClientProgress(qint64 numBytes);      //更新进度条

    void on_openFileBtn_clicked();

    void on_sendFileBtn_clicked();

    void on_closeFileBtn_clicked();

signals:
    void sendFileName(QString fileName);
};

#endif // TCPSERVER_H

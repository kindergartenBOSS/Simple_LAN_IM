#include "client.h"
#include "ui_client.h"

#include <QTcpSocket>
#include <QDebug>
#include <QMessageBox>


Client::Client(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);

    setFixedSize(350,180);

    totalBytes = 0;
    bytesReceived = 0;
    fileNameSize = 0;

    tClient = new QTcpSocket(this);
    tPort = 6666;
    connect(tClient, SIGNAL(readyRead()), this, SLOT(readMsg()));
    connect(tClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

//输出错误信息
void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError :
        break;
    default:
        qDebug() << tClient->errorString();
        break;
    }
}

//与服务器连接
void Client::newConnection()
{
    blockSize = 0;
    tClient->abort();
    tClient->connectToHost(hostAddr, tPort);
    time.start();           //建立连接后，开始计时
}

//接收从服务器发送来的数据
void Client::readMsg()
{
    QDataStream in(tClient);
    in.setVersion(QDataStream::Qt_4_7);

    float useTime = time.elapsed();

    if(bytesReceived <= sizeof (qint64) * 2)
    {
        if((tClient->bytesAvailable() >= sizeof (qint64) * 2) && (fileNameSize == 0))
        {
            in >> totalBytes >> fileNameSize;
            bytesReceived += sizeof (qint64) * 2;
        }
        if((tClient->bytesAvailable() >= fileNameSize) && (fileNameSize != 0))
        {
            in >> fileName;
            bytesReceived += fileNameSize;

            if(!localFile->open(QFile::WriteOnly))
            {
                QMessageBox::warning(this, tr("应用程序"), tr("无法读取文件%1：\n%2.").arg(fileName).arg(localFile->errorString()));
                return;
            }
        }
        else
        {
            return;
        }
    }
    if(bytesReceived < totalBytes)
    {
        bytesReceived += tClient->bytesAvailable();
        inBlock = tClient->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }
    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(bytesReceived);

    double speed = bytesReceived / useTime;

    ui->cStatusLabel->setText(tr("已接收%1MB（%2MB/s）\n 共%3MB 已用时：%4秒\n估计剩余时间：%5秒")
                              .arg(bytesReceived / (1024 * 1024))
                              .arg(speed * 1000 / (1024 * 1024), 0, 'f', 2)
                              .arg(totalBytes / (1024 * 1024))
                              .arg(useTime / 1000, 0, 'f', 0)
                              .arg(totalBytes / speed / 1000 - useTime / 1000, 0, 'f', 0));
    if(bytesReceived == totalBytes)
    {
        localFile->close();
        tClient->close();
        ui->cStatusLabel->setText(tr("接收文件%1完毕").arg(fileName));
    }
}






Client::~Client()
{
    delete ui;
}

void Client::on_cCancleBtn_clicked()
{
    tClient->abort();
    if(localFile->isOpen())
        localFile->close();
}

void Client::on_cCloseBtn_clicked()
{
    tClient->abort();
    if(localFile->isOpen())
        localFile->close();
    close();
}

void Client::closeEvent(QCloseEvent *)
{
    on_cCloseBtn_clicked();
}

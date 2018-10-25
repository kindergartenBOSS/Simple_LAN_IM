#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

TcpServer::TcpServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    setFixedSize(350,180);      //将对话框大小设置为350*180

    tPort = 6666;
    tSrv = new QTcpServer(this);
    connect(tSrv, SIGNAL(newConnection()), this, SLOT(sendMsg()));

    initSrv();
}

//服务器初始化函数initSrv();
void TcpServer::initSrv()
{
    //变量初始化
    payloadsize = 64*1024;
    totalBytes = 0;
    bytesWritten = 0;
    bytesTobeWrite = 0;

    //设置按钮状态
    ui->chooseFileLabel->setText(tr("请选择要传输的文件"));
    ui->progressBar->reset();
    ui->openFileBtn->setEnabled(true);
    ui->sendFileBtn->setEnabled(false);

    //关闭服务器
    tSrv->close();
}

//发送数据
void TcpServer::sendMsg()
{
    ui->sendFileBtn->setEnabled(false);
    clientConnection = tSrv->nextPendingConnection();
    connect(clientConnection, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));

    ui->chooseFileLabel->setText(tr("开始传输文件 %1 ！").arg(theFileName));

    localFile = new QFile(fileName);
    if (!localFile->open((QFile::ReadOnly)))                //以只读方式打开选中的文件
    {
        QMessageBox::warning(this, tr("应用程序"),tr("无法读取文件 %1:\n%2").arg(fileName).arg(localFile->errorString()));
        return;
    }
    totalBytes = localFile->size();                         //通过QFile类的size()函数获取待发送文件的大小，并存在totalBytes变量中
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);   //将发送缓冲区outBlock封装在一个QDataStream类型的变量中，这样做可以方便的通过重载的“<<”操作符填写文件头结构
    sendOut.setVersion(QDataStream::Qt_4_7);
    time.start();       //开始计时，用来统计传输所用的时间
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);  //通过QString类的right()函数去掉文件的路径部分，仅将文件部分保存在currentFile变量中
    sendOut << qint64(0) << qint64(0) << currentFile;       //构造一个临时的文件头，将该值追加到totalBytes字段，从而完成实际需发送的字节数
    totalBytes += outBlock.size();
    sendOut.device()->seek(0);      //将读写操作指向从头开始
    sendOut << totalBytes << qint64((outBlock.size() - sizeof (qint64) * 2));   //填写实际的总长度和文件长度
    bytesTobeWrite = totalBytes - clientConnection->write(outBlock);
    outBlock.resize(0);             //清空发送缓存区以备下次使用
}

//更新进度条
void TcpServer::updateClientProgress(qint64 numBytes)
{
    qApp->processEvents();                  //用于在传输较大文件时不会被冻结
    bytesWritten += (int)numBytes;
    if(bytesTobeWrite > 0)
    {
        outBlock = localFile->read(qMin(bytesTobeWrite, payloadsize));
        bytesTobeWrite -= (int)clientConnection->write(outBlock);
        outBlock.resize(0);
    }
    else
    {
        localFile->close();
    }

    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(bytesWritten);

    float useTime = time.elapsed();         //获取耗费时间（从time.start()开始时计时），用于计算传输速度等信息
    double speed = bytesWritten / useTime;
    ui->chooseFileLabel->setText(tr("已发送 %1 MB（%2 MB/s）""\n共%3MB 已用时： %4 秒\n估计剩余时间：%5秒")
                                 .arg(bytesWritten / (1024 * 1024))
                                 .arg(speed * 1000 / (1024 * 1024), 0, 'f', 2)
                                 .arg(totalBytes / (1024 * 1024))
                                 .arg(useTime / 1000, 0, 'f', 0)
                                 .arg(totalBytes / speed / 1000 - useTime / 1000, 0, 'f', 0));
    if(bytesWritten == totalBytes)
    {
        localFile->close();
        tSrv->close();
        ui->chooseFileLabel->setText(tr("传输文件%1成功").arg(theFileName));
    }
}





TcpServer::~TcpServer()
{
    delete ui;
}

//打开文件按钮
void TcpServer::on_openFileBtn_clicked()
{
    fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty())
    {
        theFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
        ui->chooseFileLabel->setText(tr("要传输的文件是：%1").arg(theFileName));
        ui->sendFileBtn->setEnabled(true);
        ui->openFileBtn->setEnabled(false);
    }
}

//发送文件按钮
void TcpServer::on_sendFileBtn_clicked()
{
    if(!tSrv->listen(QHostAddress::Any, tPort))         //开始监听
    {
        qDebug() << tSrv->errorString();
        close();
        return;
    }
    ui->chooseFileLabel->setText(tr("等待对方接收ing..."));
    emit sendFileName(theFileName);
}

//关闭按钮
void TcpServer::on_closeFileBtn_clicked()
{
    if(tSrv->isListening())
    {
        tSrv->close();
        if(localFile->isOpen())
        {
            localFile->close();
        }
        clientConnection->abort();
    }
    close();
}

//
void TcpServer::closeEvent(QCloseEvent *)
{
    on_closeFileBtn_clicked();
}

//拒绝接收
void TcpServer::refused()
{
    tSrv->close();
    ui->chooseFileLabel->setText(tr("对方拒绝接收！"));
}

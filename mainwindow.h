#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QStringList>
#include <QBuffer>
#include <QFile>
#include <QTime>
#include <QThread>
#include <QMap>
#include <QtMath>
#include <QMetaType>
#include "win_qextserialport.h"
#include "win_mysql_query.h"
//#include "com_send_thread.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    /**************************************************/
    /*数据库监测线程*/
    QThread* _TimerThread;
    QTimer* _WriteTimer;
    /**************************************************/

//    unsigned short crc16(QByteArray data);//CRC校验
    int crc16(QByteArray data);//CRC校验


    QStringList split_string(QString str);

    bool save_com_pic(QString pic_name, QByteArray byteArray);
    bool send_com_msg(QString jsonstr);
    bool send_com_err(QString jsonstr);

private:
    Ui::MainWindow *ui;
    Win_QextSerialPort *com; //声明串口对象
    Win_Mysql_Query query;
//    Com_Send_Thread *send_thread;

    char queue[3072];
    int queue_len;   //循环队列 默认1024
    int head;          //队列头索引 默认-1
    int tail;          //队列尾索引 默认-1
    char pck_pre;    //校验符首字符
    char pck_head;   //包尾校验符尾字符
    char pck_tail;   //包尾校验符尾字符
    QString crc_str;     //crc校验结果
    int cmd_no;         //命令字
    int pck_size;   //包字符大小
    int data_segs;  //数据总段数
    int cur_no;   //当前数据段编号
    bool is_trans;   //传输状态:false,为传输;true,传输中
    int num_reached;   //送达包编号
    bool push_reached;  //消息推送到达标志(默认false)
//    int trans_state;         //0,空闲;>0,传输中
    QString query_type;      //查询类型
    int info_segs;           //客户数据分段数
    QString results;         //数据库返回结果
    int cur_seg;             //返回信息中的段数
QTimer *readTimer;

    QString pic_name;
    QByteArray pic_data;
    QByteArray data;
//    QByteArray dtp;
    QByteArray info;

    QMap<QString, QString> info_map;
signals:
    void resultsChanged(int);

    /**************************************************/
    void WriteTimestop();
    /**************************************************/

//    void change_var_fr_main(int num, int state);

private slots:
    void _onWriteTimeout(); //监测线程槽函数

    void readCom();   //声明串口槽函数
    void send_com_info(int seg_no);
    void on_clearDataBtn_clicked();
    void on_openComBtn_clicked();
    void on_closeComBtn_clicked();
    void on_sendMsgBtn_clicked();
//    void change_var_fr_thread(int num, int state);

    void setRecTextFouc();//滚动条自动滚动到最底部


};

#endif // MAINWINDOW_H

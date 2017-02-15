#ifndef COMSENDTHREAD_H
#define COM_SEND_THREAD_H
#include <QThread>
#include <QTime>
#include "win_qextserialport.h"

class Com_Send_Thread : public QThread
{
    Q_OBJECT
public:
    Com_Send_Thread();
    int crc16(QByteArray data);
    QStringList split_string(QString str);
protected:
    void run();
private slots:
    void send_com_info(QString results,Win_QextSerialPort* com);
    void change_var_fr_main(int num, int state);
signals:
    void change_var_fr_thread(int num, int state);
private:
    //Win_QextSerialPort *com; //声明串口对象
    char pck_pre;    //校验符首字符
    char pck_head;   //包尾校验符尾字符
    char pck_tail;   //包尾校验符尾字符
    QString crc_str; //crc校验结果
    int pck_size;   //包字符大小
    int data_segs;  //数据总段数
    int cur_no;     //当前数据段编号
    int info_segs;  //客户数据分段数

    int num_reached;   //送达状态:false,为未送达;true,已送达
    int trans_state;         //0,空闲;>0,传输中
};

#endif // COM_SEND_THREAD_H

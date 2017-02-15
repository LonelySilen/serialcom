#include "com_send_thread.h"
#include <QtDebug>

Com_Send_Thread::Com_Send_Thread()
{
    //com = NULL;
    pck_pre = 0xFF;        //0xFFD8
    pck_head = 0xD8;        //0xFFD8
    pck_tail = 0xD9;        //0xFFD9
    pck_size = 1;   //包字符大小
    data_segs = 0;  //数据总段数
    cur_no = 0;   //当前数据段编号

    trans_state = 1;
    num_reached = 0;
}

void Com_Send_Thread::run()
{
    //触发信号
    //emit send_com_info("QString");
}

//发送串口数据
void Com_Send_Thread::send_com_info(QString results,Win_QextSerialPort* com){

    //QMessageBox::about(this,"Tsignal", "This is a signal/slot sample");
    QStringList datas = split_string(results);
    /*//以ASCII码形式将行编辑框中的数据写入串口
    QTextCodec * codec = QTextCodec::codecForName("GBK"); //建立一个unicode与GBK之间的转换器
    codec->fromUnicode(jsonstr); //unicode转换成gbk*/

    QByteArray temp;

    int retry_times = 0;            //重发次数
    QTime last = QTime::currentTime();
    QTime now;
    int i = 0;
    int crc_cur=0;

    do{
        now=QTime::currentTime();
        if(last.msecsTo(now)>=200){

            if( i<datas.size() && (num_reached == crc_cur)){
                last=QTime::currentTime();
                num_reached = -1;
                QByteArray tmp = QString(datas.at(i)).toUtf8();
                temp.clear();
                temp.append(pck_pre).append(pck_head);
                temp.append(tmp);
                QString cur_nostr = QString("%1%2%3").arg((i+1)/100).arg((i+1)%100/10).arg((i+1)%100%10);
                temp.append(cur_nostr);
                QString total_nostr = QString("%1%2%3").arg((datas.size())/100).arg((datas.size())%100/10).arg((datas.size())%100%10);
                temp.append(total_nostr);
                QString cur_sizestr = QString("%1%2%3").arg((tmp.size())/100).arg((tmp.size())%100/10).arg((tmp.size())%100%10);
                temp.append(cur_sizestr);
                crc_cur = crc16(tmp)%10000;
                QString crc_tmp = QString("%1%2%3%4").arg(crc_cur/1000).arg(crc_cur%1000/100).arg(crc_cur%100/10).arg(crc_cur%10);
                temp.append(crc_tmp);
                temp.append(pck_pre).append(pck_tail);
                com->write(temp);
                i++;
                retry_times = 0;
            }else if(num_reached==-1 && last.secsTo(now)>=5){
                last=QTime::currentTime();
                QByteArray tmp = QString(datas.at(i-1)).toUtf8();
                temp.clear();
                temp.append(pck_pre).append(pck_head);
                temp.append(tmp);
                QString cur_nostr = QString("%1%2%3").arg((i)/100).arg((i)%100/10).arg((i)%100%10);
                temp.append(cur_nostr);
                QString total_nostr = QString("%1%2%3").arg((datas.size())/100).arg((datas.size())%100/10).arg((datas.size())%10);
                temp.append(total_nostr);
                QString cur_sizestr = QString("%1%2%3").arg((tmp.size())/100).arg((tmp.size())%100/10).arg((tmp.size())%10);
                temp.append(cur_sizestr);
                crc_cur = crc16(tmp)%10000;
                QString crc_tmp = QString("%1%2%3%4").arg(crc_cur/1000).arg(crc_cur%1000/100).arg(crc_cur%100/10).arg(crc_cur%10);
                temp.append(crc_tmp);
                temp.append(pck_pre).append(pck_tail);
                com->write(temp);
                retry_times++;
            }
            if(num_reached==crc_cur ){
                num_reached = 0;
                trans_state = 0;
                emit change_var_fr_thread(0,0);
                break;
            }else if(retry_times>2){//重复次数大于10.则放弃本次发送，发送错误消息并退出
                trans_state = 0;
                num_reached = 0;
                emit change_var_fr_thread(0,0);
                break;
                //QString crc_tmp = QString("%1%2%3%4").arg((char)crc_cur/1000).arg((char)crc_cur/100).arg(crc_cur%100/10).arg(crc_cur%100%10);
                //send_com_err(crc_tmp);
                /*this->exit()*/;
            }
        }
    } while ( trans_state==1);

    return;
}

/**
 * 简单crc校验
 *
 */
//unsigned short MainWindow::crc16(QByteArray data)
//{
//    unsigned short crc=0;
//    for(int i =0;i<data.size();i++){
//        crc += data.at(i);
//    }
//    return crc;                    /* Return updated CRC */
//}
int Com_Send_Thread::crc16(QByteArray data)
{
    int crc=0;
    for(int i =0;i<data.size();i++){
        crc += data.at(i);
    }
    return crc;                    /* Return updated CRC */
}
//分割要发送的字符串
QStringList Com_Send_Thread::split_string(QString str)
{
    QStringList data_segs;          //数据list
    int com_size = 200;              //串口默认传输字节数
    int data_len = str.length();//需要传输的数据长度
    //数据切割段数
    int segments = (data_len%com_size)>0 ?(data_len/com_size)+1:(data_len/com_size);

    for(int i=0;i<segments;i++){
        QString temp ;
        if((i+1)*com_size < data_len){
            temp = str.mid(i*com_size,com_size);
        }else{
            temp = str.mid(i*com_size,com_size);
        }
        data_segs.append(temp);
    }
    return data_segs;
}

void Com_Send_Thread::change_var_fr_main(int num, int state){

    trans_state = state;
    num_reached = num;
}

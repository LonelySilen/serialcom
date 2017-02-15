#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /************************************************************************************************************/
    push_reached = false;       //默认定时器推送消息未到达
    // 使用线程,跑定时器
    _TimerThread = new QThread;
    _WriteTimer = new QTimer(this);
    _WriteTimer->setSingleShot(false);       //true时定时器仅启动一次
    // 在moveToThread前先启动定时器,不然不在一个线程里,直接调用start会失败
    _WriteTimer->start(20000);
    _WriteTimer->moveToThread(_TimerThread);
    // 定时器对象和this不在一个线程里面,因此这边指定了连接方式为Qt::DirectConnection,由定时器所在线程直接触发_onVoiceTimeout
    connect(_WriteTimer, SIGNAL(timeout()), this, SLOT(_onWriteTimeout()), Qt::DirectConnection);
    // 连接定时器槽,用来停止定时器
    connect(this, SIGNAL(WriteTimestop()), _WriteTimer, SLOT(stop()));
    _TimerThread->start();
    /************************************************************************************************************/

    is_trans = false;           //传输状态默认无传输
    results = "";
    connect(this, SIGNAL(resultsChanged(int)),SLOT(send_com_info(int)));
    connect(ui->textBrowser,SIGNAL(textChanged()),this,SLOT(setRecTextFouc()));

    ui->closeComBtn->setEnabled(false); //开始“关闭串口”按钮不可用
    ui->sendMsgBtn->setEnabled(false); //开始“发送数据”按钮不可用

    queue_len = 3072;   //循环队列 默认1024
    head = -1;          //队列头索引 默认-1
    tail = -1;          //队列尾索引 默认-1

    com = NULL;
    pck_pre = 0xFF;        //0xFFD8
    pck_head = 0xD8;        //0xFFD8
    pck_tail = 0xD9;        //0xFFD9
    cmd_no = 0;
    pck_size = 1;   //包字符大小
    data_segs = 0;  //数据总段数
    cur_no = 0;   //当前数据段编号

    num_reached = 0;

    /********************自定义读取串口定时器*************************************/
    readTimer = new QTimer(this);
    /*********************************************************************/
}

MainWindow::~MainWindow()
{
    delete ui;

    /*****************************************************/
    emit WriteTimestop();
    _TimerThread->quit();
    _TimerThread->wait();

    delete _WriteTimer;
    delete _TimerThread;
    /*****************************************************/
    delete readTimer;

    if(com)
        delete com;
}

//滚动滚动条到文本框的最底端
void MainWindow::setRecTextFouc()
{
    ui->textBrowser->moveCursor(QTextCursor::End);  //将接收文本框的滚动条滑到最下面
}

//清空接收区数据
void MainWindow::on_clearDataBtn_clicked()
{
    ui->textBrowser->clear();
}

/**
 * 打开串口事件
 *
 */
void MainWindow::on_openComBtn_clicked()
{

    //定义串口对象，并传递参数，在构造函数里对其进行初始化
    QString portName = ui->portNameComboBox->currentText(); //获取串口名
    /*********************************************************************/
    //使用此方式只需要将读取数据函数连接到提供的readyRead函数即可，不需要自己设置定时器，但会出现读包不完整问题
    //com = new Win_QextSerialPort(portName,QextSerialBase::EventDriven);
    //定义串口对象，指定串口名和查询模式，这里使用Polling,此模式需要自己设置定时器查询串口数据
    com = new Win_QextSerialPort(portName,QextSerialBase::Polling);
    /*********************************************************************/

    bool is_open = com ->open(QIODevice::ReadWrite); //打开串口
    if(is_open){
        ui->textBrowser->insertPlainText("打开串口成功\n");
        //设置波特率
        if(ui->baudRateComboBox->currentText()==tr("9600")) //根据组合框内容对串口进行设置
        com->setBaudRate(BAUD9600);
        else if(ui->baudRateComboBox->currentText()==tr("115200"))
        com->setBaudRate(BAUD115200);

        //设置数据位
        if(ui->dataBitsComboBox->currentText()==tr("8"))
        com->setDataBits(DATA_8);
        else if(ui->dataBitsComboBox->currentText()==tr("7"))
        com->setDataBits(DATA_7);

        //设置奇偶校验
        if(ui->parityComboBox->currentText()==tr("无"))
        com->setParity(PAR_NONE);
        else if(ui->parityComboBox->currentText()==tr("奇"))
        com->setParity(PAR_ODD);
        else if(ui->parityComboBox->currentText()==tr("偶"))
        com->setParity(PAR_EVEN);

        //设置停止位
        if(ui->stopBitsComboBox->currentText()==tr("1"))
        com->setStopBits(STOP_1);
        else if(ui->stopBitsComboBox->currentText()==tr("2"))
        com->setStopBits(STOP_2);

        com->setFlowControl(FLOW_OFF); //设置数据流控制，我们使用无数据流控制的默认设置
        com->setTimeout(10); //设置延时
        /* PC机向ARM通过串口发送信息时，会有一个“中间存储器”----即串口缓冲区。setTimeout()作用是定时读取数据到缓冲区。
         * setTimeout(ms)里面大小参数设置原则：越小越好，不过如果pc机一次性发送的数据比较庞大，应该增大参数大小。
         */
        /*********************************************************************/
//        connect(com,SIGNAL(readyRead()),this,SLOT(readCom()));
        readTimer->start(160);
        //设置延时为100ms
        connect(readTimer,SIGNAL(timeout()),this,SLOT(readCom()));
        /*********************************************************************/

        //信号和槽函数关联，当串口缓冲区有数据时，进行读串口操作
        ui->openComBtn->setEnabled(false); //打开串口后“打开串口”按钮不可用
        ui->closeComBtn->setEnabled(true); //打开串口后“关闭串口”按钮可用
        ui->sendMsgBtn->setEnabled(true); //打开串口后“发送数据”按钮可用
        ui->baudRateComboBox->setEnabled(false); //设置各个组合框不可用
        ui->dataBitsComboBox->setEnabled(false);
        ui->parityComboBox->setEnabled(false);
        ui->stopBitsComboBox->setEnabled(false);
        ui->portNameComboBox->setEnabled(false);
    }else{
        ui->textBrowser->insertPlainText("打开串口失败\n");
    }
}

/**
 * 关闭串口事件
 *
 */
void MainWindow::on_closeComBtn_clicked()
{
    head = tail = -1;
    /*********************************************************************/
    readTimer->stop();
    disconnect(readTimer); //断开readTimer的所有连接
    /*********************************************************************/
    com->close();
    ui->openComBtn->setEnabled(true); //关闭串口后“打开串口”按钮可用
    ui->closeComBtn->setEnabled(false); //关闭串口后“关闭串口”按钮不可用
    ui->sendMsgBtn->setEnabled(false); //关闭串口后“发送数据”按钮不可用
    ui->baudRateComboBox->setEnabled(true); //设置各个组合框可用
    ui->dataBitsComboBox->setEnabled(true);
    ui->parityComboBox->setEnabled(true);
    ui->stopBitsComboBox->setEnabled(true);
    ui->portNameComboBox->setEnabled(true);
    ui->textBrowser->clear();
}

/**
 * 消息发送按钮点击事件
 *
 */
void MainWindow::on_sendMsgBtn_clicked()
{

    /*QFile file("1.txt");
    if(!file.open(QIODevice::WriteOnly  | QIODevice::Text|QIODevice::Append))
    {
        QMessageBox::warning(this,"sdf","can't open",QMessageBox::Yes);
    }
    QTextStream in(&file);
    in<<jsonstr<<"\n";
    file.close();*/

    _onWriteTimeout();
    ui->sendMsgLineEdit->clear();

    //隐藏窗体
    //this->hide();

    //QMessageBox::warning(this,"消息","发送数据",QMessageBox::Yes);
}

/**
 * 读串口数据
 *
 */
void MainWindow::readCom()
{

        QByteArray cache = com->readAll();      //读入串口数据

        if(cache.size()<=0){                    //串口数据为空则返回
                return;
        }
        is_trans = true;

        //ui->textBrowser->insertPlainText(cache.toHex()+"\n");    //显示到窗口

        while(cache.size()>0){                      //将串口数据顺序存入循环队列
            if((head+1)%queue_len!=tail){           //循环队列没有溢出
                if(head!=queue_len-1){
                    ++head;
                    queue[head] = cache.at(0);
                    cache = cache.mid(1,cache.size()-1);
                }else{
                    head = 0;
                    queue[head] = cache.at(0);
                    cache = cache.mid(1,cache.size()-1);
                }
            }else{
                break;      //如果队列溢出则跳出
            }
        }

        int index = (tail+1)%queue_len;              //循环队列索引
        int begin = 0;                               //数据头索引
        int end = 0;                                 //数据尾索引
        bool pck_full = false;                       //是否查找到完整包

        while(index!=head){                          //遍历完已探测到的包数据

            if(queue[index]==pck_pre && queue[(index+1)%queue_len]==pck_head){
                begin = index;
                pck_full = false;
            }
            else if(queue[index]==pck_pre && queue[(index+1)%queue_len]==pck_tail){
                tail = (index+1)%queue_len;
                end = (index+1)%queue_len;

                if(queue[index+2]!=pck_pre || queue[(index+3)%queue_len]!=pck_head){

                    pck_full = true;
                    break;          //不使用break则只获取最后的包头，对多包数据丢弃,但会出问题
                }
            }
            index = (index+1)%queue_len;

        }


        QByteArray temp;
        if(pck_full){                           //将队列中包数据按照数据索引存入到数据字节数组中
            while(begin!=end){
                temp.append(queue[begin]);
                begin = (begin+1)%queue_len;    //将一个完整包出去最后一个字节全部存入temp
            }
        }else{
            return;
        }
        data = temp.mid(2,temp.size()-3);       //将数据字节数组包头包尾剔除

        //ui->textBrowser->insertPlainText(data.toHex());    //显示到窗口
        //ui->textBrowser->insertPlainText("\n");    //显示到窗口


    crc_str = QString(data.mid(data.size()-4,4));   //校验crc长度
    data.chop(4);                                   //去除4位crc校验位
    int crc_rs = crc16(data.mid(12,data.size()-12))%10000;

    pck_size = QString(data.mid(0,4)).toInt();  //数据长度校验位
    int cmd_num = QString(data.mid(4,2)).toInt();  //命令字
    data = data.mid(6,data.size()-6);              //去掉数据长度校验位


    if(data.size()-6!=pck_size || crc_rs!=crc_str.toInt()){//数据读错了
        send_com_err(QString("").append(crc_str));
        data.clear();
        //is_trans = false;
        ui->textBrowser->insertPlainText("出错了:\n");
        head = -1;          //重置循环队列
        tail = -1;
//        on_closeComBtn_clicked();
//        on_openComBtn_clicked();
        return;
    }else{
        int cur_num = QString(data.mid(0,3)).toInt();    //要接收数据总大小

        int total_size = QString(data.mid(3,3)).toInt(); //数据总包数

        if(cmd_num==51 || cmd_num==51){
            if(cur_no != cur_num-1 && data_segs!=total_size){   //数据包与之前数据包 编号和总包数不一致
                if(cur_num == 1){                     //如果是第一包数据，则默认为新的操作，将原任务废弃，清空数据，初始化参数
                    //data.clear();
                    info.clear();
                    //cur_no = 0;
                    //data_segs = 1;
                }else{                               //否则废弃这次数据
                    return  ;
                }
            }else if(data_segs==total_size && cur_no != cur_num-1){ //数据总包数一致，当前包编号不一致，则废弃本包数据
                send_com_err(QString("").append(crc_str));
                return ;
            }
        }else{
            if(cur_no != cur_num-2 && data_segs!=total_size){   //数据包与之前数据包 编号和总包数不一致
                if(cur_num == 1){                     //如果是第一包数据，则默认为新的操作，将原任务废弃，清空数据，初始化参数
                    //data.clear();
                    info.clear();
                    //cur_no = 0;
                    //data_segs = 1;
                }else{                               //否则废弃这次数据
                    return  ;
                }
            }else if(data_segs==total_size && cur_no != cur_num-2 && cmd_num!=7){ //数据总包数一致，当前包编号不一致，则废弃本包数据
                if(cur_num == 1){
                    info.clear();
                    //cur_no = 0;
                    //data_segs = 1;
                }else{
                    send_com_err(QString("").append(crc_str));
                    return ;
                }
            }else if(cur_no!=cur_num-2 && data_segs==total_size && cmd_num==7){   //数据总包数一致，当前包编号一致，且命令字为7，认为图片包数据重复发送，则废弃本包数据
                if(cur_num==1){
                    pic_data.clear();
                    //cur_no = 0;
                    //data_segs = 1;
                }else{
                    send_com_msg(QString("").append(crc_str));
                    return;
                }

            }
        }

        data = data.mid(6,data.size()-6);              //去除参数数据


        info.append(data);                             //将数据放入info,清除data
        data.clear();
        //ui->textBrowser->insertPlainText("receive:"+info.toHex()+"\n");


        if(cur_num != total_size){                      //当前包编号不等于总包数
            send_com_msg(QString("").append(crc_str));  //发送消息反馈收到数据
            cur_no = cur_num-1;                         //更新包段数和总包数
            data_segs = total_size;
            if(cmd_num!=7){
                return;
            }
        }else{                                        //初始化报段数和总包数
            cur_no = 0;
            data_segs = 0;
        }


    }

    /*QTextCodec * codec = QTextCodec::codecForName("GBK"); //建立一个unicode与GBK之间的转换器
    QString str_frm_gbk = codec->toUnicode(temp); //unicode转换成gbk*/

        if(cmd_num == 2){                       //用户信息查询
            query_type = "info";
            cmd_no = cmd_num;
            //if(rec_infos.size()==5){
                results = query.info_query();
                cur_seg = 0;
                emit resultsChanged(cur_seg);
            //}else{
            //    send_com_err("err");
            //}
        }else if(cmd_num == 1){                //用户登录
            query_type = "login";
            cmd_no = cmd_num;
            QStringList rec_infos = QString(info).split("~");
            if(rec_infos.size()==2){
                results = query.login_query(rec_infos.at(0),rec_infos.at(1));
                cur_seg = 0;
                emit resultsChanged(cur_seg);
            }else{
                send_com_err("err");
            }
        }else if(cmd_num == 8){                //股道查询
            query_type = "track_query";
            cmd_no = cmd_num;
            //QStringList rec_infos = QString(info).split("~");
            //if(rec_infos.size()==2){
                results = query.track_query();
                cur_seg = 0;
                emit resultsChanged(cur_seg);
            //}else{
                //send_com_err("err");
            //}
        }else if(cmd_num == 5){                //设备安设
            query_type = "erection";
            cmd_no = cmd_num;
            QStringList rec_infos = QString(info).split("~");qDebug()<<QString(info);
            if(rec_infos.size()==4){
                results = query.erection_query(rec_infos.at(0),rec_infos.at(1),rec_infos.at(2),QString(rec_infos.at(0)).append("0"),rec_infos.at(3));
                cur_seg = 0;
                emit resultsChanged(cur_seg);
            }else{
                send_com_err("err");
            }
        }else if(cmd_num == 6){                //设备撤除
            query_type = "removal";
            cmd_no = cmd_num;
            QStringList rec_infos = QString(info).split("~");
            if(rec_infos.size()==4){
                results = query.removal_query(rec_infos.at(0),rec_infos.at(1),rec_infos.at(2),QString(rec_infos.at(0)).append("1"),rec_infos.at(3));
                cur_seg = 0;
                emit resultsChanged(cur_seg);
            }else{
                send_com_err("err");
            }
        }else if(cmd_num == 3){                //设备出库
            query_type = "delivery";
            cmd_no = cmd_num;
            QStringList rec_infos = QString(info).split("~");
            if(rec_infos.size()==2){
                results = query.delivery_query(rec_infos.at(0),rec_infos.at(1));
                cur_seg = 0;
                emit resultsChanged(cur_seg);
            }else{
                send_com_err("err");
            }
        }else if(cmd_num == 4){                //设备入库
            query_type = "inquiry";
            cmd_no = cmd_num;
            QStringList rec_infos = QString(info).split("~");
            if(rec_infos.size()==2){
                results = query.inquiry_query(rec_infos.at(0),rec_infos.at(1));
                cur_seg = 0;
                emit resultsChanged(cur_seg);
            }else{
                send_com_err("err");
            }
        }else if(cmd_num == 7){                //上传图片
            query_type = "pic";
            cmd_no = cmd_num;

            if(cur_no==0 && data_segs!=0){
                pic_name = info;               //保存第一包数据为图片名
            }else{
                pic_data.append(info);         //保存包数据为图片
            }
QDateTime time=QDateTime::currentDateTime();//获取当前时间
QString time_str=time.toString("yyyy-MM-dd hh:mm:ss");//格式转换
ui->textBrowser->insertPlainText(time_str+":......cur_no:"+QString("%1").arg(cur_no)+"\n");
            if(data_segs==cur_no){
                //QTime time = QTime::currentTime();
                //pic_name = time.toString("hh-mm-ss");

                bool is_save = save_com_pic(pic_name,pic_data);
                if (is_save){
                    send_com_msg(crc_str);
                }else{
                    send_com_err("err");
                    ui->textBrowser->insertPlainText("保存图片失败\n");
                }
                pic_data.clear();
            }

        }else if(cmd_num == 51 ){                //接收消息
            num_reached = QString(info).toInt();  //校验crc长度


            if(results.size()>0){
                QStringList datas = split_string(results);
                for(int i =0;i<datas.size();i++){
                    int crc_tmp = crc16( QString(datas.at(i)).toUtf8())%10000;
                    if(num_reached == crc_tmp){
                        cur_seg = i+1;
                        break;
                    }
                }
                if(cur_seg==datas.size()){
                    results.clear();
                    cmd_no = 0;
                    is_trans = false;
                }
                else{
                    //继续发送
                    emit resultsChanged(cur_seg);
                }
            }

        }else if(cmd_num == 52){                //接收到错误信息

            if(results.size()>0){
                QStringList datas = split_string(results);
                for(int i =0;i<datas.size();i++){
                    int crc_tmp = crc16( QString(datas.at(i)).toUtf8())%10000;
                    if(num_reached == crc_tmp){
                        cur_seg = i;
                    }
                }
                //继续发送
                emit resultsChanged(cur_seg);
            }
        }else if(cmd_num == 64){                //接收到推送信息
            push_reached = true;
            is_trans = false;
        }

        //ui->textBrowser->insertPlainText("......"+info+"\n");


    info.clear();

}

/**
 * 发送校验失败消息
 *
 */
bool MainWindow::send_com_err(QString data)
{
    /*//以ASCII码形式将行编辑框中的数据写入串口
    QTextCodec * codec = QTextCodec::codecForName("GBK"); //建立一个unicode与GBK之间的转换器
    codec->fromUnicode(jsonstr); //unicode转换成gbk*/

    QByteArray info = QByteArray("").append("0004").append("52").append("001001").append(data.toUtf8());
    int crc_cur = crc16(data.toUtf8())%10000;
    QString crc_tmp = QString("%1%2%3%4").arg(crc_cur/1000).arg(crc_cur%1000/100).arg(crc_cur%100/10).arg(crc_cur%10);

    QByteArray tmp = QByteArray("").append(pck_pre).append(pck_head).append(info).append(crc_tmp.toUtf8()).append(pck_pre).append(pck_tail);

    if(com->isWritable()){
        com->write(tmp);
    }

    return true;
}

/**
 * 发送校验结果数据消息
 *
 */
bool MainWindow::send_com_msg(QString data)
{
    /*//以ASCII码形式将行编辑框中的数据写入串口
    QTextCodec * codec = QTextCodec::codecForName("GBK"); //建立一个unicode与GBK之间的转换器
    codec->fromUnicode(jsonstr); //unicode转换成gbk*/

    QByteArray info = QByteArray("").append("0004").append("51").append("001001").append(data.toUtf8());
    int crc_cur = crc16(data.toUtf8())%10000;
    QString crc_tmp = QString("%1%2%3%4").arg(crc_cur/1000).arg(crc_cur%1000/100).arg(crc_cur%100/10).arg(crc_cur%10);

    QByteArray tmp = QByteArray("").append(pck_pre).append(pck_head).append(info).append(crc_tmp.toUtf8()).append(pck_pre).append(pck_tail);

    if(com->isWritable()){
        ui->textBrowser->insertPlainText("send msg:"+data+"\n");
        com->write(tmp);
    }
    return true;
}

/**
 * 发送串口数据
 *
 */
void MainWindow::send_com_info(int seg_no)
{

//    QMessageBox::about(this,"Tsignal", "This is a signal/slot sample");
    /*//以ASCII码形式将行编辑框中的数据写入串口
    QTextCodec * codec = QTextCodec::codecForName("GBK"); //建立一个unicode与GBK之间的转换器
    codec->fromUnicode(jsonstr); //unicode转换成gbk*/

    QStringList datas = split_string(results);
    QByteArray desc;

    int i = seg_no;
    int crc_cur=0;

    QByteArray tmp = QString(datas.at(i)).toUtf8();
    desc.clear();
    desc.append(pck_pre).append(pck_head);
    QString cur_sizestr = QString("%1%2%3%4").arg((tmp.size())/1000).arg((tmp.size())%1000/100).arg((tmp.size())%100/10).arg((tmp.size())%10);
    desc.append(cur_sizestr.toUtf8());

    QString cmd_num = QString("%1%2").arg((cmd_no)/10).arg((cmd_no)%10);
    desc.append(cmd_num.toUtf8());

    QString cur_nostr = QString("%1%2%3").arg((i+1)/100).arg((i+1)%100/10).arg((i+1)%100%10);
    desc.append(cur_nostr.toUtf8());
    QString total_nostr = QString("%1%2%3").arg((datas.size())/100).arg((datas.size())%100/10).arg((datas.size())%10);
    desc.append(total_nostr.toUtf8());
    desc.append(tmp);
    crc_cur = crc16(tmp)%10000;
    QString crc_tmp = QString("%1%2%3%4").arg(crc_cur/1000).arg(crc_cur%1000/100).arg(crc_cur%100/10).arg(crc_cur%10);
    desc.append(crc_tmp.toUtf8());
    desc.append(pck_pre).append(pck_tail);

    if(com->isWritable()){
        com->write(desc);
    }

    ui->textBrowser->insertPlainText("send:"+tmp+"\n");


    return ;
}

/**
 * 保存图片
 *
 */
bool MainWindow::save_com_pic(QString pic_name, QByteArray bta)
{
    QByteArray bytearray = QByteArray().append(0xFF).append(0xD8);
    bta.append(0xFF).append(0xD9);
    bytearray.append(bta);

QFile pic_file("pic/"+pic_name+".jpg");
pic_file.open(QIODevice::ReadWrite);
pic_file.write(bytearray);
return true;

//    QPixmap image;
//    image.loadFromData(bytearray);
//    bool is_save = image.save("pic/"+pic_name+".jpg");

//    return is_save;
}

/**
 * 分割要发送的字符串
 *
 */
QStringList MainWindow::split_string(QString str)
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
int MainWindow::crc16(QByteArray data)
{
    int crc=0;
    for(int i =0;i<data.size();i++){
        crc += data.at(i);
    }
    return abs(crc) ;                    /* Return updated CRC */
}

/************************************************************************************************************/
/**
 * 消息推送,简单的定时查询调度信息，如果存在则推送给串口
 *
 */
void MainWindow::_onWriteTimeout()
{
    //ui->textBrowser->insertPlainText("\n启动定时器...\n");
    QJsonArray arr = query.auto_query();
    int len = arr.size();

    if(len>0){
        int i = 0;
        while(i<len){
            QJsonValue jv = arr.at(i);
            QJsonObject json = jv.toObject();
            QJsonDocument document;
            document.setObject(json);

            QByteArray byte_array = document.toJson(QJsonDocument::Compact);
            QString json_str(byte_array);

            QJsonObject::iterator iter1 = json.find("id");
            QJsonObject::iterator iter2 = json.find("state");
            QString id = iter1.value().toString();
            QString state = QString("%1").arg(iter2.value().toInt());

            QString old_state = info_map.value(id);
            if(old_state!=state && com!=NULL && !is_trans){
                if(!push_reached){
                    if(com->isWritable()){

                        QByteArray des;
                        QByteArray tm = json_str.toUtf8();
                        QString crc;

                        des.clear();
                        des.append(pck_pre).append(pck_head);
                        QString sizestr = QString("%1%2%3%4").arg((tm.size())/1000).arg((tm.size())%1000/100).arg((tm.size())%100/10).arg((tm.size())%10);
                        des.append(sizestr);

                        des.append("64");

                        QString cur_nostr = QString("001");
                        des.append(cur_nostr);
                        QString total_nostr = QString("001");
                        des.append(total_nostr);
                        des.append(tm);

                        int crc_int=0;
                        for(int i =0;i<tm.size();i++){
                            crc_int += tm.at(i);
                        }
                        crc_int = crc_int%10000;

                        crc = QString("%1%2%3%4").arg(crc_int/1000).arg(crc_int%1000/100).arg(crc_int%100/10).arg(crc_int%10);
                        des.append(crc.toUtf8());
                        des.append(pck_pre).append(pck_tail);

                        if(com->isWritable()){
                            com->write(des);
                        }

                        //ui->textBrowser->insertPlainText(tm+"\n");

                        break;
                    }
                }else{
                    push_reached = false;
                    info_map.insert(id, state);
                }
            }else{

            }

            i++;

        }
    }else{
        info_map.clear();
    }
}
/************************************************************************************************************/


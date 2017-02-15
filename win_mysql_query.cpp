#include "win_mysql_query.h"

Win_Mysql_Query::Win_Mysql_Query()
{
    QFile mFile("mysql.ini");
    if(!mFile.exists()){
        qWarning("This is a debug message");
        init_config();
    }
    //打开MySQL
    db = QSqlDatabase::addDatabase("QMYSQL");
    //配置数据库连接的相关参数
    init_param();
}

Win_Mysql_Query::~Win_Mysql_Query()
{
    db.close();
}


bool Win_Mysql_Query::reset_mysql()
{
    db.close();
    bool is_open = db.open();
    return is_open;
}

bool Win_Mysql_Query::init_param()
{
    QSettings *configIniRead = new QSettings("mysql.ini", QSettings::IniFormat);

    //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
    QString hostname = configIniRead->value("/mysql/hostname").toString();
    int port = configIniRead->value("/mysql/port").toInt();
    QString dbname = configIniRead->value("/mysql/dbname").toString();
    QString uname = configIniRead->value("/user/uname").toString();
    QString pwd = configIniRead->value("/user/pwd").toString();

    //读入入完成后删除指针
    delete configIniRead;


    db.setHostName(hostname);   //设置主机地址
    db.setPort(port);           //设置端口
    db.setDatabaseName(dbname); //设置数据库名称
    db.setUserName(uname);      //设置用户名
    db.setPassword(pwd);        //设置密码
    is_open = db.open();        // 尝试连接数据库
    if (!is_open){
        qWarning() << "打开数据库失败";
        return false;
    }

    return true;
}

bool Win_Mysql_Query::init_config()
{
    //Qt中使用QSettings类读写ini文件
    //QSettings构造函数的第一个参数是ini文件的路径,第二个参数表示针对ini文件,第三个参数可以缺省
    QSettings *configIniWrite = new QSettings("mysql.ini", QSettings::IniFormat);
    //向ini文件中写入内容,setValue函数的两个参数是键值对
    //向ini文件的第一个节写入内容,mysql节下的第一个参数
    configIniWrite->setValue("/mysql/hostname", "localhost");
    //向ini文件的第一个节写入内容,mysql节下的第二个参数
    configIniWrite->setValue("mysql/port", "3306");
    //向ini文件的第一个节写入内容,mysql节下的第一个参数
    configIniWrite->setValue("mysql/dbname", "mrps");

    //向ini文件的第二个节写入内容,user节下的第一个参数
    configIniWrite->setValue("user/uname", "root");
    //向ini文件的第二个节写入内容,user节下的第二个参数
    configIniWrite->setValue("user/pwd", "123456");
    //写入完成后删除指针
    delete configIniWrite;
    return true;
}

QJsonArray Win_Mysql_Query::auto_query( )//推送查询
{

    QJsonArray jsonArr;

    if(db.open()){//数据库已经打开


        //第一种参数追加方法
        sql = QString("select d.dispatch_id,d.track_id,d.erection_state,t.track_name from mrps_dispatch as d join mrps_track as t where d.track_id=t.track_id and d.dispatch_state=0 and (d.erection_state=0 or"
                    " d.erection_state=5 or d.erection_state=6 or d.erection_state=7 or d.erection_state=-1)");

        //查询所有数据
        QSqlQuery sql_query;
        sql_query.prepare(sql);
        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError();
        }
        else
        {   //如果返回结果不为空
            if(sql_query.size()>0){
                while(sql_query.next())
                {
                    QString dispatch_id = sql_query.value(0).toString();
                    QString track_id = sql_query.value(1).toString();
                    int dis_state = sql_query.value(2).toInt();
                    QString track_name = sql_query.value(3).toString();
                    //生成Json
                    QJsonObject tempObj;
                    tempObj.insert("id", dispatch_id);
                    tempObj.insert("tid", track_id);
                    tempObj.insert("state", dis_state);
                    tempObj.insert("tn", track_name);

                    jsonArr.append(tempObj);
                }

            }
        }
        sql_query.clear();
    }else{
        reset_mysql();//重新连接mysql
    }
    return jsonArr;

}

QString Win_Mysql_Query::login_query(QString uname, QString pwd )//用户登录
{
    QJsonObject json;
    //第一种参数追加方法
    sql = QString("select * from mrps_user where user_name='%1'"
                  " and user_password='%2'").arg(uname).arg(pwd);

    //查询所有数据
    QSqlQuery sql_query;
    sql_query.prepare(sql);
    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
        //生成Json
        json.insert("suc", false);
        json.insert("info", "查询异常");
    }
    else
    {   //如果返回结果不为空
        if(sql_query.size()>0){
            while(sql_query.next())
            {
                QString uid = sql_query.value(0).toString();
                QString name = sql_query.value(1).toString();
                //生成Json
                json.insert("suc", true);
                json.insert("info", "查询成功");
                json.insert("uid", uid);
                json.insert("uname", name);
                //qDebug()<<QString("userId:%1 userName:%2 userPass:%3").arg(uid).arg(name).arg(pass);
            }
        }else{
            //生成Json
            json.insert("suc", false);
            json.insert("info", "无此用户");
        }
    }
    sql_query.clear();

    //Json转成字符串
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;
}

QString Win_Mysql_Query::info_query( )//消息查询
{
    QJsonArray jsonArr;
    QJsonObject jsonObj;


    //第一种参数追加方法
    sql = QString("select * from mrps_dispatch where dispatch_state=0 and (erection_state=0 or"
                  " erection_state=5 or erection_state=6 or erection_state=7 or erection_state=-1)");

    //查询所有数据
    QSqlQuery sql_query;
    sql_query.prepare(sql);
    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
        //生成Json
        jsonObj.insert("suc", false);
    }
    else
    {   //如果返回结果不为空
        if(sql_query.size()>0){
            jsonObj.insert("suc", true);
            jsonObj.insert("info", "查询成功");
            while(sql_query.next())
            {
                QString dispatch_id = sql_query.value(0).toString();
                QString track_id = sql_query.value(1).toString();
                //QString pub_id = sql_query.value(2).toString();
                //QString seneeid = sql_query.value(3).toString();
                int dis_state = sql_query.value(4).toInt();
                //QString create_time = sql_query.value(5).toString();
                //int erection_state = sql_query.value(6).toInt();
                //QString device_id = sql_query.value(7).toString();
                //QString erection_id = sql_query.value(12).toString();
                //QString verify_id = sql_query.value(13).toString();
                //QString remove_id = sql_query.value(14).toString();
                //QString remove_verify_id = sql_query.value(15).toString();
                //QString erection_time = sql_query.value(16).toString();
                //QString verify_time = sql_query.value(17).toString();
                //QString remove_time = sql_query.value(18).toString();
                //QString remove_verify_time = sql_query.value(19).toString();
                //生成Json
                QJsonObject tempObj;
                tempObj.insert("id", dispatch_id);
                tempObj.insert("tid", track_id);
                //tempObj.insert("pub_id", pub_id);
                //tempObj.insert("seneeid", seneeid);
                tempObj.insert("state", dis_state);
                //tempObj.insert("create_time", create_time);
                //tempObj.insert("erect_state", erection_state);
                //tempObj.insert("device_id", device_id);
                //tempObj.insert("erect_id", erection_id);
                //tempObj.insert("vry_id", verify_id);
                //tempObj.insert("rm_id", remove_id);
                //tempObj.insert("rm_vry_id", remove_verify_id);
                //tempObj.insert("erect_time", erection_time);
                //tempObj.insert("vry_time", verify_time);
                //tempObj.insert("rm_time", remove_time);
                //tempObj.insert("rm_vry_time", remove_verify_time);

                jsonArr.append(tempObj);
            }

            QJsonDocument doc;
            doc.setArray(jsonArr);
            QByteArray byte_array = doc.toJson(QJsonDocument::Compact);
            QString temp_str(byte_array);

            jsonObj.insert("dis",temp_str);
        }else{
            //生成Json
            jsonObj.insert("suc", false);
            jsonObj.insert("info", "无消息");
        }
    }
    sql_query.clear();

    //Json转成字符串
    QJsonDocument document;
    document.setObject(jsonObj);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;

}

QString Win_Mysql_Query::track_query( )//股道查询
{
    QJsonArray jsonArr;
    QJsonObject jsonObj;


    //第一种参数追加方法
    sql = QString("select track_id,track_name from mrps_track where track_state=0");

    //查询所有数据
    QSqlQuery sql_query;
    sql_query.prepare(sql);
    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
        //生成Json
        jsonObj.insert("suc", false);
        jsonObj.insert("info", "查询异常");
    }
    else
    {   //如果返回结果不为空
        if(sql_query.size()>0){
            jsonObj.insert("suc", true);
            jsonObj.insert("info", "查询成功");
            while(sql_query.next())
            {
                QString track_id = sql_query.value(0).toString();
                QString track_name = sql_query.value(1).toString();
                //生成Json
                QJsonObject tempObj;
                tempObj.insert("id", track_id);
                tempObj.insert("name", track_name);

                jsonArr.append(tempObj);
            }

            QJsonDocument doc;
            doc.setArray(jsonArr);
            QByteArray byte_array = doc.toJson(QJsonDocument::Compact);
            QString temp_str(byte_array);

            jsonObj.insert("track",temp_str);
        }else{
            //生成Json
            jsonObj.insert("track", false);
            jsonObj.insert("info", "无股道");
        }
    }
    sql_query.clear();

    //Json转成字符串
    QJsonDocument document;
    document.setObject(jsonObj);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;

}

bool Win_Mysql_Query::device_operate(QString did, int state )
{
    bool operate = false;
    //查询所有数据
    QSqlQuery sql_query;
    //sql_query.prepare(sql);

    sql_query.prepare( "update mrps_device set device_state=:state "
                   " where device_id =:did "
                    );
    sql_query.bindValue( ":state", state);
    sql_query.bindValue( ":did", did);

    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
        operate = false;
    }else if(sql_query.numRowsAffected()==1)
    {
        operate = true;

    }else
    {
        operate = false;

    }
    sql_query.clear();

    return operate;
}

bool Win_Mysql_Query::device_chkstate(QString did, int state )
{
    bool operate = false;
    //查询所有数据
    QSqlQuery sql_query;
    //sql_query.prepare(sql);

    sql_query.prepare( "select device_id, device_state from mrps_device "
                   " where device_id =:did and device_state=:state"
                    );
    sql_query.bindValue( ":did", did);
    sql_query.bindValue( ":state", state);

    if(!sql_query.exec())
    {
        qDebug()<<"sql_query.lastError()!";
        operate = false;
    }else if(sql_query.size()>0)
    {
        operate = true;

    }else
    {
        operate = false;
    }
    sql_query.clear();

    return operate;
}

QString Win_Mysql_Query::device_query(QString did, int state )
{
    QJsonObject json;
    //第二种参数追加方法

    //查询所有数据
    QSqlQuery sql_query;
    //sql_query.prepare(sql);

    sql_query.prepare( "update mrps_device set device_state=:state "
                   " where device_id =:did "
                    );
    sql_query.bindValue( ":state", state);
    sql_query.bindValue( ":did", did);

    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
        //生成Json
        json.insert("suc", false);
        json.insert("info", "查询失败");
    }
    else
    {
        //生成Json
        json.insert("suc", true);
        json.insert("info", "查询成功");
        json.insert("device_id", did);

    }
    sql_query.clear();

    //Json转成字符串
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;
}

QString Win_Mysql_Query::delivery_query(QString lid, QString did )//出库(libraryId, deviceId)
{
    QJsonObject json;
    //第二种参数追加方法
    QDateTime time=QDateTime::currentDateTime();//获取当前时间
    QString time_str=time.toString("yyyy-MM-dd hh:mm:ss");//格式转换

    //查询所有数据
    QSqlQuery sql_query;
    //sql_query.prepare(sql);

    sql_query.prepare( "update mrps_device set device_state=2, dv_time="
                   ":time where device_id =:did "
                   " and library_id =:lid and device_state=0"
                       );
    sql_query.bindValue( ":time", time_str);
    sql_query.bindValue( ":did", did);
    sql_query.bindValue( ":lid", lid);

    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
        //生成Json
        json.insert("suc", false);
        json.insert("info", "操作失败");
    }
    else if(sql_query.numRowsAffected()==1)
    {
        json.insert("suc", true);
        json.insert("info", "操作成功");
        json.insert("device_id", did);

    }
    else
    {
        //生成Json
        json.insert("suc", false);
        json.insert("info", "设备未处于入库状态");
        json.insert("device_id", did);

    }
    sql_query.clear();

    //Json转成字符串
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;
}

QString Win_Mysql_Query::inquiry_query(QString lid, QString did )//入库
{
    QJsonObject json;
    //第三种参数追加方法
    QDateTime time=QDateTime::currentDateTime();//获取当前时间
    QString time_str=time.toString("yyyy-MM-dd hh:mm:ss");//格式转换

    //查询所有数据
    QSqlQuery sql_query;
    //sql_query.prepare(sql);

    sql_query.prepare( "update mrps_device set device_state=0, wv_time=? "
                   " where device_id =? "
                   " and library_id =? and device_state=2"
                       );
    sql_query.addBindValue(time_str);
    sql_query.addBindValue(did);
    sql_query.addBindValue(lid);

    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
        //生成Json
        json.insert("suc", false);
        json.insert("info", "操作失败");
    }
    else if(sql_query.numRowsAffected()==1)
    {
        json.insert("suc", true);
        json.insert("info", "操作成功");
        json.insert("device_id", did);

    }else
    {
        //生成Json
        json.insert("suc", false);
        json.insert("info", "设备未处于出库状态");
        json.insert("device_id", did);

    }
    sql_query.clear();

    //Json转成字符串
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;
}

QString Win_Mysql_Query::erection_query(QString disid, QString uid,
                       QString did, QString path, QString position)//设备安设
{
    QJsonObject json;

    if(!device_chkstate(did, 2)){ //是否出库
        //生成Json
        json.insert("suc", false);
        json.insert("info", "操作失败,设备未出库或正在使用中");
        json.insert("dis_id", disid);
    }else{

        //第三种参数追加方法
        QDateTime time=QDateTime::currentDateTime();//获取当前时间
        QString time_str=time.toString("yyyy-MM-dd hh:mm:ss");//格式转换

        //查询所有数据
        QSqlQuery sql_query;
        //sql_query.prepare(sql);

        sql_query.prepare( "update mrps_dispatch set erection_state=1,"
                    " erection_id=? , erection_time=?,device_id=?"
                    " erection_photo_path=?,sps_position1=? "
                    " where dispatch_id =? "
                    " and dispatch_state =0 "
                       );
        sql_query.addBindValue(uid);
        sql_query.addBindValue(time_str);
        sql_query.addBindValue(did);
        sql_query.addBindValue(path);
        sql_query.addBindValue(position);
        sql_query.addBindValue(disid);

        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError();
            //生成Json
            json.insert("suc", false);
            json.insert("info", "操作失败");
        }
        else if(sql_query.numRowsAffected()==1)
        {
            device_operate(did,3);          //修改设备状态
            //生成Json
            json.insert("suc", true);
            json.insert("info", "操作成功");
            json.insert("dis_id", disid);

        }else
        {
            //device_operate(did,3);          //修改设备状态
            //生成Json
            json.insert("suc", false);
            json.insert("info", "无调度任务");
            json.insert("dis_id", disid);

        }
        sql_query.clear();
    }
    //Json转成字符串
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;
}

QString Win_Mysql_Query::removal_query(QString disid, QString uid,
                       QString did, QString path, QString position)//设备撤除
{
    QJsonObject json;

    if(!device_chkstate(did, 3)){ //是否出库
        //生成Json
        json.insert("suc", false);
        json.insert("info", "操作失败,设备未在使用中");
        json.insert("dis_id", disid);
    }else{
        //第三种参数追加方法
        QDateTime time=QDateTime::currentDateTime();//获取当前时间
        QString time_str=time.toString("yyyy-MM-dd hh:mm:ss");//格式转换

        //查询所有数据
        QSqlQuery sql_query;
        //sql_query.prepare(sql);

        sql_query.prepare( "update mrps_dispatch set erection_state=3,"
                    " removal_id=? , removal_time=?,"
                    " removal_photo_path=?,sps_position2=? "
                    " where dispatch_id =? "
                    " and dispatch_state =0 "
                       );
        sql_query.addBindValue(uid);
        sql_query.addBindValue(time_str);
        sql_query.addBindValue(path);
        sql_query.addBindValue(position);
        sql_query.addBindValue(disid);

        if(!sql_query.exec())
        {
            qDebug()<<sql_query.lastError();
            //生成Json
            json.insert("suc", false);
            json.insert("info", "操作失败");
        }
        else if(sql_query.numRowsAffected()==1)
        {
            device_operate(did,2);          //修改设备状态
            //生成Json
            json.insert("suc", true);
            json.insert("info", "操作成功");
            json.insert("dis_id", disid);

        }else
        {
            //device_operate(did,2);          //修改设备状态
            //生成Json
            json.insert("suc", false);
            json.insert("info", "无调度任务");
            json.insert("dis_id", disid);

        }
        sql_query.clear();
    }
    //Json转成字符串
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    return json_str;
}

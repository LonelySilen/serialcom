#ifndef WIN_MYSQL_QUERY_H
#define WIN_MYSQL_QUERY_H
#include <QDebug>
#include <QtSql>
#include <QSettings>
#include <QFile>
#include <QJsonObject>

class Win_Mysql_Query
{
public:
    Win_Mysql_Query();
    ~Win_Mysql_Query();

    QJsonArray auto_query( );
    QString login_query(QString uname, QString pwd );   //用户登录
    QString info_query();                   //消息查询
    QString track_query( );                 //股道查询
    bool device_operate(QString did, int state );       //设备状态操作(3,使用中;2,出库状态)
    bool device_chkstate(QString did, int state );      //设备状态查询(3,使用中;2,出库状态)
    QString device_query(QString did, int state );      //设备状态操作(3,使用中;2,出库状态)
    QString delivery_query(QString lid, QString did );  //出库(libraryId, deviceId)
    QString inquiry_query(QString lid, QString did );   //入库
    QString erection_query(QString disid, QString uid,
                           QString did, QString path, QString position);  //设备安设
    QString removal_query(QString disid, QString uid,
                           QString did, QString path, QString position);  //设备撤除

    bool reset_mysql();
    bool init_param();
    bool init_config();

private:
    QSqlDatabase db;
    QString sql;
    bool is_open;
};

#endif // WIN_MYSQL_QUERY_H

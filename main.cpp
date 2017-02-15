#include <QApplication>
#include <QTextCodec>
#include "mainwindow.h"
#include "global.h"

int main(int argc, char **argv)
{

    QApplication app(argc, argv);
    QApplication::addLibraryPath("./plugins");
    app.setWindowIcon(QIcon(":/images/icon.png"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    //注册MessageHandler,用于打印日志
    qInstallMessageHandler(logOutPut);
    MainWindow w;
    w.show();


    return app.exec();


//    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
//    db.setHostName("localhost");
//    db.setDatabaseName("mrps");
//    db.setUserName("root");
//    db.setPassword("123456");
//    if (!db.open())
//        qDebug() << "Failed to connect to root mysql admin";

//        QString select_all_sql = "select * from mrps_user";
//        //查询所有数据
//        QSqlQuery sql_query;
//        sql_query.prepare(select_all_sql);
//        if(!sql_query.exec())
//        {
//            qDebug()<<sql_query.lastError();
//        }
//        else
//        {
//            while(sql_query.next())
//            {
//                int id = sql_query.value(0).toInt();
//                QString templ_name = sql_query.value(2).toString();
//                qDebug()<<QString("Id:%1     Templ Name:%2").arg(id).arg(templ_name);
//            }
//        }
//    db.close();
//return 0;
}

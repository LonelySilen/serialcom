#ifndef GLOBAL
#define GLOBAL
#include <QFile>
#include <QtDebug>
//打印日志的方法
//qDebug("This is a debug message");
//qWarning("This is a warning message");
//qCritical("This is a critical message");
//qFatal("This is a fatal message");

//注册MessageHandler,用于打印日志
//qInstallMessageHandler(logOutPut);
void logOutPut(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;

    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
        exit(0);
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

    QFile file("log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    //如果日志文件大小超过,则清空
    if(file.size()>1024*1024)
        file.resize(0);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();

}
#endif // GLOBAL


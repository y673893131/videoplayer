#include "ui/widget.h"

#include <QApplication>
#include <QtPlugin>
#include <QTranslator>
#include <QDir>
#include <QDebug>
#include <QThread>
#include "dump/mini_dump.hpp"
#include "qtsingleapplication/qtsingleapplication.h"


//Q_IMPORT_PLUGIN(QJpegPlugin)
//void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
//{
//  if (type != QtWarningMsg || !msg.startsWith("QWindowsWindow::setGeometry")) {
//    QByteArray localMsg = msg.toLocal8Bit();
////    fprintf(stdout, localMsg.constData());
//    OutputDebugStringA(localMsg);
//    OutputDebugStringA("\n");
//  }
//}
int main(int argc, char *argv[])
{
//    qInstallMessageHandler(myMessageOutput);
#ifdef Q_OS_WIN
    win32::debug::mini_dump dump;
//    QThread::currentThread()->setPriority(QThread::HighPriority);
//    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
//    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
//    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
    QtSingleApplication a("{BBE61D2F-EDE8-48E3-A48C-D5CBB9CCC053}", argc, argv);
    if (a.isRunning())
    {
        auto sArg = a.arguments().join('|');
        a.sendMessage(sArg);
        return EXIT_SUCCESS;
    }

    auto trans = new QTranslator();
    trans->load(":translate/zh");
    a.installTranslator(trans);

    Widget w;
    w.show();
    return a.exec();
}

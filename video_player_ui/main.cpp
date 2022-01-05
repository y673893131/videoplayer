#include "ui/widget.h"

#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QDebug>
#include "dump/mini_dump.hpp"

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  if (type != QtWarningMsg || !msg.startsWith("QWindowsWindow::setGeometry")) {
    QByteArray localMsg = msg.toLocal8Bit();
    fprintf(stdout, localMsg.constData());
  }
}
int main(int argc, char *argv[])
{
//    qInstallMessageHandler(myMessageOutput);
#ifdef Q_OS_WIN
    win32::debug::mini_dump dump;
#endif
    QApplication a(argc, argv);

    auto trans = new QTranslator();
    trans->load(":translate/zh");
    a.installTranslator(trans);

    Widget w;
    w.show();
    return a.exec();
}

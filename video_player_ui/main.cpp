#include "ui/widget.h"

#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QDebug>

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
    QApplication a(argc, argv);

    auto trans = new QTranslator();
    trans->load(":translate/zh");
    a.installTranslator(trans);

    Widget w;
    w.show();
    return a.exec();
}

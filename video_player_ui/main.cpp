#include "widget.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto trans = new QTranslator();
    trans->load(":translate/zh");
    a.installTranslator(trans);

    Widget w;
    w.show();
#ifdef WIN32
    a.installNativeEventFilter(&w);
#endif
    return a.exec();
}

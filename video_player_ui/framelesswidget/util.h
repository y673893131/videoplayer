#ifndef CUTIL_H
#define CUTIL_H

#include <QWidget>

#define UTIL CUtil::instance()
#define CALC_WIDGET_SIZE(widget, w, h) UTIL->setDesktopPercent(widget, w, h)
#define CALC_WIDGET_WIDTH(widget, w) UTIL->setDesktopPercentWidth(widget, w)
#define CALC_WIDGET_HEIGHT(widget, h) UTIL->setDesktopPercentHeight(widget, h)

#define CALC_SIZE(w, h) CALC_WIDGET_SIZE(nullptr, w, h)
#define CENTER_WIDGET(widget) UTIL->center(widget)
#define CENTER_DESKTOP(widget) UTIL->centerDesktop(widget)

class CUtil : public QObject
{
    Q_OBJECT
public:
    static CUtil* instance();

    QSize setDesktopPercent(QWidget* widget, int x, int y);
    int setDesktopPercentWidth(QWidget* widget, int w);
    int setDesktopPercentHeight(QWidget* widget, int w);
    void centerDesktop(QWidget* widget = Q_NULLPTR);
    void center(QWidget* widget = Q_NULLPTR);
    void setWindowEllispeFrame(QWidget* widget, int nWidthEllipse, int nHeightEllipse);
    int getMs(const QString& s);
private:
    CUtil();
private:
    Q_DISABLE_COPY(CUtil)
    static CUtil* m_instance;
    QSize m_desktopSize;
    QSize m_baseSize;
};

#endif // CUTIL_H

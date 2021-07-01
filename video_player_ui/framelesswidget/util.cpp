#include "util.h"
#include <QApplication>
#include <QDesktopWidget>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

CUtil* CUtil::m_instance=nullptr;
CUtil *CUtil::instance()
{
    if(!m_instance)
        m_instance = new CUtil();
    return m_instance;
}

CUtil::CUtil()
{
    m_baseSize = QSize(1920, 1080);
    m_desktopSize = qApp->desktop()->availableGeometry().size();
}

QSize CUtil::setDesktopPercent(QWidget* widget, int w, int h)
{
    auto xNew = static_cast<int>(w * m_desktopSize.width() / (m_baseSize.width() * 1.0f));
    auto yNew = static_cast<int>(h * m_desktopSize.height() / (m_baseSize.height() * 1.0f));
    if(widget)
    {
//        widget->setMinimumSize(xNew, yNew);
        widget->setFixedSize(xNew, yNew);
//        widget->adjustSize();
    }

    return QSize(xNew, yNew);
}

int CUtil::setDesktopPercentWidth(QWidget *widget, int w)
{
    auto xNew = static_cast<int>(w * m_desktopSize.width() / (m_baseSize.width() * 1.0f));
    if(widget)
    {
        widget->setFixedWidth(xNew);
    }

    return xNew;
}

int CUtil::setDesktopPercentHeight(QWidget *widget, int h)
{
    auto yNew = static_cast<int>(h * m_desktopSize.height() / (m_baseSize.height() * 1.0f));
    if(widget)
    {
        widget->setFixedHeight(yNew);
    }

    return yNew;
}

void CUtil::centerDesktop(QWidget *widget)
{
    if(!widget)
    {
        return;
    }

    widget->move((m_desktopSize.width() - widget->width()) / 2, (m_desktopSize.height() - widget->height()) / 2);
}

void CUtil::center(QWidget* widget)
{
    if(!widget)
    {
        return;
    }

    auto parent = widget->parentWidget();
    if(!parent)
    {
        centerDesktop(widget);
    }
    else
    {
        widget->move(parent->x() + (parent->width() - widget->width()) / 2, parent->y() + (parent->height() - widget->height()) / 2);
    }
}

void CUtil::setWindowEllispeFrame(QWidget* widget, int nWidthEllipse, int nHeightEllipse)
{
#ifdef Q_OS_WIN
    HRGN hRgn;
    hRgn = CreateRoundRectRgn(0, 0, widget->width()+1, widget->height()+1, nWidthEllipse, nHeightEllipse);
    SetWindowRgn(reinterpret_cast<HWND>(widget->winId()), hRgn, true);
#endif
}

int CUtil::getMs(const QString& s)
{
    auto times = s.split('.');
    if(times.size() != 2)
        return 0;
    auto time = times.at(0).split(':');
    if(time.size() != 3)
        return 0;
    return (time.at(0).toInt() * 3600 + time.at(1).toInt() * 60 + time.at(2).toInt()) * 1000 + times.at(1).toInt();
}


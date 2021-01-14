#include "framelesswidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QAction>
#include "qdragborder.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

QFrameLessWidget::QFrameLessWidget(QWidget *parent)
    : QWidget(parent)
    , m_bTopWindow(false)
{
#ifdef unix
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
#else
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
#endif

#ifdef unix
    m_dragBorder = new QDragBorder(this);
#endif
}

QFrameLessWidget::~QFrameLessWidget()
{
}

void QFrameLessWidget::setDragSelf(bool bDrag)
{
#ifdef Q_OS_WIN
    auto func = [=]
    {
        if(::ReleaseCapture())
        {
            ::SendMessage(HWND(this->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    };

    if(bDrag)
    {
        connect(this, &QFrameLessWidget::leftPress, this, func);
    }
#endif
}

void QFrameLessWidget::setDoubleClickFullScreen()
{
    auto func = [this]
    {
        if(!isFullScreen())
            showFullScreen();
        else
            showNormal();
    };

    connect(this, &QFrameLessWidget::leftDoubleClicked, func);
}

void QFrameLessWidget::setDoubleClickMax()
{
    auto func = [this]
    {
        if(!isMaximized())
            showMaximized();
        else
            showNormal();
    };

    connect(this, &QFrameLessWidget::leftDoubleClicked, func);
}

void QFrameLessWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
#ifdef unix
        m_dragBorder->setStartPos(pos(), event->globalPos());
#else
        emit leftPress();
#endif
    }
    if(event->button() == Qt::RightButton)
    {
        emit rightClicked();
    }
}

void QFrameLessWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit leftDoubleClicked();
    }
}

void QFrameLessWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if(!m_bkImg.isNull())
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.drawImage(0, 0, m_bkImg.scaled(this->size()));
    }
}

bool QFrameLessWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if(_nativeEvent(eventType, message, result, this))
        return true;

    return QWidget::nativeEvent(eventType, message, result);
}

void QFrameLessWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

#ifdef unix
    m_dragBorder->setFixedSize(size());
#endif

    updateTopWindow();
}

bool QFrameLessWidget::isValid()
{
    return false;
}

void QFrameLessWidget::updateTopWindow()
{
    if(!isFullScreen())
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(this->winId()), m_bTopWindow ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
#else
        setWindowFlag(Qt::WindowStaysOnTopHint, m_bTopWindow);
#endif
    }
    else
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
#else
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        show();
#endif
    }
}

void QFrameLessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
#ifdef unix
    if(m_dragBorder->isVisible())
    {
        m_dragBorder->hide();
        move(m_dragBorder->pos());
    }
#endif
}

void QFrameLessWidget::mouseMoveEvent(QMouseEvent *event)
{
//    QWidget::mouseMoveEvent(event);
#ifdef unix
    if(event->buttons() == Qt::LeftButton)
    {
        m_dragBorder->show();
        m_dragBorder->setMovePos(event->globalPos());
    }
#endif
}

void QFrameLessWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        if(isFullScreen())
            showNormal();
        break;
    }
}

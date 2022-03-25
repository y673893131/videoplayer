#include "framelesswidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QAction>
#include <QCursor>
#include "qdragborder.h"
#include "qframelesswidget_p.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#elif defined(Q_OS_LINUX)
#include "nativeevent_uinx.h"
#endif

QFrameLessWidget::QFrameLessWidget(QWidget *parent)
    : QWidget(parent)
    , VP_INIT(new QFrameLessWidgetPrivate(this))
{
}

QFrameLessWidget::QFrameLessWidget(QFrameLessWidgetPrivate* pri, QWidget *parent)
    : QWidget(parent)
    , VP_INIT(pri)
{
    VP_D(QFrameLessWidget);
    setWindowFlags(d->m_flags);
#ifdef unix
//    m_dragBorder = new QDragBorder(this);
#endif
}

QFrameLessWidget::~QFrameLessWidget()
{
}

void QFrameLessWidget::setDragSelf(bool bDrag)
{
    VP_D(QFrameLessWidget);
    d->m_bDragEnable = bDrag;
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

void QFrameLessWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit leftDoubleClicked();
    }
}

void QFrameLessWidget::paintEvent(QPaintEvent *event)
{
    VP_D(QFrameLessWidget);
//    Q_UNUSED(event)
    if(!d->m_bkImg.isNull())
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.drawImage(0, 0, d->m_bkImg.scaled(this->size()));
    }

    QWidget::paintEvent(event);
}

bool QFrameLessWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    VP_D(QFrameLessWidget);
    if(d->nativeEvent(eventType, message, result, this))
        return true;

//#ifdef Q_OS_WIN
//    if(_nativeEvent(eventType, message, result, this))
//        return true;
//#endif
    return QWidget::nativeEvent(eventType, message, result);
}

void QFrameLessWidget::resizeEvent(QResizeEvent *event)
{
//    Q_UNUSED(event)
    QWidget::resizeEvent(event);

#ifdef unix
//    m_dragBorder->setFixedSize(size());
    emit resized(event->size());
#endif

    updateTopWindow();
}

bool QFrameLessWidget::isValid()
{
    return false;
}

void QFrameLessWidget::updateTopWindow()
{
    VP_D(QFrameLessWidget);
    if(!isFullScreen())
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(this->winId()), d->m_bTopWindow ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
#else
        auto flags = windowFlags();
        if(d->m_bTopWindow)
            flags = flags | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint;
        else
            flags = flags & ~Qt::WindowStaysOnTopHint & ~Qt::X11BypassWindowManagerHint;
//        setWindowFlag(Qt::WindowStaysOnTopHint, m_bTopWindow);
        setWindowFlags(flags);
//        if(m_bTopWindow)
//        {
            show();
//        }
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

void QFrameLessWidget::setTopWindow(bool bTop)
{
    VP_D(QFrameLessWidget);
    d->m_bTopWindow = bTop;
}

void QFrameLessWidget::mousePressEvent(QMouseEvent *event)
{
    VP_D(QFrameLessWidget);
    QWidget::mousePressEvent(event);
    d->mousePressEvent(event);
}

void QFrameLessWidget::mouseMoveEvent(QMouseEvent *event)
{
    VP_D(QFrameLessWidget);
    QWidget::mouseMoveEvent(event);
    d->mouseMoveEvent(event);
}

void QFrameLessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    VP_D(QFrameLessWidget);
    QWidget::mouseReleaseEvent(event);
    d->mouseReleaseEvent(event);
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

void QFrameLessWidget::onMoved(const QPoint & pos)
{
    move(pos);
}

void QFrameLessWidget::onResized(const QSize & size)
{
    resize(size);
}


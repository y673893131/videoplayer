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

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

QFrameLessWidget::QFrameLessWidget(QWidget *parent)
    : QWidget(parent)
    , m_bTopWindow(false)
    , m_bDragEnable(true)
    , m_bDrag(false)
{
#ifdef unix
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
#else
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
#endif

#ifdef unix
//    m_dragBorder = new QDragBorder(this);
#endif
}

QFrameLessWidget::~QFrameLessWidget()
{
}

void QFrameLessWidget::setDragSelf(bool bDrag)
{
    m_bDragEnable = bDrag;
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
        qDebug() << __FUNCTION__ << this;
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
//    Q_UNUSED(event)
    if(!m_bkImg.isNull())
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.drawImage(0, 0, m_bkImg.scaled(this->size()));
    }

    QWidget::paintEvent(event);
}

bool QFrameLessWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
    if(_nativeEvent(eventType, message, result, this))
        return true;
#endif
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
    if(!isFullScreen())
    {
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(this->winId()), m_bTopWindow ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
#else
        auto flags = windowFlags();
        if(m_bTopWindow)
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

void QFrameLessWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
#ifdef unix
        if(m_bDragEnable)
        {
            m_pos = pos();
            m_press = QCursor::pos();
            m_bDrag = true;
        }
//        m_dragBorder->setStartPos(pos(), event->globalPos());
#else
        emit leftPress();
#endif
    }
    if(event->button() == Qt::RightButton)
    {
        emit rightClicked();
    }
}

void QFrameLessWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if(event->buttons() == Qt::LeftButton)
    {
#ifdef unix
        if(m_bDragEnable)
        {
            auto pos = m_pos + QCursor::pos() - m_press;
            move(pos);
            emit moved(pos);
        }
//        m_dragBorder->setStartPos(pos(), event->globalPos());
#else
        emit leftPress();
#endif
    }
#ifdef unix

//    if(event->buttons() == Qt::LeftButton)
//    {
//        m_dragBorder->show();
//        m_dragBorder->setMovePos(event->globalPos());
//    }
#endif
}

void QFrameLessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
#ifdef unix
//    if(m_dragBorder->isVisible())
//    {
//        m_dragBorder->hide();
//        move(m_dragBorder->pos());
//    }
    m_bDrag = false;
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

void QFrameLessWidget::onMoved(const QPoint & pos)
{
    move(pos);
}

void QFrameLessWidget::onResized(const QSize & size)
{
    resize(size);
}

#include "qframelesswidget_p.h"
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include "framelesswidget.h"
#define PADDING 10

QFrameLessWidgetPrivate::QFrameLessWidgetPrivate(QFrameLessWidget* parent)
    : VP_Data(parent)
    , m_bResizeabe(true)
    , m_bTopWindow(false)
    , m_bDragEnable(true)
    , m_bDrag(false)
    , m_flags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint)
{
}

void QFrameLessWidgetPrivate::setResizeable(bool bResizeable)
{
    m_bResizeabe = bResizeable;
}

bool QFrameLessWidgetPrivate::resizeable()
{
    return m_bResizeabe;
}

bool QFrameLessWidgetPrivate::isCapture()
{
    return true;
}

void QFrameLessWidgetPrivate::mousePressEvent(QMouseEvent *event)
{
    VP_Q(QFrameLessWidget);
    if(event->button() == Qt::LeftButton)
    {
#ifdef unix
        m_press = QCursor::pos();
        if(m_bDragEnable)
        {
            m_pos = q->pos();
//            m_press = QCursor::pos();
            m_bDrag = true;
        }
#endif
    }
    if(event->button() == Qt::RightButton)
    {
        Q_EMIT q->rightClicked();
    }
}

void QFrameLessWidgetPrivate::mouseReleaseEvent(QMouseEvent *event)
{
#ifdef unix
    m_bDrag = false;
#endif
}

void QFrameLessWidgetPrivate::mouseMoveEvent(QMouseEvent *event)
{
    VP_Q(QFrameLessWidget);
    if(event->buttons() == Qt::LeftButton)
    {
        if(!isCapture())
            return;
#ifdef unix
        if(q->cursor() != Qt::ArrowCursor)
        {
            auto sub = QCursor::pos() - m_press;
            m_press = QCursor::pos();
            qDebug() << q->size() << sub;
            q->setFixedSize(q->size() + QSize(sub.x(), sub.y()));
        }
        else if(m_bDragEnable)
        {
            auto pos = m_pos + QCursor::pos() - m_press;
            q->move(pos);
            Q_EMIT q->moved(pos);
        }
#else
        Q_EMIT q->leftPress();
#endif
    }
    else
    {
        qDebug() << __FUNCTION__;
        checkMouseDirection(event->pos());
    }
}

bool QFrameLessWidgetPrivate::check(void *message, long *result)
{
    Q_UNUSED(message)
    Q_UNUSED(result)
    return false;
}

bool QFrameLessWidgetPrivate::nativeEvent(const QByteArray &eventType, void *message, long *result, QWidget *)
{
    Q_UNUSED(eventType)
    Q_UNUSED(message)
    Q_UNUSED(result)
    return false;
}

bool QFrameLessWidgetPrivate::checkMouseDirection(const QPoint &cursor)
{
    VP_Q(QFrameLessWidget);
    auto rc = q->rect();
    auto rect = QRect(q->mapToGlobal(rc.topLeft()), q->mapToGlobal(rc.bottomRight()));

//    qDebug() << rect << cursor << rect.contains(cursor);
    if(!rect.contains(cursor))
        return false;

    int x = cursor.x();
    int y = cursor.y();
    const int borderWidth = PADDING;

    // bottom left
    if (x >= rect.left() && x < rect.left() + borderWidth &&
            y < rect.bottom() && y >= rect.bottom() - borderWidth)
    {
        q->setCursor(Qt::SizeBDiagCursor);
        return true;
    }

    // bottom right()
    if (x < rect.right() && x >= rect.right() - borderWidth &&
            y < rect.bottom() && y >= rect.bottom() - borderWidth)
    {
        q->setCursor(Qt::SizeFDiagCursor);
        return true;
    }

    // top left()
    if (x >= rect.left() && x < rect.left() + borderWidth &&
            y >= rect.top() && y < rect.top() + borderWidth)
    {
        q->setCursor(Qt::SizeFDiagCursor);
        return true;
    }

    // top right()
    if (x < rect.right() && x >= rect.right() - borderWidth &&
            y >= rect.top() && y < rect.top() + borderWidth)
    {
        q->setCursor(Qt::SizeBDiagCursor);
        return true;
    }

    // left
    if (x >= rect.left() && x < rect.left() + borderWidth)
    {
        q->setCursor(Qt::SizeHorCursor);
        return true;
    }

    // right()
    if (x < rect.right() && x >= rect.right() - borderWidth)
    {
        q->setCursor(Qt::SizeHorCursor);
        return true;
    }

    // bottom()
    if (y < rect.bottom() && y >= rect.bottom() - borderWidth)
    {
        q->setCursor(Qt::SizeVerCursor);
        return true;
    }

    // top()
    if (y >= rect.top() && y < rect.top() + borderWidth)
    {
        q->setCursor(Qt::SizeVerCursor);
        return true;
    }

    q->setCursor(Qt::ArrowCursor);
    return false;
}

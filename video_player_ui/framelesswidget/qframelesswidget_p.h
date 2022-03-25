#ifndef QFRAMELESSWIDGET_P_H
#define QFRAMELESSWIDGET_P_H

#include "video_pimpl.h"
#include <QImage>
#include <QPoint>
#include <QWidget>

class QFrameLessWidget;
class QFrameLessWidgetPrivate: public VP_Data<QFrameLessWidget>
{
    VP_DECLARE_PUBLIC(QFrameLessWidget)
public:
    QFrameLessWidgetPrivate(QFrameLessWidget*);
    void setResizeable(bool);
    bool resizeable();
    virtual bool isCapture();
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual bool check(void* message, long* result);
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result,  QWidget*);
protected:
    bool checkMouseDirection(const QPoint& cursor);

protected:
    QImage m_bkImg;
    QPoint m_press, m_pos;

    bool m_bResizeabe;
    bool m_bTopWindow;
    bool m_bDragEnable;
    bool m_bDrag;
    Qt::WindowFlags m_flags;
};

#endif // QFRAMELESSWIDGET_P_H

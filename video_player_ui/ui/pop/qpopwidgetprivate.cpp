#include "qpopwidgetprivate.h"
#include <QLabel>
#include <QTimer>
QPopWidgetPrivate::QPopWidgetPrivate(QPopWidget* parent)
    : CNativeEvent_p(parent)
    , QCreateItem()
{
}

void QPopWidgetPrivate::setKey(const QString &sKey)
{
    m_sKey = sKey;
}

QString QPopWidgetPrivate::key()
{
    return m_sKey;
}
#include <QDebug>

void QPopWidgetPrivate::init(QWidget* parent)
{
    VP_Q(QPopWidget);
#ifdef Q_OS_WIN
    QTimer::singleShot(0, q, [=]{
        auto hwnd = reinterpret_cast<HWND>(q->winId());
        setAreo(hwnd);
        setShadow(hwnd);
    });
#endif
    setResizeable(false);
//    q->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
//    q->setAttribute(Qt::WA_ShowModal, true);

    q->setDragSelf(true);
    q->connect(parent, SIGNAL(pop(const QString&)), q, SLOT(onShow(const QString&)));
    q->connect(q, &QPopWidget::showInit, q, &QPopWidget::onShowCenter);
}

bool QPopWidgetPrivate::isCapture()
{
    return isTitleCapture();
}

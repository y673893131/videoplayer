#include "qinputfilter.h"
#include <QApplication>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QWidget>

QInputFilter* QInputFilter::s_instance=nullptr;

QInputFilter *QInputFilter::instance(QObject* parent)
{
    if(!s_instance)
        s_instance = new QInputFilter(parent);
    return s_instance;
}

QInputFilter::QInputFilter(QObject* parent)
    :QObject(parent)
{
    qApp->installEventFilter(this);
}

void QInputFilter::appendExceptObj(QObject *obj)
{
    m_exceptObjs.insert(obj);
}

bool QInputFilter::eventFilter(QObject *watched, QEvent *event)
{
    if(m_exceptObjs.contains(watched))
    {
        switch (event->type()) {
        case QEvent::Wheel:
        {
            if(watched && watched->objectName() != "toolWd")
            {
                auto keyEvent = reinterpret_cast<QWheelEvent*>(event);
                auto step = keyEvent->angleDelta().y();
                bool bInCrease = step > 0;
                emit volumeJump(bInCrease);
            }
        }break;
        default:
            break;
        }

        return QObject::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::MouseMove:
        emit mouseMove();
        return QObject::eventFilter(watched, event);
    case QEvent::KeyPress:
    {
        if(watched && (watched->inherits("QLineEdit")
                       || watched->objectName() == "toolWdWindow"))
        {
            return QObject::eventFilter(watched, event);
        }

        auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Escape:
            emit escap();
            break;
        case Qt::Key_Left:
            emit progressJump(false);
            break;
        case Qt::Key_Right:
            emit progressJump(true);
            break;
        case Qt::Key_Up:
            emit volumeJump(true);
            break;
        case Qt::Key_Down:
            emit volumeJump(false);
            break;
        case Qt::Key_Space:
            emit space();
            break;
        case Qt::Key_F3:
            emit cap();
            break;
        default:
            return QObject::eventFilter(watched, event);
        }
        return true;
    }
    default:
        return QObject::eventFilter(watched, event);
    }
}

#include "qfilelistdelegate.h"
#include <QPainter>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include "qplayfilelistmodel.h"

QFileListDelegate::QFileListDelegate(QObject* parent)
    :QStyledItemDelegate(parent), m_model(nullptr)
{
}

void QFileListDelegate::setModel(QPlayFileListModel *model)
{
    m_model = model;
}

bool QFileListDelegate::inCloseArea(const QRect &rc, const QPoint &pt) const
{
    return QRect(rc.right() - 17, rc.y(), 12, rc.height()).contains(pt);
}

void QFileListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    QStyledItemDelegate::paint(painter, option, index);
    if(option.state.testFlag(QStyle::State_MouseOver))
    {
        painter->save();
        auto rc = QRect(option.rect.x() + option.rect.width() - 12 - 5, option.rect.y() + (option.rect.height() -12) / 2, 12, 12);
        auto p = (QWidget*)parent();
        auto pos = p->mapFromGlobal(QCursor::pos());
        auto pen = QPen(QColor(255,0,0));

        pen.setWidth(2);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter->setPen(pen);

        QPoint points[4] = {
            QPoint(rc.topLeft()),
            QPoint(rc.bottomRight()),
            QPoint(rc.topRight()),
            QPoint(rc.bottomLeft()),
        };

        if(inCloseArea(option.rect, pos))
        {
            points[0] += QPoint(1,1);
            points[1] += QPoint(-1,-1);
            points[2] += QPoint(-1,1);
            points[3] += QPoint(1,-1);
        }
        painter->drawLines(points, 2);
        painter->restore();
    }

    if(m_model->isSelected(index))
    {
        painter->save();
        auto rc = QRect(option.rect.x() + 10, option.rect.y() + (option.rect.height() - 10) / 2, 8, 10);
        QVector<QPointF> point;
        point << QPoint(rc.topLeft())
            << QPoint(rc.bottomRight() + QPoint(0, -rc.height() / 2))
            << QPoint(rc.bottomLeft());
#define PLAY_FILL_ICON
#ifndef PLAY_FILL_ICON
        point << QPoint(rc.topLeft());
        QPainterPath path;
        path.addPolygon(point);
        painter->setPen(QColor("#FF5C38"));
        painter->drawPath(path);
#else
        QPainterPath path;
        path.addPolygon(point);
        painter->fillPath(path, QColor("#FF5C38"));
#endif
        painter->restore();
    }
}

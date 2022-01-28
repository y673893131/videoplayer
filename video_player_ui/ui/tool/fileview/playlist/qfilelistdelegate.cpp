#include "qfilelistdelegate.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QStyle>
#include <QScrollBar>
#include "qplayfilelistmodel.h"
#include "qfilelistview.h"

QFileListDelegate::QFileListDelegate(QObject* parent)
    :QStyledItemDelegate(parent), m_model(nullptr)
{
    m_defaultColor = "#d5d5d5";
    m_selectColor = "#FF5C38";
    m_font.setFamily("Microsoft YaHei");
    m_font.setPixelSize(13);
}

void QFileListDelegate::setModel(QPlayFileListModel *model)
{
    m_model = model;
}

bool QFileListDelegate::inCloseArea(const QWidget* widget, const QRect &rc, const QPoint &pt) const
{
    auto w = qobject_cast<const QFileListView*>(widget);
    auto bar = w->verticalScrollBar();
    int barWidth = bar->isVisible() ? bar->width() : 0;
    return QRect(widget->rect().right() - 15 - barWidth, rc.y(), 12, rc.height()).contains(pt);
}

void QFileListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    auto fileList = reinterpret_cast<const QFileListView*>(option.widget);

    auto isSelect = m_model->isSelected(index);
    auto isMouseOver = fileList->isMouseOver(index);
    viewOption.state.setFlag(QStyle::State_MouseOver, isMouseOver);

    painter->setRenderHint(QPainter::Antialiasing, true);

    //background
    auto style = option.widget->style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &viewOption, painter, viewOption.widget);

    //text option
    int padding = 10;
    auto role = QPalette::Text;
    if(viewOption.state.testFlag(QStyle::State_MouseOver) || isSelect)
    {
        padding = 20;
        role = QPalette::HighlightedText;
        viewOption.palette.setColor(QPalette::HighlightedText, m_selectColor);
    }
    else
    {
        viewOption.palette.setColor(QPalette::Text, m_defaultColor);
    }

    viewOption.rect.setX(padding);

    //text
    painter->setFont(m_font);
    style->drawItemText(painter, viewOption.rect,
                        viewOption.displayAlignment, viewOption.palette,
                        true, viewOption.text, role);

    auto rect = option.widget->rect();
    rect.setY(option.rect.y());
    rect.setHeight(option.rect.height());
    if(option.state.testFlag(QStyle::State_MouseOver))
    {
        painter->save();
        auto verBar = fileList->verticalScrollBar();
        int nVerBarWidth = verBar->isVisible() ? verBar->width() : 0;
        auto rc = QRect(rect.x() + rect.width() - 10 - 5 - nVerBarWidth, rect.y() + (rect.height() -10) / 2, 10, 10);
        auto p =  reinterpret_cast<QWidget*>(parent());
        auto pos = p->mapFromGlobal(QCursor::pos());
        auto pen = QPen(Qt::lightGray);

        pen.setWidth(2);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);

        QPoint points[4] = {
            QPoint(rc.topLeft()),
            QPoint(rc.bottomRight()),
            QPoint(rc.topRight()),
            QPoint(rc.bottomLeft()),
        };

        if(inCloseArea(option.widget, rc, pos))
        {
            pen.setColor(m_selectColor);
            points[0] += QPoint(1,1);
            points[1] += QPoint(-1,-1);
            points[2] += QPoint(-1,1);
            points[3] += QPoint(1,-1);
        }

        painter->setPen(pen);
        painter->drawLines(points, 2);
        painter->restore();
    }

    if(isSelect)
    {
        painter->save();
        auto rc = QRect(rect.x() + 10, rect.y() + (rect.height() - 10) / 2, 10, 12);
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
        painter->fillPath(path, m_selectColor);
#endif
        painter->restore();
    }
}

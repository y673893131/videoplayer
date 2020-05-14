#include "qfilelistdelegate.h"
#include <QPainter>
#include <QDebug>
QFileListDelegate::QFileListDelegate(QObject* parent)
    :QStyledItemDelegate(parent)
{

}

void QFileListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing, true);

//    qDebug() << index << (option.state);
    __super::paint(painter, option, index);
}

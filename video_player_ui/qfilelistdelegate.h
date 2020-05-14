#ifndef QFILELISTDELEGATE_H
#define QFILELISTDELEGATE_H

#include <QStyledItemDelegate>

class QFileListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QFileListDelegate(QObject* parent);
private:
    void paint(QPainter *painter,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // QFILELISTDELEGATE_H

#ifndef QFILELISTDELEGATE_H
#define QFILELISTDELEGATE_H

#include <QStyledItemDelegate>

class QPlayFileListModel;
class QFileListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QFileListDelegate(QObject* parent);
    void setModel(QPlayFileListModel* model);
    bool inCloseArea(const QRect&, const QPoint&) const;
private:
    void paint(QPainter *painter,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QPlayFileListModel* m_model;
};

#endif // QFILELISTDELEGATE_H

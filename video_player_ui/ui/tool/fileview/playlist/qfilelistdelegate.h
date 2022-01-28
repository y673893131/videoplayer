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
    bool inCloseArea(const QWidget*, const QRect&, const QPoint&) const;
private:
    void paint(QPainter *painter,
            const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QPlayFileListModel* m_model;
    QColor m_defaultColor;
    QColor m_selectColor;
    QFont m_font;
};

#endif // QFILELISTDELEGATE_H

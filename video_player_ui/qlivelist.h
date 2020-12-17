#ifndef QLIVELIST_H
#define QLIVELIST_H

#include <QTreeView>

class QLiveList : public QTreeView
{
    Q_OBJECT
public:
    QLiveList(QWidget* parent = nullptr);
signals:
    void select(const QModelIndex&);
private slots:
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // QLIVELIST_H

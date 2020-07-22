#ifndef QFILELISTVIEW_H
#define QFILELISTVIEW_H

#include <QListView>
class QFileListView : public QListView
{
    Q_OBJECT
public:
    QFileListView(QWidget* parent);

    bool isVerticalUnder();
signals:
    void select(const QString&);
    void addLocalUrl(const QString&);
public slots:

    // QWidget interface
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // QFILELISTVIEW_H

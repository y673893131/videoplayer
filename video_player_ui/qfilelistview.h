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
    void loadFile();
    void select(const QString&);
    void addLocalUrl(const QString&);
    void remove(const QModelIndex&);
    void rightClick(const QPoint&);
    void inputUrlFile(const QString&);
    void filter(const QString&);
    // QWidget interface
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
};

#endif // QFILELISTVIEW_H

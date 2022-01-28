#ifndef QFILELISTVIEW_H
#define QFILELISTVIEW_H

#include <QListView>
class QFileListMenu;
class QAction;
class QFileListView : public QListView
{
    Q_OBJECT
public:
    QFileListView(QWidget* parent);

    bool isVerticalUnder();
    bool isMouseOver(const QModelIndex&) const;
signals:
    void loadFile();
    void select(const QString&);
    void addLocalUrl(const QStringList&);
    void remove(const QModelIndex&);
    void rightClick(const QPoint&);
    void inputUrlFile(const QString&);
    void filter(const QString&);
public slots:
    void onAddUrlSuccessed(const QString& url);
    void onPopMenu(const QPoint& point);
    void onBrowerFile();
    void onDeleteFile();
    void onEnd();
    // QWidget interface
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    QFileListMenu* m_menu;
};

#endif // QFILELISTVIEW_H

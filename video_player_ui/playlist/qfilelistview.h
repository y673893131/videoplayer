#ifndef QFILELISTVIEW_H
#define QFILELISTVIEW_H

#include <QListView>
class QPlayMenuBase;
class QAction;
class QFileListView : public QListView
{
    Q_OBJECT
public:
    QFileListView(QWidget* parent);

    bool isVerticalUnder();
    bool isMouseOver(const QModelIndex&);
signals:
    void loadFile();
    void select(const QString&);
    void addLocalUrl(const QString&);
    void remove(const QModelIndex&);
    void rightClick(const QPoint&);
    void inputUrlFile(const QString&);
    void filter(const QString&);
public slots:
    void onAddUrlSuccessed(const QString& url);
    void onPopMenu(const QPoint& point);
    void onBrowerFile();
    void onEnd();
    // QWidget interface
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    QPlayMenuBase* m_menu;
    QAction* m_openDir;
    QAction* m_loadFile;
};

#endif // QFILELISTVIEW_H

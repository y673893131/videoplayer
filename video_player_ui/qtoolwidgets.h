#ifndef QTOOLWIDGETS_H
#define QTOOLWIDGETS_H

#include <QWidget>

class QBoxLayout;
class QListView;
class QPushButton;
class QToolWidgets : public QWidget
{
    Q_OBJECT
public:
    explicit QToolWidgets(QWidget *parent = nullptr);

    bool isUnderValid();
signals:
    void play(const QString& = QString());
    void pause();
    void stop();
    void mute(bool);
public slots:
    void selectMode(int);
private:
    QBoxLayout* CreateTitle(QWidget*);
    QBoxLayout* CreateCenterToolbar(QWidget*);
    QBoxLayout* CreateProcessbar(QWidget*);
    QBoxLayout* CreateToolbar(QWidget*);
    QWidget* CreateFilelist(QWidget*);
private:
    QWidget* m_filelistWd;
    QListView* m_filelist;
    QPushButton *m_openfile,* m_filelistIndicator;
};

#endif // QTOOLWIDGETS_H

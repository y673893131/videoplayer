#ifndef QTOOLWIDGETS_H
#define QTOOLWIDGETS_H

#include <QWidget>

class QBoxLayout;
class QFileListView;
class QPushButton;
class QProgressSlider;
class QDataModel;
class QInputUrlWidget;
class QDouyuWidget;
class QLivePlatformManager;
class QToolWidgets : public QWidget
{
    Q_OBJECT
private:
    enum Live_Platform
    {
        Live_DouYu = 0,
        Live_HuYa,


        Live_Max
    };

public:
    explicit QToolWidgets(QWidget *parent = nullptr);

    bool isUnderValid();
    int index();
signals:
    void exit();
    void play(const QString& = QString());
    void pause();
    void continuePlay();
    void stop();
    void setVol(int);
    void mute(bool);
    void loadFile();
    void start(int);
    void move();
    void hideOrShow(bool);
    void setTotalSeconds(int);
    void setPosSeconds(int);
    void setSeekPos(int);
    void selectMode(int);
    void showMenu();
    void viewAdjust(bool);
    void topWindow(bool);
    void frameRate(int video);
    void inputUrl();
    void inputUrlFile(const QString&);
public slots:
    void onLoadFile();
    void onSelectMode(int);
private:
    QWidget* CreateTitle(QWidget*);
    QBoxLayout* CreateCenterToolbar(QWidget*);
    QBoxLayout* CreateProcessbar(QWidget*);
    QWidget* CreateToolbar(QWidget*);
    QWidget* CreateLeftlist(QWidget*);
    QWidget* CreateFilelist(QWidget*);

    void CreateMenu(QWidget *parent);
    void mousePressEvent(QMouseEvent *event);
private:
    QProgressSlider* m_process;
    QWidget* m_livePlatformWd,* m_filelistWd;
    QDouyuWidget* m_douyu;
    QLivePlatformManager* m_platformManager;
    QFileListView* m_filelist;
    QPushButton *m_openfile/*,* m_filelistIndicator*/;
    QInputUrlWidget* m_inputUrl;
    bool m_bPlaying;
    int m_index, m_playMode;
    QDataModel* m_data;
};

#endif // QTOOLWIDGETS_H

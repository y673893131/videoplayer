#ifndef QTOOLWIDGETS_H
#define QTOOLWIDGETS_H

#include <QWidget>
#include "framelesswidget/framelesswidget.h"

struct sub_title_info
{
    sub_title_info()
        : tmBeg(0)
        , tmEnd(0)
        , bShow(false)
    {

    }
    int tmBeg;
    int tmEnd;
    bool bShow;
    std::map<QString, QString> titls;
};

class QLabel;
class QBoxLayout;
class QFileListView;
class QPushButton;
class QProgressSlider;
class QDataModel;
class QInputUrlWidget;
class QDouyuWidget;
class QLivePlatformManager;
class QVideoControl;

#ifdef Q_OS_WIN
class QToolWidgets : public QWidget, public CNativeEvent_Win, public QAbstractNativeEventFilter
#else
class QToolWidgets : public QWidget, public QAbstractNativeEventFilter
#endif
{
    Q_OBJECT
public:
    explicit QToolWidgets(QWidget *parent = nullptr);

    int index();
    void setExists(bool);
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
    void mouseMove();
    void hideOrShow(bool);
    void setTotalSeconds(int);
    void setPosSeconds(int);
    void setSeekPos(int);
    void selectMode(int);
    void showMenu();
    void viewAdjust(bool);
    void topWindow(bool);
    void frameRate(int);
    void inputUrl();
    void inputUrlFile(const QString&);
    void _move(const QPoint&);
    void _resize(const QSize&);
    void getPreview(int);
    void _preview(void*, int, int);
public slots:
    void onLoadFile();
    void onSelectMode(int);
    void onLeftPress();
    void onMax();
    void onFull();
    void onSubtitle(const QString&);
private:
    void init(QWidget* parent);
    void initStyle();
    void initUi(QWidget* parent);
    void initSize();

    QWidget* CreateTitle(QWidget*);
    QBoxLayout* CreateCenterToolbar(QWidget*);
    QBoxLayout* CreateProcessbar(QWidget*);
    QWidget* CreateToolbar(QWidget*);
    QWidget* CreateLeftlist(QWidget*);
    QWidget* CreateFilelist(QWidget*);
    QWidget* CreateSubTitle(QWidget*);

    void CreateMenu(QWidget *parent);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
private:
    QProgressSlider* m_process;
    QWidget* m_backWd,* m_titleWd, * m_livePlatformWd,* m_filelistWd, *m_subtitleWd, *m_toolWd;
    QDouyuWidget* m_douyu;
    QLivePlatformManager* m_platformManager;
    QFileListView* m_filelist;
    QPushButton *m_openfile/*,* m_filelistIndicator*/,*m_min,*m_max,*m_close;
    QInputUrlWidget* m_inputUrl;
    bool m_bPlaying, m_bLocalFile;
    int m_index, m_playMode, m_totalSeconds;
    QDataModel* m_data;
    QLabel* m_title;
    QString m_curRender;

    QVideoControl* m_contorl;
    sub_title_info m_subtitle;
    QLabel* m_ch;
    QLabel* m_other;
};

#endif // QTOOLWIDGETS_H

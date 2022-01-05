#ifndef QTOOLWIDGETS_H
#define QTOOLWIDGETS_H

#include <QWidget>
#include "framelesswidget/framelesswidget.h"

class QLabel;
class QBoxLayout;
class QPushButton;
class QDataModel;
class QInputUrlWidget;
class QDouyuWidget;
class QLivePlatformManager;
class QVideoControl;
class QMenu;
class QActionGroup;
class QToolBase;
class QPlayMenu;
#ifdef Q_OS_WIN
class QToolWidgets : public QWidget, public CNativeEvent_Win, public QAbstractNativeEventFilter
#else
class QToolWidgets : public QWidget, public QAbstractNativeEventFilter
#endif
{
    Q_OBJECT
public:
    enum tools
    {
        tool_play_title,
        tool_file_list,
        tool_play_control,
        tool_play_progress,
        tool_live_platform,
        tool_play_subtitle,
        tool_play_output,

        tool_max
    };

public:
    explicit QToolWidgets(QWidget *parent = nullptr);

    int index();
    void setExists(bool);
signals:
    void showMin();
    void exit();
    void load(const QString& = QString());
    void loadFile();
    void start(int);
//    void mouseMove();
    void hideOrShow(bool);
    void moveShowPlatform();
    void setTotalSeconds(int);
    void showMenu();
    void viewAdjust(bool);
    void topWindow(bool);
    void inputUrl();
    void inputUrlFile(const QString&);
    void _move(const QPoint&);
    void _resize(const QSize&);
public slots:
    void onLoadFile();
    void onLeftPress();
    void onMax();
    void onFull();
    void onAutoVisable(bool bHide);
private:
    void init(QWidget* parent);
    void initStyle();
    void initUi(QWidget* parent);
    void initLayout();
    void initSize();
    void initConnect();

    void CreateCenterToolbar();

    void CreateMenu(QWidget* parent);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QWidget* m_backWd;
    QToolBase* m_tools[tool_max];
    QPushButton *m_openfile/*,* m_filelistIndicator*/,*m_min,*m_max,*m_close;
    QInputUrlWidget* m_inputUrl;
    bool m_bLocalFile;
    int m_index, m_playMode, m_totalSeconds;
    QDataModel* m_data;

    QPlayMenu* m_playMenu;

    QVideoControl* m_contorl;
    QTimer* m_autoHidetimer;
};

#endif // QTOOLWIDGETS_H

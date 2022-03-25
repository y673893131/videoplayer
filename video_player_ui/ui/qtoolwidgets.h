#ifndef QTOOLWIDGETS_H
#define QTOOLWIDGETS_H

#include "framelesswidget/framelesswidget.h"

class QToolWidgetsPrivate;
#ifdef Q_OS_WIN
class QToolWidgets : public QFrameLessWidget, public QAbstractNativeEventFilter
#else
class QToolWidgets : public QFrameLessWidget
#endif
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QToolWidgets)
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

//    int index();
    void setExists(bool);
signals:
    void showMin();
    void showNor();
    void showFull();
    void exit();
    void load(const QStringList& = QStringList());
    void loadFile();
    void start(int);
//    void mouseMove();
    void hideOrShow(bool);
    void moveShowPlatform();
    void setTotalSeconds(int);
    void showMenu();
    void viewAdjust(bool);
    void topWindow(bool);
    void pop(const QString&);
    void inputUrlFile(const QString&);
    void _move(const QPoint&);
    void _resize(const QSize&);
    void thumb(int);
    void cmd(int, const QString&);
    void playSize(int);
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

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
#ifdef Q_OS_WIN
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

#endif // QTOOLWIDGETS_H

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <framelesswidget/framelesswidget.h>
#include <QDateTime>
class QGLVideoWidget;
class video_player_core;
class QToolWidgets;
class Widget : public QFrameLessWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    // video_interface interface
public:
    void endCall();
private:
    video_player_core* m_core;
    QGLVideoWidget* m_video;
    QToolWidgets* m_toolbar;
    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    virtual bool isValid();
private slots:
    void flushSheetStyle();
signals:
    void inputUrlFile(const QString&);
private:
    QDateTime m_last;

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};
#endif // WIDGET_H

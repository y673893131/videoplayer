#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDateTime>
#include "framelesswidget/framelesswidget.h"
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
    virtual bool isValid();
private slots:
    void flushSheetStyle();
private:
    QDateTime m_last;
};
#endif // WIDGET_H

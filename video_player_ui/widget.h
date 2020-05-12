#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "framelesswidget/framelesswidget.h"
class QLabel;
class video_player_core;
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
    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
};
#endif // WIDGET_H

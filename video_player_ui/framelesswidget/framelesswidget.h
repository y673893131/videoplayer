#ifndef QFrameLessWidget_H
#define QFrameLessWidget_H

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include "nativeevent_win.h"
#include "util.h"

class QDragBorder;
#ifdef Q_OS_WIN
class QFrameLessWidget : public QWidget, public CNativeEvent_Win
#else
class QFrameLessWidget : public QWidget
#endif
{
    Q_OBJECT
signals:
    void leftPress();
    void rightClicked();
    void leftDoubleClicked();
public:
    QFrameLessWidget(QWidget *parent = nullptr);
    virtual ~QFrameLessWidget() override;

    void setDragSelf(bool);
    void setDoubleClickFullScreen();
    void setDoubleClickMax();
    // QWidget interface
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    virtual bool isValid();
    void updateTopWindow();
protected:
    bool m_bTopWindow;
private:
    QImage m_bkImg;
    QDragBorder* m_dragBorder;
};
#endif // QFrameLessWidget_H

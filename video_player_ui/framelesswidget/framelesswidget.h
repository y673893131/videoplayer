#ifndef QFrameLessWidget_H
#define QFrameLessWidget_H

#include <QWidget>
#include <QAbstractNativeEventFilter>
#ifdef WIN32
#include <windows.h>
#endif
class QDragBorder;
class QFrameLessWidget : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT

#define PADDING 10
    enum Drag_Move
    {
        DragMove_None = 0,
        DragMove_Left,
        DragMove_Right,
        DragMove_Top,
        DragMove_Restore
    };

public:
    QFrameLessWidget(QWidget *parent = nullptr);
    ~QFrameLessWidget();

private:
    void resizeBackground(int w, int h, int round, int margin, QColor color);
    void region(const QPoint &cursorGlobalPoint, bool &activeFlag);
    void checkMouseMoveRegion();
    bool checkLButtonPressRegion();
    void checkSize(void*);
    void checkDragMove(void*);

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void setBackgroundColor(QColor);
    virtual bool isValid();
    bool isFullScreen();
    void showFullScreen();
    void showNormal();
    void updateTopWindow();
protected:
    bool m_bTopWindow;
private:
    QColor m_bkColor;
    QImage m_bkImg;
    int m_dir;
    Drag_Move m_drag;
    QSize m_normalSize,m_minSize;
    QRect m_dragRc;
    QPoint m_dragLastPos,m_doubleClickPos;
    QDragBorder* m_dragBorder;

    // QWidget interface
protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};
#endif // QFrameLessWidget_H

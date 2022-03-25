#ifndef QFrameLessWidget_H
#define QFrameLessWidget_H

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include "util.h"
#include "video_pimpl.h"

class QFrameLessWidgetPrivate;
class QFrameLessWidget : public QWidget
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(QFrameLessWidget)
protected:
    VP_DECLARE(QFrameLessWidget)
signals:
    void leftPress();
    void rightClicked();
    void leftDoubleClicked();
    void moved(const QPoint&);
    void resized(const QSize&);
public slots:
    void onMoved(const QPoint&);
    void onResized(const QSize&);
public:
    explicit QFrameLessWidget(QWidget *parent = nullptr);
    QFrameLessWidget(QFrameLessWidgetPrivate*, QWidget *parent = nullptr);
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
    void setTopWindow(bool);
};
#endif // QFrameLessWidget_H

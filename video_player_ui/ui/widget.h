#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <framelesswidget/framelesswidget.h>
#include <QDateTime>

class WidgetPrivate;
#ifdef Q_OS_WIN
#include "ui/thumb/qwinthumbnail.h"
class Widget : public QWinThumbnail
#else
class Widget : public QFrameLessWidget
#endif
{
    Q_OBJECT
    VP_DECLARE_PRIVATE(Widget)
public:
    Widget(QWidget *parent = nullptr);
    ~Widget() override;

signals:
    void inputUrlFile(const QString&);

private slots:
    void onExit();
    void onTopWindow(bool);
    void onFlushSheetStyle();
private:
    void init();
    void initData();
    void initStyle();
    void initResource();
    void initConnect();
    void flushQss();
    void flushInitSize();
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};
#endif // WIDGET_H

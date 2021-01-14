#ifndef QDOUYUWIDGET_H
#define QDOUYUWIDGET_H

#include "framelesswidget/framelesswidget.h"
class QTimer;
class QPushButton;
class QDouyuWidget : public QFrameLessWidget
{
    Q_OBJECT
public:
    explicit QDouyuWidget(QWidget *parent = nullptr);

signals:
    void play(const QString&);
    void showIndex(int);

private:
    void init(QWidget* parent);
    void initStyle();
    void initUi(QWidget* parent);
    // QObject interface
private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool isValid() override;
    void showEvent(QShowEvent *event) override;
private:
    QTimer* m_timer;
    QWidget* m_title;
    QPushButton* m_close;
};

#endif // QDOUYUWIDGET_H

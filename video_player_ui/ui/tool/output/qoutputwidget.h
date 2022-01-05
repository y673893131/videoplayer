#ifndef QOUTPUTWIDGET_H
#define QOUTPUTWIDGET_H

#include "../base/qtoolbase.h"

class QLabel;
class QOutputWidget : public QToolBase
{
    Q_OBJECT
public:
    enum label
    {
        label_info,

        label_max
    };


public:
    explicit QOutputWidget(QWidget *parent = nullptr);

private:
    void initUi();
    void initLayout();
    void initConnect() override;

public slots:
    void onDelay();
    void onInfo(const QString&);
private:
    QLabel* m_label[label_max];

    QTimer* m_timerDelay;
};

#endif // QOUTPUTWIDGET_H

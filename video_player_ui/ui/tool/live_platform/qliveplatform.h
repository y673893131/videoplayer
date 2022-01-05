#ifndef QLIVEPLATFORM_H
#define QLIVEPLATFORM_H

#include "../base/qtoolbase.h"

class QPushButton;
class QLivePlatformManager;
class QDouyuWidget;
class QLivePlatform : public QToolBase
{
    Q_OBJECT

    enum button
    {
        button_close,

        button_max
    };

public:
    explicit QLivePlatform(QWidget *parent = nullptr);

private:
    void initUi();
    void initLayout();
    void initConnect() override;
private slots:
    void onAutoVisable(bool) override;
    void onMoveShow();
private:
    QLivePlatformManager* m_platformManager;
    QPushButton* m_button[button_max];
    QDouyuWidget* m_dy;
};

#endif // QLIVEPLATFORM_H

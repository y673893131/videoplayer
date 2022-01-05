#ifndef QPLAYPROGRESS_H
#define QPLAYPROGRESS_H

#include "../base/qtoolbase.h"

class QProgressSlider;
class QPlayProgress : public QToolBase
{
    Q_OBJECT
public:
    explicit QPlayProgress(QWidget* parent = nullptr);

private:
    void initUi();
    void initLayout();
    void initConnect() override;
private slots:
    void onTotalTime(int);
    void onSetProgress(int);
    void onEnd();
    void onAutoVisable(bool) override;
private:
    QProgressSlider* m_progress;
};

#endif // QPLAYPROCESS_H

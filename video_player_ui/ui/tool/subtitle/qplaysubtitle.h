#ifndef QPLAYSUBTITLE_H
#define QPLAYSUBTITLE_H

#include "video_pimpl.h"
#include "../base/qtoolbase.h"

class QPlaySubtitlePrivate;
class QPlaySubtitle : public QToolBase
{
    Q_OBJECT
    VP_DECLARE(QPlaySubtitle)
    VP_DECLARE_PRIVATE(QPlaySubtitle)

public:
    explicit QPlaySubtitle(QWidget *parent = nullptr);
    ~QPlaySubtitle() override;
private:
    void initUi();
    void initLayout();
    void initConnect() override;
    void paintEvent(QPaintEvent *event) override;
public slots:
    void onGotoPos();
    void onDelayClear();
    void onPos(int);
    void onChannelModify();
    void onSetEngine(int);
};

#endif // QPLAYSUBTITLE_H

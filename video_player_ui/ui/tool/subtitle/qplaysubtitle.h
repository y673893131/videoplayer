#ifndef QPLAYSUBTITLE_H
#define QPLAYSUBTITLE_H

#include "../base/qtoolbase.h"
#include "video_player_core.h"
#include <stdint.h>

struct sub_title_time
{
    sub_title_time()
        : tmBeg(0)
        , tmEnd(0)
    {

    }
    int64_t tmBeg;
    int64_t tmEnd;
};

class QLabel;
class QPlaySubtitle : public QToolBase
{
    Q_OBJECT
public:
    explicit QPlaySubtitle(QWidget *parent = nullptr);

private:
    void initUi();
    void initLayout();
    void initConnect() override;
    void resizeEvent(QResizeEvent *event) override;
public slots:
    void onDelayClear();
    void onPos(int);
    void onChannelModify();
private slots:
    void onSubTitleHeader(const subtitle_header&);
    void onSubtitle(const QString &str, unsigned int index, int type, int64_t start, int64_t end);
private:
    std::vector</*QSubTitleLabel*/QLabel*> m_label;
    std::map<QString, int> m_nameToIndex;
    QTimer* m_timerDisplay;
    sub_title_time m_delay;
};

#endif // QPLAYSUBTITLE_H

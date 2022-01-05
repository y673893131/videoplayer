#ifndef QPLAYSUBTITLE_H
#define QPLAYSUBTITLE_H

#include "../base/qtoolbase.h"

struct sub_title_info
{
    sub_title_info()
        : tmBeg(0)
        , tmEnd(0)
        , bShow(false)
    {

    }
    int tmBeg;
    int tmEnd;
    bool bShow;
    std::map<QString, QString> titls;
};

class QLabel;
class QPlaySubtitle : public QToolBase
{
    Q_OBJECT
public:
    enum label
    {
        label_main,
        label_sub,

        label_max
    };

public:
    explicit QPlaySubtitle(QWidget *parent = nullptr);

private:
    void initUi();
    void initLayout();
    void initConnect() override;
    void resizeEvent(QResizeEvent *event) override;
public slots:
    void onDelayClear();
    void onChannelModify();
private slots:
    void onSubtitle(const QString &str, unsigned int index, int type);
private:
    QLabel* m_label[label_max];
    QTimer* m_timerDisplay;
    sub_title_info m_subtitle;
};

#endif // QPLAYSUBTITLE_H

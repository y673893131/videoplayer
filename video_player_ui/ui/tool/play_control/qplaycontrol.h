#ifndef QPLAYCONTROL_H
#define QPLAYCONTROL_H

#include "../base/qtoolbase.h"
class QPushButton;
class QLabel;
class QTimer;
class QPlayControl : public QToolBase
{
    Q_OBJECT
public:
    enum button
    {
        button_stop,
        button_prev,
        button_play_or_continue,
        button_next,
        button_vol_mute,
        button_file_list,

        button_max
    };

    enum label
    {
        label_time,
        label_rate,
        label_max
    };

public:
    explicit QPlayControl(QWidget* parent = nullptr);

private:
    void initUi();
    void initLayout();
    void initConnect() override;

signals:
    void play();
    void pause();
    void continuePlay();
    void loadOrPlay();

    void next();
    void priv();
    void stop();
    void showVolume(bool, const QPoint&, const QSize&);

private slots:
    void onPlay();
    void onPause();
    void onPlayOrPause();
    void onStop();
    void onMute(bool);
    void onRate(int);
    void onLoadConfig();
    void onTotal(int);
    void onUpdateCurrentTime(int);
private:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    QPushButton* m_button[button_max];
    QLabel* m_label[label_max];
    bool m_bPlaying;
    int m_nTotal;
};

#endif // QPLAYCONTROL_H

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
        button_mode,
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
    void load();

    void next();
    void priv();
    void stop();
    void showVolume(bool, const QPoint&, const QSize&);

public slots:
    void onPlayOrPause();

private slots:
    void onPlay();
    void onPause();
    void onStop();
    void onMute(bool);
    void onRate(int);
    void onLoadConfig();
    void onSetConfig();
    void onTotal(int);
    void onUpdateCurrentTime(int);
    void onMode();
private:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    QPushButton* m_button[button_max];
    QLabel* m_label[label_max];
    bool m_bPlaying;
    int m_nTotal;
    QString m_sTotal;
};

#endif // QPLAYCONTROL_H

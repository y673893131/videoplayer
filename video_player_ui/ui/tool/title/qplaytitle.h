#ifndef QPLAYTITLE_H
#define QPLAYTITLE_H

#include "../base/qtoolbase.h"

class QPushButton;
class QLabel;
class QPlayTitle : public QToolBase
{
    Q_OBJECT

public:
    enum button
    {
        button_minimize,
        button_maximize,
        button_close,

        button_max
    };


public:
    explicit QPlayTitle(QWidget *parent = nullptr);
private:
    void initUi();
    void initLayout();
    void initConnect() override;
public slots:
    void onPlay(const QString&);
private:
    QPushButton* m_button[button_max];
    QLabel* m_title;
};

#endif // QPLAYTITLE_H

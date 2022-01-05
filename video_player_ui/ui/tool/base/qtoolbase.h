#ifndef QTOOLBASE_H
#define QTOOLBASE_H

#include <QWidget>

class QToolBase : public QWidget
{
    Q_OBJECT
public:
    explicit QToolBase(QWidget *parent = nullptr, bool bAutoHide = true);

signals:

public slots:
    virtual void onAutoVisable(bool);
public:
    virtual void initConnect() = 0;
private:
    void paintEvent(QPaintEvent *event) override;
protected:
    QWidget* m_parent;
};

#endif // QTOOLBASE_H

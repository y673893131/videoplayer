#ifndef CNATIVEEVENT_WIN_H
#define CNATIVEEVENT_WIN_H

#include <QWidget>
#ifdef Q_OS_WIN

#include "qframelesswidget_p.h"

class CNativeEvent_Win : public QFrameLessWidgetPrivate
{
public:
    CNativeEvent_Win(QFrameLessWidget* parent = nullptr);
    ~CNativeEvent_Win() override;
    void setAreo(void*);
    void setShadow(void*);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result, QWidget* widget) override;
    virtual bool isCaption();
    bool check(void *message, long *result) override;
private:
    bool m_bShadow;
};

#endif
#endif // CNATIVEEVENT_WIN_H

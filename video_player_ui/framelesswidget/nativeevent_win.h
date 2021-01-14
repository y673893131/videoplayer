#ifndef CNATIVEEVENT_WIN_H
#define CNATIVEEVENT_WIN_H

#include <QWidget>
#ifdef Q_OS_WIN

#define PADDING 10
class CNativeEvent_Win
{
public:
    CNativeEvent_Win();
    virtual ~CNativeEvent_Win();
    void setAreo(void*);
    void setShadow(void*);
    void setResizeable(bool);
    bool _nativeEvent(const QByteArray &eventType, void *message, long *result, QWidget* widget);
    virtual bool isCaption();
    virtual bool check(void *message, long *result);
private:
    bool m_bResizeable, m_bShadow;
};

#endif
#endif // CNATIVEEVENT_WIN_H

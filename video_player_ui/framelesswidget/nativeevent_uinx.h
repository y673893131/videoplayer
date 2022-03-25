#ifndef CNATIVEEVENT_UINX_H
#define CNATIVEEVENT_UINX_H

#ifndef Q_OS_WIN
#include "qframelesswidget_p.h"

class CNativeEvent_Uinx : public QFrameLessWidgetPrivate
{
public:
    CNativeEvent_Uinx(QFrameLessWidget*);

    bool nativeEvent(const QByteArray &eventType, void *message, long *result, QWidget*) override;
    bool check(void* message, long* result) override;
};



#endif

#endif // CNATIVEEVENT_UINX_H

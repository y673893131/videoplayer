#ifndef CNATIVEEVENT_P_H
#define CNATIVEEVENT_P_H

#ifdef WIN32
#include "nativeevent_win.h"
#else
#include "nativeevent_uinx.h"
#endif

#ifdef WIN32
class CNativeEvent_p : public CNativeEvent_Win
#else
class CNativeEvent_p : public CNativeEvent_Uinx
#endif
{
public:
    CNativeEvent_p(QFrameLessWidget* parent);
};

#endif // CNATIVEEVENT_P_H

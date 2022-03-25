#include "nativeevent_p.h"

CNativeEvent_p::CNativeEvent_p(QFrameLessWidget* parent)
#ifdef WIN32
    : CNativeEvent_Win(parent)
#else
    : CNativeEvent_Uinx(parent)
#endif
{
}

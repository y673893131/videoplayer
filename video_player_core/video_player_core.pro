CONFIG -= qt

TEMPLATE = lib
DEFINES += VIDEO_PLAYER_CORE_LIBRARY
CONFIG += dll
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(core.pri)
# Default rules for deployment.
win32{
    DESTDIR = $$PWD/../bin
}

unix {
    target.path = /usr/lib
}

!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    core.pri

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../vidoe_player_log/release/ -lvidoe_player_log
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../vidoe_player_log/debug/ -lvidoe_player_log
else:unix: LIBS += -L$$OUT_PWD/../vidoe_player_log/ -lvidoe_player_log

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../vidoe_player_log/release/libvidoe_player_log.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../vidoe_player_log/debug/libvidoe_player_log.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../vidoe_player_log/release/vidoe_player_log.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../vidoe_player_log/debug/vidoe_player_log.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../vidoe_player_log/libvidoe_player_log.a

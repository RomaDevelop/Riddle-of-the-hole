QT -= gui

TEMPLATE = lib
DEFINES += MARATHONINTERFACE_LIBRARY


CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    marathoninterface.cpp \
    marathonchannel.cpp

HEADERS += \
    MarathonInterface_global.h \
    marathoninterface.h \
    marathonchannel.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DESTDIR = ..\bin
win32:CONFIG(release, debug|release): TARGET = MarathonInterfaceThrd
else:win32:CONFIG(debug, debug|release): TARGET = MarathonInterfaceThrdd

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

#begin connecting chai
win32: LIBS += -L$$PWD/../bin/ -lchai
#end connecting chai

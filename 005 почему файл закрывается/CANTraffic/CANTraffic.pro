QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cleanerthread.cpp \
    main.cpp \
    mainwindow.cpp \
    traffic.cpp \
    trafficview.cpp

HEADERS += \
    cleanerthread.h \
    mainwindow.h \
    traffic.h \
    trafficview.h

FORMS += \
    mainwindow.ui \
    trafficview.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

#begin connecting CANInterfaces
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../bin/ -lCANInterfaces
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../bin/ -lCANInterfacesd
#end connecting CANInterfaces

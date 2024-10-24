QT       += core gui axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    MyQExcel.cpp \
    MyQShowText.cpp \
    compiler.cpp \
    constants.cpp \
    logedclass.cpp \
    main.cpp \
    mainwindow.cpp \
    object.cpp \
    param.cpp \
    programm.cpp \
    value.cpp \
    widget.cpp

HEADERS += \
    MyQExcel.h \
    MyQShowText.h \
    colindexes.h \
    compiler.h \
    constants.h \
    logedclass.h \
    mainwindow.h \
    object.h \
    param.h \
    programm.h \
    value.h \
    widget.h

INCLUDEPATH += F:\C++\CAN\include
INCLUDEPATH += F:\C++\include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

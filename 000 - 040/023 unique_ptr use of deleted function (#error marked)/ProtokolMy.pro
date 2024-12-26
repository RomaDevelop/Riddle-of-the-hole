QT       += core gui axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += ..\include
INCLUDEPATH += ..\..\include

SOURCES += \
    MyQExcel.cpp \
    addproperties.cpp \
    category.cpp \
    code.cpp \
    constants.cpp \
    editor.cpp \
    frameworker.cpp \
    logs.cpp \
    main.cpp \
    mainwindow.cpp \
    object.cpp \
    param.cpp \
    programm.cpp \
    protocol.cpp \
    settings.cpp \
    tests/tests.cpp \
    value.cpp \
    widget.cpp

HEADERS += \
    MyQExcel.h \
    addproperties.h \
    category.h \
    code.h \
    configconstants.h \
    constants.h \
    editor.h \
    frameworker.h \
    ioperand.h \
    logs.h \
    mainwindow.h \
    object.h \
    param.h \
    programm.h \
    protocol.h \
    settings.h \
    tests/tests.h \
    value.h \
    widget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

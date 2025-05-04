QT       += core gui axcontainer xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    MyQExcel.cpp \
    addproperties.cpp \
    category.cpp \
    code.cpp \
    constants.cpp \
    editor/configitemfabric.cpp \
    editor/constantsconfig.cpp \
    editor/editor.cpp \
    editor/iconfigitem.cpp \
    editor/import.cpp \
    editor/objectcongif.cpp \
    editor/protocolconfig.cpp \
    editor/windowclass.cpp \
    frameworker.cpp \
    logs.cpp \
    main.cpp \
    mainwindow.cpp \
    object.cpp \
    param.cpp \
    programm.cpp \
    protocol.cpp \
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
    editor/configitemfabric.h \
    editor/constantsconfig.h \
    editor/editor.h \
    editor/iconfigitem.h \
    editor/import.h \
    editor/objectcongif.h \
    editor/protocolconfig.h \
    editor/windowclass.h \
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

INCLUDEPATH += ..\include
INCLUDEPATH += ..\..\include
DEPENDPATH += $$PWD/../include

#begin connecting CANInterfaces
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../bin/ -lCANInterfaces
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../bin/ -lCANInterfacesd
#end connecting CANInterfaces

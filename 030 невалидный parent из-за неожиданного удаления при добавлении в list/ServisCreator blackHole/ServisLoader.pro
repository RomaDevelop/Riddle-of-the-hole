QT += core gui widgets xml network

CONFIG += c++17

SOURCES += \
    addProperties.cpp \
    category.cpp \
    code.cpp \
    constants.cpp \
    editor/IConfigItem.cpp \
    editor/channelConfig.cpp \
    editor/config.cpp \
    editor/configItemFabric.cpp \
    editor/constantsConfig.cpp \
    editor/editor.cpp \
    editor/import.cpp \
    editor/objectConfig.cpp \
    editor/otherConfig.cpp \
    editor/protocolConfig.cpp \
    editor/sync.cpp \
    editor/windowConfig.cpp \
    editor/xmleditor.cpp \
    externalWindows/IExternalWindow.cpp \
    externalWindows/bkz27.cpp \
    externalWindows/bkz27psi.cpp \
    internalChannel.cpp \
    ioperand.cpp \
    logs.cpp \
    mainLoader.cpp \
    object.cpp \
    param.cpp \
    programm.cpp \
    protocol.cpp \
    tests/tests.cpp \
    value.cpp \
    widget.cpp \
    worker.cpp

HEADERS += \
    addProperties.h \
    allConstants.h \
    category.h \
    code.h \
    constants.h \
    editor/ConfigConversions.h \
    editor/IConfigItem.h \
    editor/channelConfig.h \
    editor/config.h \
    editor/configItemFabric.h \
    editor/constantsConfig.h \
    editor/editor.h \
    editor/import.h \
    editor/objectConfig.h \
    editor/otherConfig.h \
    editor/protocolConfig.h \
    editor/sync.h \
    editor/windowConfig.h \
    editor/xmleditor.h \
    externalWindows/IConnectExternalWidgets.h \
    externalWindows/IExternalWindow.h \
    externalWindows/bkz27.h \
    externalWindows/bkz27psi.h \
    internalChannel.h \
    ioperand.h \
    logs.h \
    object.h \
    param.h \
    programm.h \
    protocol.h \
    settings.h \
    tests/tests.h \
    value.h \
    widget.h \
    worker.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DEPENDPATH += ../include
INCLUDEPATH += \
    ../include \
    ../../include \
    editor \
    externalWindows \
    tests

#begin connecting CANInterfaces
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../bin/ -lCANInterfaces
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../bin/ -lCANInterfacesd
#end connecting CANInterfaces

#begin connecting CANSettingDialogDynamic
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../bin/ -lCANSettingDialogDynamic
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../bin/ -lCANSettingDialogDynamicd
#end connecting CANSettingDialogDynamic

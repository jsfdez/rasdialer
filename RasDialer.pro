TEMPLATE = app

QT += qml quick widgets xml

CONFIG += c++11

LIBS += Rasapi32.lib Rasdlg.lib Iphlpapi.lib Ws2_32.lib

DEFINES += WINVER=0x601


SOURCES += main.cpp \
    rasdialer.cpp \
    rasdialerconnectthread.cpp \
    servermodel.cpp \
    pingrunnable.cpp \
    ping.cpp

RESOURCES += qml.qrc
OTHER_FILES += phonebook.pbk servers.xml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    rasdialer.h \
    rasdialerconnectthread.h \
    servermodel.h \
    pingrunnable.h \
    ping.h

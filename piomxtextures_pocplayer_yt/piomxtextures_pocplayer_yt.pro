QT += core gui webkitwidgets quick

TARGET = piomxtextures_pocplayer_yt
TEMPLATE = app

INCLUDEPATH += ../3rdparty/LightLogger

SOURCES += main.cpp \
    poc_bridge.cpp

RESOURCES += \
    res.qrc

HEADERS += \
    poc_bridge.h

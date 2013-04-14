!include("../Common/retroshare_plugin.pri"): error("Could not include file ../Common/retroshare_plugin.pri")

CONFIG += qt uic qrc 

INCLUDEPATH += ../../retroshare-gui/src/temp/ui

SOURCES = \
    paintchatplugin.cpp \
    gui/paintchatwindow.cpp \
    gui/paintchatpopupchatdialog.cpp \
    services/paintchatitems.cpp \
    services/p3paintchatservice.cpp \
    gui/paintwidget.cpp \
    services/imageresource.cpp
HEADERS = \
    paintchatplugin.h \
    gui/paintchatwindow.h \
    gui/paintchatpopupchatdialog.h \
    services/paintchatitems.h \
    services/sync.h \
    services/p3paintchatservice.h \
    interface/paintchatservice.h \
    gui/paintwidget.h \
    services/imageresource.h
FORMS   = \
    gui/paintchatwindow.ui

TARGET = PaintChat

RESOURCES += \
    gui/images.qrc

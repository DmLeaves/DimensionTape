QT += core gui widgets multimedia

CONFIG += c++11

include($$PWD/../MessageSdk/MessageSdk.pri)
SOURCES += \
    applicationmanager.cpp \
    eventhandler.cpp \
    followlayouthelper.cpp \
    main.cpp \
    mainwindow.cpp \
    messagefollowcontroller.cpp \
    stickerdata.cpp \
    stickercontextmenucontroller.cpp \
    stickereventcontroller.cpp \
    stickereditcontroller.cpp \
    stickerfollowcontroller.cpp \
    stickerimage.cpp \
    stickerinteractioncontroller.cpp \
    stickerrepository.cpp \
    stickerrenderer.cpp \
    stickerruntime.cpp \
    stickermanager.cpp \
    stickertransformlayout.cpp \
    stickerwidget.cpp \
    trayicon.cpp \
    windowattachmentservice.cpp \
    windowrecognitionservice.cpp

HEADERS += \
    applicationmanager.h \
    eventhandler.h \
    followlayouthelper.h \
    mainwindow.h \
    messagefollowcontroller.h \
    stickerdata.h \
    stickercontextmenucontroller.h \
    stickereventcontroller.h \
    stickereditcontroller.h \
    stickerfollowcontroller.h \
    stickerimage.h \
    stickerinstance.h \
    stickerinteractioncontroller.h \
    stickerrepository.h \
    stickerrenderer.h \
    stickerruntime.h \
    stickermanager.h \
    stickertransformlayout.h \
    stickerwidget.h \
    trayicon.h \
    windowattachmentservice.h \
    windowrecognitionservice.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

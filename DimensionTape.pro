QT += core gui widgets multimedia

CONFIG += c++11

SOURCES += \
    applicationmanager.cpp \
    eventhandler.cpp \
    main.cpp \
    mainwindow.cpp \
    stickerdata.cpp \
    stickercontextmenucontroller.cpp \
    stickereventcontroller.cpp \
    stickereditcontroller.cpp \
    stickerimage.cpp \
    stickerinteractioncontroller.cpp \
    stickerrepository.cpp \
    stickerrenderer.cpp \
    stickerruntime.cpp \
    stickermanager.cpp \
    stickertransformlayout.cpp \
    stickerwidget.cpp \
    trayicon.cpp

HEADERS += \
    applicationmanager.h \
    eventhandler.h \
    mainwindow.h \
    stickerdata.h \
    stickercontextmenucontroller.h \
    stickereventcontroller.h \
    stickereditcontroller.h \
    stickerimage.h \
    stickerinteractioncontroller.h \
    stickerrepository.h \
    stickerrenderer.h \
    stickerruntime.h \
    stickermanager.h \
    stickertransformlayout.h \
    stickerwidget.h \
    trayicon.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

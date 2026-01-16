QT += core gui widgets multimedia opengl

CONFIG += c++11
QMAKE_CXXFLAGS += /utf-8
QMAKE_CFLAGS += /utf-8
win32: LIBS += user32.lib gdi32.lib shell32.lib ole32.lib

include($$PWD/../MessageSdk/MessageSdk.pri)
include($$PWD/../live2D/live2d_module.pri)
SOURCES += \
    applicationmanager.cpp \
    stickerassetstore.cpp \
    eventcombodelegate.cpp \
    eventdetailpanel.cpp \
    eventeditorpanel.cpp \
    eventhandler.cpp \
    eventparametereditor.cpp \
    followlayouthelper.cpp \
    eventlistmodel.cpp \
    eventtyperegistry.cpp \
    main.cpp \
    mainwindow.cpp \
    messagefollowcontroller.cpp \
    parametercodec.cpp \
    parametertablemodel.cpp \
    parametertypedelegate.cpp \
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
    stickerassetstore.h \
    eventcombodelegate.h \
    eventdetailpanel.h \
    eventeditorpanel.h \
    eventhandler.h \
    eventparametereditor.h \
    followlayouthelper.h \
    eventlistmodel.h \
    eventtyperegistry.h \
    mainwindow.h \
    messagefollowcontroller.h \
    parametercodec.h \
    parametertablemodel.h \
    parametertypedelegate.h \
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

QT       += core gui
QT       += sql
QT       += network

greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    common/base64.c \
    main.cpp \
    mainwindow.cpp \
    common/common.cpp \
    common/des.c \
    common/logininfoinstance.cpp \
    login.cpp \
    selfwidget/titlewidget.cpp \
    wares.cpp


HEADERS += \
    common/base64.h \
    common/common.h \
    common/des.h \
    common/logininfoinstance.h \
    login.h \
    mainwindow.h \
    selfwidget/titlewidget.h \
    wares.h

FORMS += \
    login.ui \
    mainwindow.ui \
    selfwidget/titlewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

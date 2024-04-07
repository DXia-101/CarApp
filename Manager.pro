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
    CustomerOrders.cpp \
    ProcureRecords.cpp \
    ProduceRecords.cpp \
    Purchase.cpp \
    PurchaseItem.cpp \
    ReportForms.cpp \
    ShopWidget.cpp \
    UserInterface.cpp \
    UserManager.cpp \
    UserOrderInterface.cpp \
    UserOrderTable.cpp \
    common/base64.c \
    homepage.cpp \
    main.cpp \
    mainwindow.cpp \
    common/common.cpp \
    common/des.c \
    common/logininfoinstance.cpp \
    login.cpp \
    product.cpp \
    productitem.cpp \
    selfwidget/mymenu.cpp \
    selfwidget/titlewidget.cpp \
    userwindow.cpp \
    wares.cpp


HEADERS += \
    CustomerOrders.h \
    ProcureRecords.h \
    ProduceRecords.h \
    Purchase.h \
    PurchaseItem.h \
    ReportForms.h \
    ShopWidget.h \
    UserInterface.h \
    UserManager.h \
    UserOrderInterface.h \
    UserOrderTable.h \
    common/base64.h \
    common/common.h \
    common/des.h \
    common/logininfoinstance.h \
    homepage.h \
    login.h \
    mainwindow.h \
    product.h \
    productitem.h \
    selfwidget/mymenu.h \
    selfwidget/titlewidget.h \
    userwindow.h \
    wares.h

FORMS += \
    CustomerOrders.ui \
    PurchaseItem.ui \
    ShopWidget.ui \
    UserInterface.ui \
    UserOrderInterface.ui \
    homepage.ui \
    login.ui \
    mainwindow.ui \
    productitem.ui \
    selfwidget/titlewidget.ui \
    userwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

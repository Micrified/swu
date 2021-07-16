QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    attributes.cpp \
    cfgelement.cpp \
    cfgparser.cpp \
    cfgstatemachine.cpp \
    cfgupdater.cpp \
    cfgxmlhandler.cpp \
    element.cpp \
    fsoperation.cpp \
    main.cpp \
    mainwindow.cpp \
 \    #update.cpp
    resource.cpp \
    resource_manager.cpp

HEADERS += \
    attributes.h \
    cfgelement.h \
    cfgparser.h \
    cfgstatemachine.h \
    cfgupdater.h \
    cfgxmlhandler.h \
    element.h \
    fsoperation.h \
    mainwindow.h \
 \    #update.h
    resource.h \
    resource_manager.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    software_updater_en_US.ts

QMAKE_CXXFLAGS += -v

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    stylesheet.css \
    update_config.xml

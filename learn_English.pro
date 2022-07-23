QT       += core gui sql texttospeech

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    table1config.cpp \
    table2config.cpp

HEADERS += \
    mainwindow.h \
    table1config.h \
    table2config.h

FORMS += \
    mainwindow.ui \
    table1config.ui \
    table2config.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# QXlsx code for Application Qt project
QXLSX_PARENTPATH=./QXlsx    # current QXlsx path is ./QXlsx (. means curret directory)
QXLSX_HEADERPATH=./QXlsx/header/  # current QXlsx header path is ./QXlsx/header/
QXLSX_SOURCEPATH=./QXlsx/source/  # current QXlsx source path is ./QXlsx/source/
include(./QXlsx/QXlsx.pri)

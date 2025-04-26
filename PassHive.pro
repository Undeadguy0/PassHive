QT += widgets sql

LIBS += -latomic

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    passokno.cpp \
    logilireg.cpp \
    qaesencryption.cpp \
    regokno.cpp \
    passmanager.cpp \
    dbpoisk.cpp \
    oknoparoley.cpp

HEADERS += \
    mainwindow.h \
    passokno.h \
    logilireg.h \
    qaesencryption.h \
    regokno.h \
    PassManager.h \
    dbpoisk.h \
    oknoparoley.h

FORMS += \
    mainwindow.ui \
    passokno.ui \
    logilireg.ui \
    regokno.ui \
    oknoparoley.ui

RESOURCES += \
    resurs.qrc


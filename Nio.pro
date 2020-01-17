QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    main.cpp \
    ceshi_file.cpp \
    logperf.cpp \
    sem_pv.cpp \
    mem_poll.cpp \
    sock_buff.cpp \
    tcp_buf.cpp \
    tcp_client.cpp \
    tcp_ser.cpp \
    epollnode.cpp \
    epollloop.cpp \
    heartalarm.cpp \
    epollloopi.cpp \
    threadpool.cpp

    tcp_serii.cpp \

SUBDIRS += \
    Nio.pro

DISTFILES += \
    Nio.pro.user

HEADERS += \
    logperf.h \
    sem_pv.h \
    head.h \
    mem_poll.h \
    sock_buff.h \
    tcp_buf.h \
    tcp_client.h \
    tcp_ser.h \
    epollnode.h \
    epollloop.h \
    heartalarm.h \
    epollloopi.h \
    threadpool.h

    tcp_serii.h \

CONFIG+=++11

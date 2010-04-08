# QMake include file for building libhid as part of another project
#
# This does not build a library or an app, it merely links the libhid source
# directly into a parent project

INCLUDEPATH += $$PWD/include

SOURCES += device.cc hid.cc
HEADERS += $$PWD/include/hid.h

macx {
    DEPENDPATH += $$PWD/macos
    LIBS += -framework IOKit -framework CoreServices
    SOURCES += util.cc
}

win32 {
    DDK_TARGET = w2k
    DEPENDPATH += $$PWD/win32
    HEADERS += $$PWD/win32/auto_free.h
    LIBS += -lhid -lsetupapi
    # Want Win2K or later
    DEFINES += WINVER=0x0500
}

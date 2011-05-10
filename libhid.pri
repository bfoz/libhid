# QMake include file for building libhid as part of another project
#
# This does not build a library or an app, it merely links the libhid source
# directly into a parent project

INCLUDEPATH += $$PWD/include

SOURCES += device.cc enumerator.cc hid.cc
HEADERS += $$PWD/include/hid.h $$PWD/include/device.h $$PWD/include/element.h
HEADERS += $$PWD/include/enumerator.h

macx {
    DEPENDPATH += $$PWD/macos
    LIBS += -framework IOKit -framework CoreServices
    SOURCES += element.cc util.cc
}

win32 {
    DDK_TARGET = w2k
    DEPENDPATH += $$PWD/win32
    HEADERS += $$PWD/win32/auto_free.h
    HEADERS += button.h collection.h element.h value.h
    LIBS += -lhid -lsetupapi
    # Want Win2K or later
    DEFINES += WINVER=0x0500
}

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/qthdf5.cpp

HEADERS +=  \
    $$PWD/qthdf5.h

# Link with the hdf5 lib
LIBS += -lhdf5

# this works in Ubuntu 20.04
#linux {
#    CONFIG += link_pkgconfig
#    PKGCONFIG += hdf5-serial
#}

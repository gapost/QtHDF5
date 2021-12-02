INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/qthdf5.cpp

HEADERS +=  \
    $$PWD/qthdf5.h


unix {

    UBUNTU = $$system(cat /proc/version | grep -o Ubuntu)

    contains( UBUNTU, Ubuntu ) {
        CONFIG += link_pkgconfig
        PKGCONFIG += hdf5-serial
    } else {
        LIBS += -lhdf5
    }
}

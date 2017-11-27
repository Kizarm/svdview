QT      += core gui xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_CXXFLAGS += -Wno-unused-variable # Fix to build with GCC 4.8

TARGET   = svdview
TEMPLATE = app


SOURCES  = main.cpp
SOURCES += mainwindow.cpp
SOURCES += svdtree.cpp
SOURCES += utils.cpp
SOURCES += devicetree.cpp
SOURCES += devicedump.cpp
SOURCES += devicevalid.cpp
SOURCES += cmdline.cpp

HEADERS  = mainwindow.h
HEADERS += svdtree.h
HEADERS += device.h
HEADERS += devicetree.h
HEADERS += utils.h
HEADERS += cmdline.h

#FORMS    =
#INCLUDEPATH +=
#LIBS        +=
 RESOURCES   += view.qrc

unix {
    MOC_DIR     = moc
    OBJECTS_DIR = obj
    DEFINES    += UNIX
}

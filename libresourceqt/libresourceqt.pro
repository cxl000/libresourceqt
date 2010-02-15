include(../common.pri)
TEMPLATE = lib
TARGET = resourceqt
DESTDIR = build
DEPENDPATH += $${POLICY} src
INCLUDEPATH += $${LIBRESOURCEINC} $${LIBDBUSQEVENTLOOP} src

# Input
PUBLIC_HEADERS = $${POLICY}/resource.h $${POLICY}/resource-set.h $${POLICY}/resources.h

HEADERS += $${PUBLIC_HEADERS} src/resource-engine.h

SOURCES += src/resource.cpp \
           src/resource-set.cpp \
           src/resource-engine.cpp \
           src/resources.cpp

QMAKE_CXXFLAGS += -Wall
LIBS += -L$${LIBDBUSQEVENTLOOP}/build -ldbus-qeventloop

OBJECTS_DIR = build
MOC_DIR = build

CONFIG  += qt link_pkgconfig dll
QT = core
PKGCONFIG += dbus-1 libresource0

# Install directives
headers.files  = $${PUBLIC_HEADERS}
INSTALLBASE    = /usr
target.path    = $${INSTALLBASE}/lib
headers.path   = $${INSTALLBASE}/include/resource/qt4/policy

INSTALLS       = target headers

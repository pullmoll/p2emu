#-------------------------------------------------
#
# Project created by QtCreator 2019-08-08T01:52:49
#
#-------------------------------------------------

QT += core gui widgets
TARGET = p2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -Wno-signed-enum-bitfield

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
		about.cpp \
		gotoaddress.cpp \
		main.cpp \
		mainwindow.cpp \
		p2asm.cpp \
		p2asmsym.cpp \
		p2asmsymtbl.cpp \
		p2cog.cpp \
		p2cogview.cpp \
		p2dasm.cpp \
		p2dasmmodel.cpp \
		p2hub.cpp \
		p2token.cpp

HEADERS += \
		about.h \
		gotoaddress.h \
		mainwindow.h \
		p2asm.h \
		p2asmsym.h \
		p2asmsymtbl.h \
		p2cog.h \
		p2cogview.h \
		p2dasm.h \
		p2dasmmodel.h \
		p2defs.h \
		p2hub.h \
		p2token.h

FORMS += \
		about.ui \
		gotoaddress.ui \
		mainwindow.ui \
		p2cogview.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
	p2emu.qrc

DISTFILES += \
		README.md \
		ROM_Booter_v33_01j.spin2 \
		csv.cpp \
		csv.h

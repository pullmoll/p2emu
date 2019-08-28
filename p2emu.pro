#-------------------------------------------------
#
# Project created by QtCreator 2019-08-08T01:52:49
#
#-------------------------------------------------

QT += core gui widgets svg
TARGET = p2
TEMPLATE = app
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -Wno-signed-enum-bitfield -DVER_MAJ=$$VER_MAJ -DVER_MIN=$$VER_MIN -DVER_PAT=$$VER_PAT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
		main.cpp \
		mainwindow.cpp \
		p2asm.cpp \
		p2atom.cpp \
		p2cog.cpp \
		p2cogview.cpp \
		p2dasm.cpp \
		p2defs.cpp \
		p2hub.cpp \
		p2hubview.cpp \
		p2opcode.cpp \
		p2symbol.cpp \
		p2symboltable.cpp \
		p2token.cpp \
		delegates/p2opcodedelegate.cpp \
		delegates/p2sourcedelegate.cpp \
		delegates/p2referencesdelegate.cpp \
		dialogs/about.cpp \
		dialogs/gotoaddress.cpp \
		dialogs/gotoline.cpp \
		dialogs/p2asmlisting.cpp \
		models/p2asmmodel.cpp \
		models/p2dasmmodel.cpp \
		models/p2symbolsmodel.cpp \
		p2word.cpp \
		util/p2util.cpp

HEADERS += \
		mainwindow.h \
		models/p2symbolsmodel.h \
		p2asm.h \
		p2atom.h \
		p2cog.h \
		p2cogview.h \
		p2dasm.h \
		p2defs.h \
		p2hub.h \
		p2hubview.h \
		p2opcode.h \
		p2symbol.h \
		p2symboltable.h \
		p2token.h \
		delegates/p2opcodedelegate.h \
		delegates/p2sourcedelegate.h \
		delegates/p2referencesdelegate.h \
		dialogs/about.h \
		dialogs/gotoaddress.h \
		dialogs/gotoline.h \
		dialogs/p2asmlisting.h \
		models/p2asmmodel.h \
		models/p2dasmmodel.h \
		p2tokens.h \
		p2word.h \
		util/p2util.h

FORMS += \
		mainwindow.ui \
		p2cogview.ui \
		p2hubview.ui \
		dialogs/about.ui \
		dialogs/gotoaddress.ui \
		dialogs/gotoline.ui \
		dialogs/p2asmlisting.ui

INCLUDEPATH += $$PWD/delegates
INCLUDEPATH += $$PWD/dialogs
INCLUDEPATH += $$PWD/models
INCLUDEPATH += $$PWD/util

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
	p2emu.qrc

TRANSLATIONS += \
	p2emu.de.ts

DISTFILES += \
		P2-qz80-rr032.spin2 \
		README.md \
		ROM_Booter_v33_01j.spin2 \
		csv.cpp \
		csv.h \
		icons/decrease.png \
		icons/error.png \
		icons/increase.png

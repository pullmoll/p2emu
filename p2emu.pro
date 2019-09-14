#-------------------------------------------------
#
# Project created by QtCreator 2019-08-08T01:52:49
#
#-------------------------------------------------

QT += core gui widgets svg xml
TARGET = p2
TEMPLATE = app
VER_MAJ = 0
VER_MIN = 4
VER_PAT = 0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -DVER_MAJ=$$VER_MAJ -DVER_MIN=$$VER_MIN -DVER_PAT=$$VER_PAT

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
	p2dasm.cpp \
	p2defs.cpp \
	p2doc.cpp \
	p2docopcode.cpp \
	p2hub.cpp \
	p2opcode.cpp \
	p2symbol.cpp \
	p2symboltable.cpp \
	p2token.cpp \
	p2union.cpp \
	p2word.cpp \
	delegates/p2opcodedelegate.cpp \
	delegates/p2sourcedelegate.cpp \
	delegates/p2referencesdelegate.cpp \
	dialogs/about.cpp \
	dialogs/gotoaddress.cpp \
	dialogs/gotoline.cpp \
	dialogs/palettesetup.cpp \
	dialogs/textbrowser.cpp \
	filters/p2symbolsorter.cpp \
	models/p2asmmodel.cpp \
	models/p2dasmmodel.cpp \
	models/p2symbolsmodel.cpp \
	util/p2colors.cpp \
	util/p2util.cpp \
	views/p2cogview.cpp \
	views/p2hubview.cpp

HEADERS += \
	mainwindow.h \
	p2asm.h \
	p2atom.h \
	p2cog.h \
	p2dasm.h \
	p2defs.h \
	p2doc.h \
	p2docopcode.h \
	p2hub.h \
	p2opcode.h \
	p2symbol.h \
	p2symboltable.h \
	p2token.h \
	p2tokens.h \
	p2union.h \
	p2word.h \
	delegates/p2opcodedelegate.h \
	delegates/p2sourcedelegate.h \
	delegates/p2referencesdelegate.h \
	dialogs/about.h \
	dialogs/gotoaddress.h \
	dialogs/gotoline.h \
	dialogs/palettesetup.h \
	dialogs/textbrowser.h \
	filters/p2symbolsorter.h \
	models/p2asmmodel.h \
	models/p2dasmmodel.h \
	models/p2symbolsmodel.h \
	util/p2colors.h \
	util/p2html.h \
	util/p2util.h \
	views/p2cogview.h \
	views/p2hubview.h

FORMS += \
	mainwindow.ui \
	dialogs/about.ui \
	dialogs/gotoaddress.ui \
	dialogs/gotoline.ui \
	dialogs/palettesetup.ui \
	dialogs/textbrowser.ui \
	views/p2cogview.ui \
	views/p2hubview.ui

INCLUDEPATH += $$PWD/delegates
INCLUDEPATH += $$PWD/dialogs
INCLUDEPATH += $$PWD/filters
INCLUDEPATH += $$PWD/models
INCLUDEPATH += $$PWD/views
INCLUDEPATH += $$PWD/util

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = icons/propeller2.ico
RC_FILE = p2emu.rc

RESOURCES += \
	p2emu.qrc

TRANSLATIONS += \
	translations/p2emu.de.ts

DISTFILES += \
	README.md \
	csv.cpp \
	csv.h \
	translations/p2emu.de.qm

FLEXSOURCES += \
	p2flex.l

flexsource.input = FLEXSOURCES
flexsource.output = ${QMAKE_FILE_BASE}.cpp
flexsource.commands = flex --header-file=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flexsource

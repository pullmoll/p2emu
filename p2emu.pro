#-------------------------------------------------
#
# Project created by QtCreator 2019-08-08T01:52:49
#
#-------------------------------------------------

QT += core gui widgets svg xml
TARGET = p2
TEMPLATE = app
VER_MAJ = 0
VER_MIN = 3
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
	icons/down_arrow.png \
	icons/error.png \
	icons/increase.png \
	spin2/MainLoader.spin2 \
	spin2/NTSC_256_x_192.spin2 \
	spin2/NTSC_256_x_192_interrupt.spin2 \
	spin2/P2-qz80-rr032.spin2 \
	spin2/ROM_137PBJ.spin2 \
	spin2/ROM_Booter_v32.spin2 \
	spin2/ROM_Booter_v33_01j.spin2 \
	spin2/SD2_test_121a.spin2 \
	spin2/USBHost.spin2 \
	spin2/VGA_640_x_480_16bpp.spin2 \
	spin2/VGA_640_x_480_4bpp1.spin2 \
	spin2/VGA_640_x_480_8bpp.spin2 \
	spin2/all_cogs_blink.spin2 \
	spin2/bas.spin2 \
	spin2/birds_16bpp.bmp \
	spin2/bitmap.bmp \
	spin2/bitmap2.bmp \
	spin2/blink.spin2 \
	spin2/blinkit.spin2 \
	spin2/chess.spin2 \
	spin2/crcbit_crcnib.spin2 \
	spin2/dry.spin2 \
	spin2/echo1.spin2 \
	spin2/echo2.spin2 \
	spin2/fibo.spin2 \
	spin2/fibo1.spin2 \
	spin2/fibo2.spin2 \
	spin2/fibo3.spin2 \
	spin2/lock_test.spin2 \
	spin2/ml.spin2 \
	spin2/p2pfth.spin2 \
	spin2/pointers.spin2 \
	spin2/prefix.spin2 \
	spin2/printf.spin2 \
	spin2/programmer.spin2 \
	spin2/sin_cos_dacs.spin2 \
	spin2/single_step.spin2 \
	spin2/smartpin_pwm.spin2 \
	spin2/smartpin_serial_turnaround.spin2 \
	spin2/smartpin_usb_turnaround.spin2 \
	spin2/spin2_interpreter.spin2 \
	spin2/test1.spin2 \
	spin2/testjmploc.spin2 \
	spin2/testops.spin2 \
	spin2/tt.spin2 \
	spin2/xbyte.spin2 \
	spin2/xoro32.spin2 \
	spin2/xoroshiro128plus.spin2

/****************************************************************************
 *
 * P2 emulator main window
 *
 * Copyright (C) 2019 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include "p2token.h"

namespace Ui {
class MainWindow;
}
class P2Hub;
class P2CogView;
class P2Asm;
class P2Params;
class P2AsmModel;
class P2Dasm;
class P2DasmModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ui_tab_e {
        id_hub,
        id_asm,
        id_dasm
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void saveSettings();
    void restoreSettings();
    void about();
    void aboutQt5();

    void gotoHex(const QString& address = QString());
    void gotoCog();
    void gotoLut();
    void gotoRom();
    void gotoAddress();

    void setAsmOpcodes(int mode);
    void setAsmOpcodesBin();
    void setAsmOpcodesByt();
    void setAsmOpcodesOct();
    void setAsmOpcodesHex();
    void incAsmFontSize();
    void decAsmFontSize();
    void setAsmFontSize(qreal size);

    void setDasmOpcodes(int mode);
    void setDasmOpcodesBin();
    void setDasmOpcodesByt();
    void setDasmOpcodesOct();
    void setDasmOpcodesHex();
    void incDasmFontSize();
    void decDasmFontSize();
    void setDasmFontSize(qreal size);

    void setDasmLowercase(bool check);
    void asmHeaderColums(const QPoint& pos);
    void dasmHeaderColums(const QPoint& pos);

    void hubSingleStep();

    void openSource(const QString& filename = QString());
    void assemble();
private:
    Ui::MainWindow *ui;
    QVector<P2CogView*> m_vcog;
    P2Hub* m_hub;
    P2Asm* m_asm;
    P2AsmModel* m_amodel;
    P2Dasm* m_dasm;
    P2DasmModel* m_dmodel;

    void setupAssembler();
    void setupDisassembler();
    void setupMenu();
    void setupTabWidget();
    void setupToolbars();
    void setupStatusbar();

    void updateAsmColumnSizes();
    void updateDasmColumnSizes();
    void setupCogView();
};

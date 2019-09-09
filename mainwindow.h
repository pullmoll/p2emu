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
#include <QSettings>
#include <QUrl>
#include "p2token.h"

namespace Ui {
class MainWindow;
}
class P2Hub;
class P2CogView;

class P2Asm;
class P2AsmModel;

class P2Dasm;
class P2DasmModel;

class P2SymbolsModel;

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

    void saveSettingsPalette();
    void saveSettingsAsm();
    void saveSettingsDasm();

    void restoreSettingsPalette();
    void restoreSettingsAsm();
    void restoreSettingsDasm();

    void about();
    void aboutQt5();
    void help_opcodes();

    void goto_hex(const QString& address = QString());
    void goto_cog();
    void goto_lut();
    void goto_rom();
    void goto_address();

    void setAsmOpcodes(int mode);
    void setAsmOpcodes();
    void incAsmFontSize();
    void decAsmFontSize();
    void setAsmFontSize(int size);

    void setDasmOpcodes(int mode);
    void setDasmOpcodes();
    void incDasmFontSize();
    void decDasmFontSize();
    void setDasmFontSize(int size);

    void setDasmLowercase(bool check);
    void setAsmHeaderColums(const QPoint& pos);
    void setDasmHeaderColums(const QPoint& pos);
    void setSymbolsHeaderColums(const QPoint& pos);

    void hubSingleStep();
    void loadObject(const QString& filename = QString());
    void loadObjectRandom();

    void openSource(const QString& filename = QString());
    void loadSource(const QString& filename = QString());
    void loadSourceRandom();
    void assemble();

    void palette_setup();

    void print_error(int pass, int lineno, const QString& message);
    void goto_line(const QUrl& url);
    void goto_line_number();
    void resize_source_results(const int results_min = 10, const int errors_min = 5);
private:
    Ui::MainWindow *ui;
    QVector<P2CogView*> m_vcog;
    P2Hub* m_hub;
    P2Asm* m_asm;
    P2AsmModel* m_amodel;
    P2Dasm* m_dasm;
    P2DasmModel* m_dmodel;
    P2SymbolsModel* m_smodel;

    QFont m_font;
    int m_asm_font_size;
    int m_dasm_font_size;
    int m_source_percent;
    int m_results_percent;

    void setupAssembler();
    void setupDisassembler();
    void setupFonts();
    void setupMenu();
    void setupTabWidget();
    void setupToolbars();
    void setupStatusbar();

    void updateAsmColumnSizes();
    void updateDasmColumnSizes();
    void updateSymbolsColumnSizes();
    void setupCogView();
};

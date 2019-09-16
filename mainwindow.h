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
    void save_settings();
    void restore_settings();

    void save_settings_palette();
    void save_settings_asm();
    void save_settings_dasm();

    void restoreSettingsPalette();
    void restore_settings_asm();
    void restore_settings_dasm();

    void about();
    void aboutQt5();
    void help_opcodes();

    void goto_hex(const QString& address = QString());
    void goto_cog();
    void goto_lut();
    void goto_rom();
    void goto_address();

    void set_opcodes_asm(int mode);
    void set_opcodes_asm();
    void font_size_inc_asm();
    void font_size_dec_asm();
    void set_font_asm(const QFont& font);

    void set_opcodes_dasm(int mode);
    void set_opcodes_dasm();
    void font_size_inc_dasm();
    void font_size_dec_dasm();
    void set_font_dasm(const QFont& font);

    void set_lowercase_dasm(bool check);
    void header_columns_asm(QPoint pos);
    void header_columns_dasm(QPoint pos);
    void header_columns_sym(QPoint pos);

    void hub_single_step();
    void load_object(const QString& filename = QString());
    void load_object_random();

    void open_source(const QString& filename = QString());
    void load_source(const QString& filename = QString());
    void load_source_random();
    void assemble();

    void palette_setup();
    void preferences();

    void print_error(int pass, int lineno, const QString& message);
    void goto_line(const QUrl& url);
    void goto_line_number();
    void goto_definition(const QModelIndex& index);
    void resize_splitter_source();
private:
    Ui::MainWindow *ui;
    QVector<P2CogView*> m_vcog;
    P2Hub* m_hub;
    P2Asm* m_asm;
    P2Dasm* m_dasm;

    QFont m_font_asm;
    QFont m_font_dasm;
    int m_num_errors;
    int m_source_percent;
    int m_symbols_percent;
    int m_errors_percent;

    P2AsmModel* asm_model();
    P2DasmModel* dasm_model();
    P2SymbolsModel* sym_model();

    void setup_asm();
    void setup_sym();
    void setup_err();
    void setup_dasm();
    void setup_fonts();
    void setup_menu();
    void setup_tabwidget();
    void setup_toolbars();
    void setup_statusbar();

    void update_sizes_asm();
    void update_sizes_dasm();
    void update_sizes_symbols();
    void setup_cog_views();
};

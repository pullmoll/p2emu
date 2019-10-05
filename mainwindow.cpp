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
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimer>
#include <QSettings>
#include <QLabel>
#include <QLayout>
#include <QFontDatabase>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "p2doc.h"
#include "gotoaddress.h"
#include "gotoline.h"
#include "palettesetup.h"
#include "preferences.h"
#include "textbrowser.h"
#include "p2hub.h"
#include "p2cog.h"
#include "p2asm.h"
#include "p2asmmodel.h"
#include "p2dasm.h"
#include "p2dasmmodel.h"
#include "p2symboltable.h"
#include "p2symbolsmodel.h"
#include "p2opcodedelegate.h"
#include "p2sourcedelegate.h"
#include "p2referencesdelegate.h"
#include "p2symbolsorter.h"

static const int ncogs = 8;
static const QLatin1String key_windowGeometry("windowGeometry");
static const QLatin1String key_windowState("windowState");
static const QLatin1String grp_assembler("assembler");
static const QLatin1String grp_disassembler("disassembler");
static const QLatin1String grp_palette("palette");
static const QLatin1String key_palette("p2_palette");
static const QLatin1String key_opcodes("opcodes");
static const QLatin1String key_lowercase("lowercase");
static const QLatin1String key_current_row("current_row");
static const QLatin1String key_current_column("current_column");
static const QLatin1String key_column_address("hide_address");
static const QLatin1String key_column_origin("hide_origin");
static const QLatin1String key_column_tokens("hide_tokens");
static const QLatin1String key_column_opcode("hide_opcode");
static const QLatin1String key_column_errors("hide_errors");
static const QLatin1String key_column_symbols("hide_symbols");
static const QLatin1String key_column_instruction("hide_instruction");
static const QLatin1String key_column_source("hide_source");
static const QLatin1String key_column_description("hide_description");
static const QLatin1String key_pnut_compatible("pnut_compatible");
static const QLatin1String key_v33mode("v33mode");
static const QLatin1String key_file_errors("file_errors");
static const QLatin1String key_font("font");
static const QLatin1String key_splitter_source_percent("source_percent");
static const QLatin1String key_splitter_symbols_percent("symbols_percent");
static const QLatin1String key_splitter_errors_percent("errors_percent");

static const QLatin1String key_source("source");
static const QLatin1String key_directory("directory");
static const QLatin1String key_filename("filename");
static const QLatin1String key_history("directory");

static const QLatin1String key_lines("lines");
static const QLatin1String key_status("status");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_vcog()
    , m_hub(new P2Hub(ncogs, this))
    , m_asm(new P2Asm(this))
    , m_dasm(new P2Dasm(m_hub->cog(0)))
    , m_font_asm(QLatin1String("Source Code Pro"), 9)
    , m_font_dasm(QLatin1String("Source Code Pro"), 9)
    , m_source_percent(80)
    , m_symbols_percent(15)
    , m_errors_percent(5)
{
    qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch()));
    ui->setupUi(this);

    setWindowTitle(QString("%1 v%2").arg(qApp->applicationName()).arg(qApp->applicationVersion()));
    setup_menu();
    setup_tabwidget();
    setup_toolbars();
    setup_statusbar();

    setup_asm();
    setup_err();
    setup_sym();
    setup_dasm();
    setup_cog_views();

    setup_fonts();
    restore_settings();

    // load_source(QStringLiteral(":/spin2/spin2_interpreter.spin2"));
    // load_source(QStringLiteral(":/spin2/USBHost.spin2"));
    load_source(QStringLiteral(":/spin2/VGA_640_x_480_8bpp.spin2"));
    // load_source(QStringLiteral(":/spin2/P2-qz80-rr032.spin2"));
    // load_source(QStringLiteral(":/spin2/ROM_Booter_v33_01j.spin2"));
    // load_source(QStringLiteral(":/spin2/pointers.spin2"));
    // load_source(QStringLiteral(":/spin2/all_cogs_blink.spin2"));
    // load_source(QStringLiteral(":/spin2/lock_test.spin2"));
    // load_source(QStringLiteral(":/spin2/testjmploc.spin2"));
    load_object_random();
}

MainWindow::~MainWindow()
{
    save_settings();
    delete ui;
}

void MainWindow::save_settings()

{
    QSettings s;
    s.setValue(key_windowGeometry, saveGeometry());
    s.setValue(key_windowState, saveState());
    save_settings_asm();
    save_settings_dasm();
    save_settings_palette();

}

void MainWindow::restore_settings()
{
    QSettings s;
    restoreGeometry(s.value(key_windowGeometry).toByteArray());
    restoreState(s.value(key_windowState).toByteArray());
    restore_settings_asm();
    restore_settings_dasm();
    restore_settings_palette();
}

void MainWindow::save_settings_asm()
{
    QSettings s;
    QList<int> sizes = ui->splSource->sizes();
    QSize size = ui->tabAsm->size();
    if (size.height() > 0) {
        m_source_percent = sizes.value(0) * 100 / size.height();
        m_symbols_percent = sizes.value(1) * 100 / size.height();
        m_errors_percent = sizes.value(2) * 100 / size.height();
    }

    s.beginGroup(grp_assembler);
    const P2AsmModel* amodel = asm_model();
    if (amodel)
        s.setValue(key_opcodes, amodel->opcode_format());
    s.setValue(key_column_origin, ui->tvAsm->isColumnHidden(P2AsmModel::c_HubAddr));
    s.setValue(key_column_address, ui->tvAsm->isColumnHidden(P2AsmModel::c_CogAddr));
    s.setValue(key_column_opcode, ui->tvAsm->isColumnHidden(P2AsmModel::c_Opcode));
    s.setValue(key_column_tokens, ui->tvAsm->isColumnHidden(P2AsmModel::c_Tokens));
    s.setValue(key_column_symbols, ui->tvAsm->isColumnHidden(P2AsmModel::c_Symbols));
    s.setValue(key_column_errors, ui->tvAsm->isColumnHidden(P2AsmModel::c_Errors));
    s.setValue(key_column_source, ui->tvAsm->isColumnHidden(P2AsmModel::c_Source));
    QModelIndex index = ui->tvAsm->currentIndex();
    s.setValue(key_current_row, index.row());
    s.setValue(key_current_column, index.column());
    s.setValue(key_font, ui->tvAsm->font());
    s.setValue(key_splitter_source_percent, m_source_percent);
    s.setValue(key_splitter_symbols_percent, m_symbols_percent);
    s.setValue(key_splitter_errors_percent, m_errors_percent);

    s.setValue(key_pnut_compatible, m_asm->pnut());
    s.setValue(key_v33mode, m_asm->v33mode());
    s.setValue(key_file_errors, m_asm->file_errors());

    s.endGroup();
}

void MainWindow::restore_settings_asm()
{
    QSettings s;
    s.beginGroup(grp_assembler);
    set_opcodes_asm(s.value(key_opcodes, fmt_bin).toInt());
    int row = s.value(key_current_row).toInt();
    int column = s.value(key_current_row).toInt();
    P2AsmModel* amodel = asm_model();
    if (amodel)
        ui->tvAsm->setCurrentIndex(amodel->index(row, column));
    ui->tvAsm->setColumnHidden(P2AsmModel::c_HubAddr, s.value(key_column_origin, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_CogAddr, s.value(key_column_address, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Opcode, s.value(key_column_opcode, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Tokens, s.value(key_column_tokens, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Symbols, s.value(key_column_symbols, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Errors, s.value(key_column_errors, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Source, s.value(key_column_source, false).toBool());
    m_font_asm = qvariant_cast<QFont>(s.value(key_font, m_font_asm));
    m_source_percent = qBound(1,s.value(key_splitter_source_percent, m_source_percent).toInt(),100);
    m_symbols_percent = qBound(1,s.value(key_splitter_symbols_percent, m_symbols_percent).toInt(),100);
    m_errors_percent = qBound(1,s.value(key_splitter_errors_percent, m_errors_percent).toInt(),100);

    m_asm->set_pnut(s.value(key_pnut_compatible, true).toBool());
    m_asm->set_v33mode(s.value(key_v33mode, false).toBool());
    m_asm->set_file_errors(s.value(key_file_errors, false).toBool());

    s.endGroup();
    set_font_asm(m_font_asm);
    resize_splitter_source();
}

void MainWindow::save_settings_dasm()
{
    const P2DasmModel* dmodel = dasm_model();
    QSettings s;
    s.beginGroup(grp_disassembler);
    s.setValue(key_opcodes, dmodel->opcode_format());
    s.setValue(key_lowercase, ui->action_Dasm_Lowercase->isChecked());
    s.setValue(key_column_address, ui->tvDasm->isColumnHidden(P2DasmModel::c_Address));
    s.setValue(key_column_opcode, ui->tvDasm->isColumnHidden(P2DasmModel::c_Opcode));
    s.setValue(key_column_instruction, ui->tvDasm->isColumnHidden(P2DasmModel::c_Instruction));
    s.setValue(key_column_description, ui->tvDasm->isColumnHidden(P2DasmModel::c_Description));
    QModelIndex index = ui->tvDasm->currentIndex();
    s.setValue(key_current_row, index.row());
    s.setValue(key_current_column, index.column());
    m_font_dasm = qvariant_cast<QFont>(s.value(key_font, m_font_dasm));
    s.endGroup();
}

void MainWindow::restore_settings_dasm()
{
    QSettings s;
    s.beginGroup(grp_disassembler);
    set_opcodes_dasm(s.value(key_opcodes, fmt_bin).toInt());
    set_lowercase_dasm(s.value(key_lowercase).toBool());
    ui->tvDasm->selectRow(s.value(key_current_row).toInt());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Address, s.value(key_column_address, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Opcode, s.value(key_column_opcode, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Instruction, s.value(key_column_instruction, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Description, s.value(key_column_description, false).toBool());
    m_font_dasm = qvariant_cast<QFont>(s.value(key_font, m_font_dasm));
    s.endGroup();
    set_font_dasm(m_font_dasm);
}

void MainWindow::save_settings_palette()
{
    QSettings s;
    s.beginGroup(grp_palette);
    Colors.save_palette(s);
    s.endGroup();
}

void MainWindow::restore_settings_palette()
{
    QSettings s;
    s.beginGroup(grp_palette);
    Colors.restore_palette(s);
    s.endGroup();
}

void MainWindow::about()
{
    About dlg;
    dlg.setApplicationNameVersion(qApp->applicationName(), qApp->applicationVersion());
    dlg.adjustSize();
    dlg.exec();
}

void MainWindow::aboutQt5()
{
    qApp->aboutQt();
}

void MainWindow::help_opcodes()
{
    static int tab = -1;
    if (tab >= 0) {
        ui->tabWidget->removeTab(tab);
        tab = -1;
    } else {
        QStringList html = Doc.html_opcodes();
        TextBrowser* dlg = new TextBrowser;
        dlg->set_html(html);
        dlg->setWindowTitle(tr("Propeller2 Opcode Table"));
        connect(dlg, SIGNAL(finished(int)), SLOT(help_opcodes()));
        tab = ui->tabWidget->addTab(dlg, dlg->windowTitle());
        ui->tabWidget->setCurrentIndex(tab);
    }
}

void MainWindow::goto_hex(const QString& address)
{
    bool ok;
    p2_LONG addr = address.toUInt(&ok, 16);
    if (ok)
        ui->tvDasm->selectRow(static_cast<int>(addr / 4));
}

void MainWindow::goto_cog()
{
    goto_hex("00000");
}

void MainWindow::goto_lut()
{
    goto_hex("00800");
}

void MainWindow::goto_rom()
{
    goto_hex("fc000");
}

void MainWindow::goto_address()
{
    GotoAddress dlg;
    if (QDialog::Accepted == dlg.exec()) {
        goto_hex(dlg.address());
    }
}

void MainWindow::set_opcodes_asm(int mode)
{
    p2_FORMAT_e format = static_cast<p2_FORMAT_e>(mode);
    ui->action_Asm_Opcodes_bin->setChecked(format == fmt_bin);
    ui->action_Asm_Opcodes_byt->setChecked(format == fmt_bit);
    ui->action_Asm_Opcodes_dec->setChecked(format == fmt_dec);
    ui->action_Asm_Opcodes_hex->setChecked(format == fmt_hex);
    P2AsmModel* amodel = asm_model();
    if (amodel)
        amodel->setOpcodeFormat(format);
    update_sizes_asm();
}

void MainWindow::set_opcodes_asm()
{
    QAction* act = qobject_cast<QAction*>(sender());
    Q_ASSERT(act);
    set_opcodes_asm(act->data().toInt());
}

void MainWindow::font_size_inc_asm()
{
    QFont font = ui->tvAsm->font();
    int size = font.pointSize();
    if (size < 24) {
        size++;
        font.setPointSize(size);
        set_font_asm(font);
    }
}

void MainWindow::font_size_dec_asm()
{
    QFont font = ui->tvAsm->font();
    int size = font.pointSize();
    if (size > 5) {
        size--;
        font.setPointSize(size);
        set_font_asm(font);
    }
}

void MainWindow::set_font_asm(const QFont& font)
{
    ui->tvAsm->setUpdatesEnabled(false);
    ui->tvSym->setUpdatesEnabled(false);
    ui->tbErr->setUpdatesEnabled(false);

    ui->tvAsm->setFont(font);
    P2AsmModel* amodel = asm_model();
    if (amodel)
        amodel->setFont(font);

    ui->tvSym->setFont(font);
    P2SymbolsModel* smodel = sym_model();
    if (smodel)
        smodel->setFont(font);

    ui->tbErr->setFont(font);

    update_sizes_asm();
    update_sizes_symbols();

    ui->tbErr->setUpdatesEnabled(true);
    ui->tvSym->setUpdatesEnabled(true);
    ui->tvAsm->setUpdatesEnabled(true);
}

void MainWindow::set_opcodes_dasm(int mode)
{
    P2DasmModel* dmodel = dasm_model();
    p2_FORMAT_e format = static_cast<p2_FORMAT_e>(mode);
    ui->action_Dasm_Opcodes_bin->setChecked(format == fmt_bin);
    ui->action_Dasm_Opcodes_byt->setChecked(format == fmt_bit);
    ui->action_Dasm_Opcodes_dec->setChecked(format == fmt_dec);
    ui->action_Dasm_Opcodes_hex->setChecked(format == fmt_hex);
    dmodel->setOpcodeFormat(format);
    update_sizes_dasm();
}

void MainWindow::set_opcodes_dasm()
{
    QAction* act = qobject_cast<QAction*>(sender());
    Q_ASSERT(act);
    set_opcodes_dasm(act->data().toInt());
}

void MainWindow::font_size_inc_dasm()
{
    QFont font = ui->tvDasm->font();
    int size = font.pointSize();
    if (size < 24) {
        size++;
        font.setPointSize(size);
        set_font_dasm(font);
    }
}

void MainWindow::font_size_dec_dasm()
{
    QFont font = ui->tvDasm->font();
    int size = font.pointSize();
    if (size > 5) {
        size--;
        font.setPointSize(size);
        set_font_dasm(font);
    }
}

void MainWindow::set_font_dasm(const QFont& font)
{
    ui->tvDasm->setUpdatesEnabled(false);

    ui->tvDasm->setFont(font);
    P2DasmModel* dmodel = dasm_model();
    if (dmodel)
        dmodel->setFont(font);
    update_sizes_dasm();

    ui->tvDasm->setUpdatesEnabled(true);
}

void MainWindow::header_columns_asm(QPoint pos)
{
    P2AsmModel* amodel = asm_model();
    if (!amodel)
        return;

    QMenu popup;
    QAction* tact = new QAction(QString("- %1 -").arg(tr("Select visible columns")));
    tact->setEnabled(false);
    popup.addAction(tact);
    for (int column = 0; column < amodel->columnCount(); column++) {
        QString title = amodel->headerData(column, Qt::Horizontal).toString();
        if (title == QStringLiteral("T"))
            title = tr("Tokens");
        if (title == QStringLiteral("E"))
            title = tr("Errors");
        if (title == QStringLiteral("S"))
            title = tr("Symbols");
        QAction* act = new QAction(title);
        act->setData(column);
        act->setCheckable(true);
        act->setChecked(!ui->tvAsm->isColumnHidden(column));
        popup.addAction(act);
    }

    // Popup menu
    QHeaderView* hv = ui->tvAsm->horizontalHeader();
    QAction* act = popup.exec(hv->mapToGlobal(pos));
    if (!act)
        return;
    // hide or show the selected column
    P2AsmModel::column_e column = static_cast<P2AsmModel::column_e>(act->data().toInt());
    ui->tvAsm->setColumnHidden(column, !act->isChecked());
}

void MainWindow::header_columns_dasm(QPoint pos)
{
    const P2DasmModel* dmodel = dasm_model();
    if (!dmodel)
        return;

    QMenu popup;
    QAction* tact = new QAction(QString("- %1 -").arg(tr("Select visible columns")));
    tact->setEnabled(false);
    popup.addAction(tact);
    for (int column = 0; column < dmodel->columnCount(); column++) {
        QString title = dmodel->headerData(column, Qt::Horizontal).toString();
        QAction* act = new QAction(title);
        act->setData(column);
        act->setCheckable(true);
        act->setChecked(!ui->tvDasm->isColumnHidden(column));
        popup.addAction(act);
    }

    // Popup menu
    QHeaderView* hv = ui->tvDasm->horizontalHeader();
    QAction* act = popup.exec(hv->mapToGlobal(pos));
    if (!act)
        return;
    // hide or show the selected column
    P2DasmModel::column_e column = static_cast<P2DasmModel::column_e>(act->data().toInt());
    ui->tvDasm->setColumnHidden(column, !act->isChecked());
}

void MainWindow::header_columns_sym(QPoint pos)
{
    P2SymbolsModel* smodel = sym_model();
    if (!smodel)
        return;

    QMenu popup;
    QAction* tact = new QAction(QString("- %1 -").arg(tr("Select visible columns")));
    tact->setEnabled(false);
    popup.addAction(tact);
    for (int column = 0; column < smodel->columnCount(); column++) {
        QString title = smodel ->headerData(column, Qt::Horizontal).toString();
        QAction* act = new QAction(title);
        act->setData(column);
        act->setCheckable(true);
        act->setChecked(!ui->tvSym->isColumnHidden(column));
        popup.addAction(act);
    }

    // Popup menu
    QHeaderView* hv = ui->tvSym->horizontalHeader();
    QAction* act = popup.exec(hv->mapToGlobal(pos));
    if (!act)
        return;
    // hide or show the selected column
    P2SymbolsModel::column_e column = static_cast<P2SymbolsModel::column_e>(act->data().toInt());
    ui->tvSym->setColumnHidden(column, !act->isChecked());
}

void MainWindow::hub_single_step()
{
    m_hub->execute(ncogs*2);
    for (int id = 0; id < ncogs; id++)
        m_vcog[id]->updateView();
}

void MainWindow::load_object(const QString& filename)
{
    P2DasmModel* dmodel = dasm_model();
    if (!dmodel)
        return;
    QFileInfo info(filename);
    ui->tabWidget->setTabText(1, QString("%1 [%2]").arg(tr("Disassembler")).arg(info.fileName()));

    if (!info.path().startsWith(QChar(':')))
        m_hub->set_pathname(info.path());
    m_hub->load_obj(filename);
    dmodel->invalidate();
    update_sizes_dasm();
    ui->tvDasm->update();
}

void MainWindow::load_object_random()
{
    QDir dir(QStringLiteral(":/bin"));
    QStringList name_filters =
            QStringList()
            << QStringLiteral("*.bin")
            << QStringLiteral("*.obj");
    QDir::Filters filters = QDir::Files;
    QDir::SortFlags sort = QDir::Name;
    QStringList files = dir.entryList(name_filters, filters, sort);
    QString objectfile = QString("%1/%2")
                         .arg(dir.path())
                         .arg(files[qrand() % files.count()]);
    load_object(objectfile);
}

void MainWindow::open_source(const QString& sourcefile)
{
    QSettings s;
    s.beginGroup(key_source);
    QString directory = s.value(key_directory).toString();
    QString filename = s.value(key_filename).toString();
    QStringList history = s.value(key_history).toStringList();
    s.endGroup();

    if (sourcefile.isEmpty()) {
        QFileDialog dlg(this);

        dlg.setAcceptMode(QFileDialog::AcceptOpen);
        dlg.setDefaultSuffix(QStringLiteral("spin2"));
        dlg.setDirectory(directory);
        dlg.setFilter(QDir::AllEntries | QDir::Readable);
        dlg.setHistory(history);
        dlg.setNameFilter(tr("Spin2 source files (*.spin2)"));
        QString propeller2_path = QString("%1/Propeller2").arg(QDir::homePath());
        QString appdata_path = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
        QList<QUrl> urls = QList<QUrl>()
            << QUrl::fromLocalFile(propeller2_path)
            << QUrl::fromLocalFile(appdata_path);

        if (QDialog::Accepted != dlg.exec())
            return;
        QStringList selected = dlg.selectedFiles();
        if (selected.isEmpty())
            return;

        directory = dlg.directory().path();
        filename = selected.first();
        filename.remove(QDir::cleanPath(directory) + QStringLiteral("/"));
        history = dlg.history();
        s.beginGroup(key_source);
        s.setValue(key_directory, directory);
        s.setValue(key_filename, filename);
        s.setValue(key_history, history);
        s.endGroup();

        filename = QString("%1/%2")
                   .arg(directory)
                   .arg(filename);
    } else {
        filename = sourcefile;
    }

    load_source(filename);
}

void MainWindow::load_source(const QString& filename)
{
    QFileInfo info(filename);
    ui->tabWidget->setTabText(0, QString("%1 [%2]").arg(tr("Assembler")).arg(info.fileName()));

    if (!info.path().startsWith(QChar(':')))
        m_asm->set_pathname(info.path());
    m_asm->load(filename);
    P2AsmModel* amodel = asm_model();
    if (amodel)
        amodel->invalidate();
    update_sizes_asm();
    update_sizes_symbols();
    if (amodel)
        ui->tvAsm->setCurrentIndex(amodel->index(0, P2AsmModel::c_Source));
    ui->tvAsm->update();

    QLabel* lbl_lines = ui->toolbarAsm->findChild<QLabel*>(key_lines);
    if (lbl_lines)
        lbl_lines->setText(tr("%1 lines").arg(m_asm->count()));
    assemble();
}

void MainWindow::load_source_random()
{
    QDir dir(QStringLiteral(":/spin2"));
    QStringList name_filters = QStringList() << QStringLiteral("*.spin2");
    QDir::Filters filters = QDir::Files;
    QDir::SortFlags sort = QDir::Name;
    QStringList files = dir.entryList(name_filters, filters, sort);
    QString sourcecode = QString("%1/%2")
                         .arg(dir.path())
                         .arg(files[qrand() % files.count()]);
    load_source(sourcecode);
}

void MainWindow::assemble()
{
    QStringList source = m_asm->source();
    ui->tbErr->clear();
    ui->splSource->widget(2)->setVisible(false);
    m_num_errors = 0;

    qint64 t0 = QDateTime::currentMSecsSinceEpoch();
    if (m_asm->assemble(source)) {
        qint64 t1 = QDateTime::currentMSecsSinceEpoch();
        QLabel* status = ui->statusBar->findChild<QLabel*>(key_status);
        if (status)
            status->setText(tr("Assembly took %1 ms.").arg(t1 - t0));

        P2SymbolsModel* smodel = sym_model();
        if (smodel)
            smodel->setTable(m_asm->symbols());
        // Inspect result
        const QModelIndex idx = ui->tvAsm->currentIndex();

        P2AsmModel* amodel = asm_model();
        if (amodel)
            amodel->invalidate();
        ui->tvAsm->resizeRowsToContents();
        ui->tvAsm->setCurrentIndex(idx);
        QString text = ui->tbErr->toPlainText();
        if (text.isEmpty()) {
            text = m_asm->listing().join(QChar::LineFeed);
            ui->tbErr->setText(text);
            ui->splSource->widget(2)->setVisible(true);
        }
    }
}

void MainWindow::palette_setup()
{
    PaletteSetup* dlg = ui->dlgPaletteSetup->findChild<PaletteSetup *>();

    if (!dlg) {
        // On the first view create a vertical box layout in the QWidget ui->dlgPaletteSetup
        QLayout* lay = new QVBoxLayout(ui->dlgPaletteSetup);
        dlg = new PaletteSetup;
        // and add the PaletteSetup dialog to it
        lay->addWidget(dlg);
    }
    if (dlg->isVisible()) {
        // just hide the dialog
        dlg->hide();
    } else {
        // On changed palette invalidate the P2AsmModel
        P2AsmModel* amodel = asm_model();
        connect(dlg, SIGNAL(changedPalette()), amodel, SLOT(invalidate()), Qt::UniqueConnection);
        // show the dialog
        dlg->show();
    }
}

void MainWindow::preferences()
{
    Preferences dlg(this);
    dlg.set_pnut(m_asm->pnut());
    dlg.set_v33mode(m_asm->v33mode());
    dlg.set_file_errors(m_asm->file_errors());
    dlg.set_font_asm(ui->tvAsm->font());
    dlg.set_font_dasm(ui->tvDasm->font());
    if (QDialog::Accepted != dlg.exec())
        return;
    m_asm->set_pnut(dlg.pnut());
    m_asm->set_v33mode(dlg.v33mode());
    m_asm->set_file_errors(dlg.file_errors());
    set_font_asm(dlg.font_asm());
    set_font_dasm(dlg.font_dasm());
}

void MainWindow::print_error(int pass, int line, const QString& message)
{
    QString str_pass = QString("%1 #%2 - ")
                       .arg(tr("Pass"))
                       .arg(QString::number(pass));
    QString str_line = QString("%1 <a href=\"%2#%3\">#%3</a>: ")
                       .arg(tr("Line"))
                        .arg(key_tv_asm)
                       .arg(QString::number(line));
    QString error = str_pass + str_line + message;
    ui->tbErr->append(error);
    m_num_errors++;
    ui->splSource->widget(2)->setVisible(true);
}

void MainWindow::goto_line(const QUrl& url)
{
    const P2SymbolTable& table = m_asm->symbols();
    const QString& path = url.path();
    const QString& name = url.query();
    const QStringList& frag = url.fragment().split(QChar(','));
    const int lineno = frag.value(0).toInt();
    const int pos = frag.value(1).toInt();
    const int len = frag.value(2).toInt();

    const P2Symbol symbol = table->symbol(name);
    const P2Words& words = m_asm->words(lineno);
    P2Word word;
    for (int i = 0; !word.isValid() && i < words.count(); i++)
        if (pos == words[i].pos() && len == words[i].len())
            word = words[i];

    if (path == key_tv_asm) {
        P2AsmModel* amodel = asm_model();
        if (amodel) {
            const QModelIndex idx = amodel->index(lineno - 1, P2AsmModel::c_Source);
            amodel->set_highlight(idx, symbol, word);
            ui->tvAsm->setCurrentIndex(idx);
            ui->tvAsm->viewport()->repaint();
            // ui->tvAsm->setFocus(Qt::OtherFocusReason);
        }
    }
}

void MainWindow::goto_line_number()
{
    GotoLine dlg(this);
    if (QDialog::Accepted != dlg.exec())
        return;
    QUrl url(key_tv_asm);
    url.setFragment(QString::number(dlg.number()));
    goto_line(url);
}

void MainWindow::goto_definition(const QModelIndex& index)
{
    const QAbstractItemModel* model = index.model();
    const P2SymbolsModel::column_e column = static_cast<P2SymbolsModel::column_e>(index.column());
    switch (column) {
    case P2SymbolsModel::c_Definition:
        {
            QVariant var_sym = model->data(model->index(index.row(), P2SymbolsModel::c_Name), Qt::EditRole);
            if (var_sym.canConvert(qMetaTypeId<P2Symbol>())){
                P2Symbol sym = qvariant_cast<P2Symbol>(var_sym);
                goto_line(sym->url(sym->definition()));
            }
        }
        break;
    default:
        break;
    }
}

void MainWindow::resize_splitter_source()
{
    while (m_source_percent + m_symbols_percent + m_errors_percent > 100) {
        if (m_errors_percent > m_symbols_percent) {
            m_errors_percent--;
        } else {
            m_symbols_percent--;
        }
    }
    QList<int> resized = {m_source_percent, m_symbols_percent, m_errors_percent};
    const QList<int> sizes = ui->splSource->sizes();
    const int height = ui->tabAsm->height();
    for (int i = 0; i < resized.count(); i++)
        resized[i] = height * resized[i] / 100;
    ui->splSource->setSizes(resized);
    ui->splSource->widget(2)->setVisible(m_num_errors > 0);
}

P2AsmModel* MainWindow::asm_model()
{
    P2AsmModel* amodel = qobject_cast<P2AsmModel*>(ui->tvAsm->model());
    return amodel;
}

P2DasmModel* MainWindow::dasm_model()
{
    P2DasmModel* dmodel = qobject_cast<P2DasmModel*>(ui->tvDasm->model());
    return dmodel;
}

P2SymbolsModel* MainWindow::sym_model()
{
    P2SymbolsModel* smodel = nullptr;
    P2SymbolSorter* sorter = qobject_cast<P2SymbolSorter*>(ui->tvSym->model());
    if (sorter)
        smodel = qobject_cast<P2SymbolsModel*>(sorter->sourceModel());
    return smodel;
}

void MainWindow::set_lowercase_dasm(bool check)
{
    P2DasmModel* dmodel = dasm_model();
    ui->action_Dasm_Lowercase->setChecked(check);
    m_dasm->set_lowercase(check);
    p2_LONG PC = m_hub->cog(0)->rd_PC();
    int row = static_cast<int>((PC < HUB_ADDR0) ? PC : PC / 4);
    dmodel->invalidate();
    ui->tvDasm->selectRow(row);
}

void MainWindow::setup_asm()
{
    connect(m_asm,
            SIGNAL(Error(int,int,QString)),
            SLOT(print_error(int,int,QString)));
    ui->splSource->setCollapsible(1, true);
    ui->splSource->setCollapsible(2, true);


    // tvAsm setup
    ui->tvAsm->setFocusPolicy(Qt::ClickFocus);
    ui->tvAsm->setEditTriggers(QAbstractItemView::DoubleClicked
                               | QAbstractItemView::SelectedClicked
                               | QAbstractItemView::EditKeyPressed
                               | QAbstractItemView::AnyKeyPressed);
    ui->tvAsm->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tvAsm->setSelectionBehavior(QAbstractItemView::SelectItems);

    ui->tvAsm->setModel(new P2AsmModel(m_asm, ui->tvAsm));

    // Set a delegate for the opcode column
    QAbstractItemDelegate* od = ui->tvAsm->itemDelegateForColumn(P2AsmModel::c_Opcode);
    ui->tvAsm->setItemDelegateForColumn(P2AsmModel::c_Opcode, new P2OpcodeDelegate);
    delete od;

    // Set a delegate for the source column
    QAbstractItemDelegate* sd = ui->tvAsm->itemDelegateForColumn(P2AsmModel::c_Source);
    ui->tvAsm->setItemDelegateForColumn(P2AsmModel::c_Source, new P2SourceDelegate);
    delete sd;

    ui->tvAsm->resizeColumnsToContents();

    QHeaderView* hh = ui->tvAsm->horizontalHeader();
    hh->setStretchLastSection(true);
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh,
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(header_columns_asm(QPoint)),
            Qt::UniqueConnection);
}

void MainWindow::setup_sym()
{
    QAbstractItemDelegate* sd = ui->tvSym->itemDelegateForColumn(P2SymbolsModel::c_References);
    P2ReferencesDelegate* delegate = new P2ReferencesDelegate;
    connect(delegate, SIGNAL(urlSelected(QUrl)), SLOT(goto_line(QUrl)));
    ui->tvSym->setItemDelegateForColumn(P2SymbolsModel::c_References, delegate);
    delete sd;

    P2SymbolsModel* smodel = new P2SymbolsModel;
    P2SymbolSorter* sorter = new P2SymbolSorter(this);
    sorter->setSourceModel(smodel);
    ui->tvSym->setModel(sorter);
    ui->tvSym->setSortingEnabled(true);
    connect(ui->tvSym,
            SIGNAL(clicked(const QModelIndex &)),
            SLOT(goto_definition(const QModelIndex &)), Qt::UniqueConnection);
    connect(ui->tvSym,
            SIGNAL(activated(const QModelIndex &)),
            SLOT(goto_definition(const QModelIndex &)), Qt::UniqueConnection);

    ui->tvSym->resizeColumnsToContents();

    QHeaderView* hh = ui->tvSym->horizontalHeader();
    hh->setStretchLastSection(true);
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh,
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(header_columns_sym(QPoint)),
            Qt::UniqueConnection);
}

void MainWindow::setup_err()
{
    // prevent opening of (external) links
    ui->tbErr->setOpenLinks(false);
    ui->tbErr->setOpenExternalLinks(false);
    // but connect to the anchorClicked(QUrl) signal
    connect(ui->tbErr,
            SIGNAL(anchorClicked(QUrl)),
            SLOT(goto_line(QUrl)), Qt::UniqueConnection);
}

void MainWindow::setup_dasm()
{
    ui->tvDasm->setModel(new P2DasmModel(m_dasm, ui->tvDasm));
    update_sizes_dasm();

    QHeaderView* hh = ui->tvDasm->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    hh->setStretchLastSection(true);
    connect(hh,
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(header_columns_dasm(QPoint)),
            Qt::UniqueConnection);
}

void MainWindow::setup_fonts()
{
    static const QString preferred_font = QStringLiteral("Source Code Pro");
    QFont font = QFont(preferred_font);

    if (font.family() != preferred_font)
        font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
    font.setPointSize(9);
    m_font_asm = font;
    m_font_dasm = font;

    font.setPointSize(11);
    ui->tabWidget->setFont(font);

    font.setPointSize(m_font_asm.pointSize());
    ui->tvAsm->setFont(font);
    ui->tvSym->setFont(font);
    ui->tbErr->setFont(font);

    font.setPointSize(m_font_dasm.pointSize());
    ui->tvDasm->setFont(font);
}

void MainWindow::setup_tabwidget()
{
    QTabBar* bar = ui->tabWidget->tabBar();
    // FIXME: how to identify tabs if the order changes?
    bar->setTabData(0, id_asm);
    bar->setTabData(1, id_dasm);
    bar->setTabData(2, id_hub);
}

void MainWindow::setup_menu()
{
    connect(ui->action_Open_src, SIGNAL(triggered()), SLOT(open_source()));
    connect(ui->action_Open_src_random, SIGNAL(triggered()), SLOT(load_source_random()));
    // connect(ui->action_Open_obj, SIGNAL(triggered()), SLOT(openObject()));
    connect(ui->action_Open_obj_random, SIGNAL(triggered()), SLOT(load_object_random()));
    connect(ui->action_Go_to_line, SIGNAL(triggered()), SLOT(goto_line_number()));
    connect(ui->action_Assemble, SIGNAL(triggered()), SLOT(assemble()));
    connect(ui->action_SingleStep, SIGNAL(triggered()), SLOT(hub_single_step()));

    connect(ui->action_Palette_setup, SIGNAL(triggered()), SLOT(palette_setup()));
    connect(ui->action_Preferences, SIGNAL(triggered()), SLOT(preferences()));

    connect(ui->action_Asm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(set_opcodes_asm()));
    connect(ui->action_Asm_Opcodes_byt, SIGNAL(triggered(bool)), SLOT(set_opcodes_asm()));
    connect(ui->action_Asm_Opcodes_dec, SIGNAL(triggered(bool)), SLOT(set_opcodes_asm()));
    connect(ui->action_Asm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(set_opcodes_asm()));

    connect(ui->action_Asm_DecFontSize, SIGNAL(triggered(bool)), SLOT(font_size_dec_asm()));
    connect(ui->action_Asm_IncFontSize, SIGNAL(triggered(bool)), SLOT(font_size_inc_asm()));

    connect(ui->action_Dasm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(set_opcodes_dasm()));
    connect(ui->action_Dasm_Opcodes_byt, SIGNAL(triggered(bool)), SLOT(set_opcodes_dasm()));
    connect(ui->action_Dasm_Opcodes_dec, SIGNAL(triggered(bool)), SLOT(set_opcodes_dasm()));
    connect(ui->action_Dasm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(set_opcodes_dasm()));

    connect(ui->action_Dasm_DecFontSize, SIGNAL(triggered(bool)), SLOT(font_size_dec_dasm()));
    connect(ui->action_Dasm_IncFontSize, SIGNAL(triggered(bool)), SLOT(font_size_inc_dasm()));
    connect(ui->action_Dasm_Lowercase, SIGNAL(triggered(bool)), SLOT(set_lowercase_dasm(bool)));

    connect(ui->action_Go_to_COG, SIGNAL(triggered()), SLOT(goto_cog()));
    connect(ui->action_Go_to_LUT, SIGNAL(triggered()), SLOT(goto_lut()));
    connect(ui->action_Go_to_ROM, SIGNAL(triggered()), SLOT(goto_rom()));
    connect(ui->action_Go_to_address, SIGNAL(triggered()), SLOT(goto_address()));

    // help
    connect(ui->action_Quit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->action_About, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->action_AboutQt5, SIGNAL(triggered(bool)), this, SLOT(aboutQt5()));
    connect(ui->action_P2_opcodes, SIGNAL(triggered(bool)), this, SLOT(help_opcodes()));

}

void MainWindow::setup_toolbars()
{
    // HUB toolbar
    ui->toolbarHub->addAction(ui->action_SingleStep);

    // Assembler toolbar
    ui->toolbarAsm->addAction(ui->action_Open_src);
    ui->toolbarAsm->addAction(ui->action_Open_src_random);
    ui->toolbarAsm->addSeparator();
    ui->toolbarAsm->addAction(ui->action_Assemble);
    ui->toolbarAsm->addSeparator();

    ui->action_Asm_Opcodes_bin->setData(fmt_bin);
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_bin);
    ui->action_Asm_Opcodes_byt->setData(fmt_bit);
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_byt);
    ui->action_Asm_Opcodes_dec->setData(fmt_dec);
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_dec);
    ui->action_Asm_Opcodes_hex->setData(fmt_hex);
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_hex);

    ui->toolbarAsm->addSeparator();
    ui->toolbarAsm->addAction(ui->action_Asm_DecFontSize);
    ui->toolbarAsm->addAction(ui->action_Asm_IncFontSize);
    ui->toolbarAsm->addSeparator();
    ui->toolbarAsm->addAction(ui->action_Palette_setup);

    // Expanding spacer
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolbarAsm->addWidget(spacer);

    // Label for count of lines
    QLabel* lbl_lines = new QLabel(tr("%1 lines").arg(0));
    lbl_lines->setObjectName(key_lines);
    lbl_lines->setMinimumWidth(12*10);
    lbl_lines->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lbl_lines->setFrameStyle(QLabel::StyledPanel);
    lbl_lines->setFrameStyle(QLabel::StyledPanel);
    lbl_lines->setFrameShadow(QLabel::Sunken);
    lbl_lines->setBackgroundRole(QPalette::Base);
    ui->toolbarAsm->addWidget(lbl_lines);

    // Disassembler toolbar
    ui->toolbarDasm->addAction(ui->action_Open_obj);
    ui->toolbarDasm->addAction(ui->action_Open_obj_random);
    ui->toolbarDasm->addSeparator();

    ui->action_Dasm_Opcodes_bin->setData(fmt_bin);
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_bin);
    ui->action_Dasm_Opcodes_byt->setData(fmt_bit);
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_byt);
    ui->action_Dasm_Opcodes_dec->setData(fmt_dec);
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_dec);
    ui->action_Dasm_Opcodes_hex->setData(fmt_hex);
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_hex);

    ui->toolbarDasm->addSeparator();
    ui->toolbarDasm->addAction(ui->action_Go_to_COG);
    ui->toolbarDasm->addAction(ui->action_Go_to_LUT);
    ui->toolbarDasm->addAction(ui->action_Go_to_ROM);
    ui->toolbarDasm->addAction(ui->action_Go_to_address);

    ui->toolbarDasm->addSeparator();
    ui->toolbarDasm->addAction(ui->action_Dasm_Lowercase);

    ui->toolbarDasm->addSeparator();
    ui->toolbarDasm->addAction(ui->action_Dasm_DecFontSize);
    ui->toolbarDasm->addAction(ui->action_Dasm_IncFontSize);

    // Main toolbar

    ui->toolbar->addAction(ui->action_About);
    ui->toolbar->addSeparator();
    ui->toolbar->addAction(ui->action_P2_opcodes);

    ui->toolbar->addSeparator();
    ui->toolbar->addAction(ui->action_Quit);
}

void MainWindow::setup_statusbar()
{
    QLabel* status = new QLabel(tr("Status"));
    status->setObjectName(key_status);
    ui->statusBar->addWidget(status);
}

void MainWindow::update_sizes_asm()
{
    ui->tvAsm->resizeColumnsToContents();
    ui->tvAsm->resizeRowsToContents();
}

/**
 * @brief Update tvDasm column sizes
 */
void MainWindow::update_sizes_dasm()
{
    ui->tvDasm->resizeColumnsToContents();
}

void MainWindow::update_sizes_symbols()
{
    QFont font = ui->tvSym->font();
    QFontMetrics metrics(font);
}

void MainWindow::setup_cog_views()
{
    m_vcog = QVector<P2CogView*>()
             << ui->cog0 << ui->cog1 << ui->cog2 << ui->cog3
             << ui->cog4 << ui->cog5 << ui->cog6 << ui->cog7;
    for (int id = 0; id < 8; id++) {
        P2CogView* vcog = m_vcog[id];
        if (!vcog)
            continue;
        if (id >= ncogs) {
            vcog->hide();
            continue;
        }
        vcog->setCog(m_hub->cog(id));
    }
}

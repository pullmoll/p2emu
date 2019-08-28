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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "gotoaddress.h"
#include "gotoline.h"
#include "p2asmlisting.h"
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

static const int ncogs = 8;
static const QLatin1String key_windowGeometry("windowGeometry");
static const QLatin1String key_windowState("windowState");
static const QLatin1String grp_assembler("assembler");
static const QLatin1String grp_disassembler("disassembler");
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
static const QLatin1String key_font_size("font_size");

static const QLatin1String key_source("source");
static const QLatin1String key_directory("directory");
static const QLatin1String key_filename("filename");
static const QLatin1String key_history("directory");

static const QLatin1String key_status("status");


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_vcog()
    , m_hub(new P2Hub(ncogs, this))
    , m_asm(new P2Asm(this))
    , m_amodel(new P2AsmModel(m_asm))
    , m_dasm(new P2Dasm(m_hub->cog(0)))
    , m_dmodel(new P2DasmModel(m_dasm))
    , m_smodel(new P2SymbolsModel())
{
    ui->setupUi(this);
    connect(ui->action_Quit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->action_About, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->action_AboutQt5, SIGNAL(triggered(bool)), this, SLOT(aboutQt5()));

    setWindowTitle(QString("%1 v%2").arg(qApp->applicationName()).arg(qApp->applicationVersion()));
    setupMenu();
    setupTabWidget();
    setupToolbars();
    setupStatusbar();

    m_hub->load(":/ROM_Booter_v33_01j.bin");

    setupAssembler();
    setupDisassembler();
    setupCogView();

    restoreSettings();

    // QString sourcecode = QStringLiteral(":/ROM_Booter_v33_01j.spin2");
    QString sourcecode = QStringLiteral(":/P2-qz80-rr032.spin2");
    openSource(sourcecode);
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::saveSettingsAsm(QSettings& s)
{
    s.beginGroup(grp_assembler);
    s.setValue(key_opcodes, m_amodel->opcode_format());
    s.setValue(key_column_origin, ui->tvAsm->isColumnHidden(P2AsmModel::c_Origin));
    s.setValue(key_column_address, ui->tvAsm->isColumnHidden(P2AsmModel::c_Address));
    s.setValue(key_column_opcode, ui->tvAsm->isColumnHidden(P2AsmModel::c_Opcode));
    s.setValue(key_column_tokens, ui->tvAsm->isColumnHidden(P2AsmModel::c_Tokens));
    s.setValue(key_column_symbols, ui->tvAsm->isColumnHidden(P2AsmModel::c_Symbols));
    s.setValue(key_column_errors, ui->tvAsm->isColumnHidden(P2AsmModel::c_Errors));
    s.setValue(key_column_source, ui->tvAsm->isColumnHidden(P2AsmModel::c_Source));
    QModelIndex index = ui->tvAsm->currentIndex();
    s.setValue(key_current_row, index.row());
    s.setValue(key_current_column, index.column());
    s.setValue(key_font_size, ui->tvAsm->font().pixelSize());
    s.endGroup();
}

void MainWindow::saveSettingsDasm(QSettings& s)
{
    s.beginGroup(grp_disassembler);
    s.setValue(key_opcodes, m_dmodel->opcode_format());
    s.setValue(key_lowercase, ui->action_Dasm_Lowercase->isChecked());
    s.setValue(key_column_address, ui->tvDasm->isColumnHidden(P2DasmModel::c_Address));
    s.setValue(key_column_opcode, ui->tvDasm->isColumnHidden(P2DasmModel::c_Opcode));
    s.setValue(key_column_instruction, ui->tvDasm->isColumnHidden(P2DasmModel::c_Instruction));
    s.setValue(key_column_description, ui->tvDasm->isColumnHidden(P2DasmModel::c_Description));
    QModelIndex index = ui->tvDasm->currentIndex();
    s.setValue(key_current_row, index.row());
    s.setValue(key_current_column, index.column());
    s.setValue(key_font_size, ui->tvDasm->font().pixelSize());
    s.endGroup();
}

void MainWindow::saveSettings()

{
    QSettings s;
    s.setValue(key_windowGeometry, saveGeometry());
    s.setValue(key_windowState, saveState());
    saveSettingsAsm(s);
    saveSettingsDasm(s);

}
void MainWindow::restoreSettingsAsm(QSettings& s)
{
    s.beginGroup(grp_assembler);
    setAsmOpcodes(s.value(key_opcodes, fmt_bin).toInt());
    int row = s.value(key_current_row).toInt();
    int column = s.value(key_current_row).toInt();
    ui->tvAsm->setCurrentIndex(m_amodel->index(row, column));
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Origin, s.value(key_column_origin, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Address, s.value(key_column_address, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Opcode, s.value(key_column_opcode, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Tokens, s.value(key_column_tokens, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Symbols, s.value(key_column_symbols, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Errors, s.value(key_column_errors, false).toBool());
    ui->tvAsm->setColumnHidden(P2AsmModel::c_Source, s.value(key_column_source, false).toBool());
    setAsmFontSize(s.value(key_font_size, 8).toInt());
    s.endGroup();

}

void MainWindow::restoreSettingsDasm(QSettings& s)
{
    s.beginGroup(grp_disassembler);
    setDasmOpcodes(s.value(key_opcodes, fmt_bin).toInt());
    setDasmLowercase(s.value(key_lowercase).toBool());
    ui->tvDasm->selectRow(s.value(key_current_row).toInt());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Address, s.value(key_column_address, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Opcode, s.value(key_column_opcode, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Instruction, s.value(key_column_instruction, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Description, s.value(key_column_description, false).toBool());
    setDasmFontSize(s.value(key_font_size, 8).toInt());
    s.endGroup();
}

void MainWindow::restoreSettings()
{
    QSettings s;
    restoreGeometry(s.value(key_windowGeometry).toByteArray());
    restoreState(s.value(key_windowState).toByteArray());
    restoreSettingsAsm(s);
    restoreSettingsDasm(s);
}

void MainWindow::about()
{
    About dlg;
    dlg.setApplicationName(qApp->applicationName());
    dlg.setApplicationVersion(qApp->applicationVersion());
    dlg.adjustSize();
    dlg.exec();
}

void MainWindow::aboutQt5()
{
    qApp->aboutQt();
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

void MainWindow::setAsmOpcodes(int mode)
{
    p2_opcode_format_e format = static_cast<p2_opcode_format_e>(mode);
    ui->action_Asm_Opcodes_bin->setChecked(format == fmt_bin);
    ui->action_Asm_Opcodes_byt->setChecked(format == fmt_byt);
    ui->action_Asm_Opcodes_oct->setChecked(format == fmt_oct);
    ui->action_Asm_Opcodes_dec->setChecked(format == fmt_dec);
    ui->action_Asm_Opcodes_hex->setChecked(format == fmt_hex);
    m_amodel->setOpcodeFormat(format);
    updateAsmColumnSizes();
}

void MainWindow::setAsmOpcodes()
{
    QAction* act = qobject_cast<QAction*>(sender());
    Q_ASSERT(act);
    setAsmOpcodes(act->data().toInt());
}

void MainWindow::incAsmFontSize()
{
    QFont font = ui->tvAsm->font();
    int size = font.pixelSize();
    if (size < 24) {
        size++;
        setAsmFontSize(size);
    }
}

void MainWindow::decAsmFontSize()
{
    QFont font = ui->tvAsm->font();
    int size = font.pixelSize();
    if (size > 5) {
        size--;
        setAsmFontSize(size);
    }
}

void MainWindow::setAsmFontSize(int size)
{
    QFont font = ui->tvAsm->font();
    font.setPixelSize(size);
    ui->tvAsm->setFont(font);
    m_amodel->setFont(font);
    ui->tvSymbols->setFont(font);
    m_smodel->setFont(font);
    updateAsmColumnSizes();
    updateSymbolsColumnSizes();
}

void MainWindow::setDasmOpcodes(int mode)
{
    p2_opcode_format_e format = static_cast<p2_opcode_format_e>(mode);
    ui->action_Dasm_Opcodes_bin->setChecked(format == fmt_bin);
    ui->action_Dasm_Opcodes_byt->setChecked(format == fmt_byt);
    ui->action_Dasm_Opcodes_oct->setChecked(format == fmt_oct);
    ui->action_Dasm_Opcodes_dec->setChecked(format == fmt_dec);
    ui->action_Dasm_Opcodes_hex->setChecked(format == fmt_hex);
    m_dmodel->setOpcodeFormat(format);
    updateDasmColumnSizes();
}

void MainWindow::setDasmOpcodes()
{
    QAction* act = qobject_cast<QAction*>(sender());
    Q_ASSERT(act);
    setDasmOpcodes(act->data().toInt());
}

void MainWindow::incDasmFontSize()
{
    QFont font = ui->tvDasm->font();
    int size = font.pixelSize();
    if (size < 24) {
        size++;
        setDasmFontSize(size);
    }
}

void MainWindow::decDasmFontSize()
{
    QFont font = ui->tvDasm->font();
    int size = font.pixelSize();
    if (size > 5) {
        size--;
        setDasmFontSize(size);
    }
}

void MainWindow::setDasmFontSize(int size)
{
    QFont font = ui->tvDasm->font();
    font.setPixelSize(size);
    ui->tvDasm->setFont(font);
    m_dmodel->setFont(font);
    updateDasmColumnSizes();
}

void MainWindow::asmHeaderColums(const QPoint& pos)
{
    QList<P2AsmModel::column_e> columns = m_amodel->columns();

    QMenu m(tr("Select columns"));
    foreach(P2AsmModel::column_e column, columns) {
        QAction* act = new QAction(m_amodel->headerData(column, Qt::Horizontal).toString());
        act->setData(column);
        act->setCheckable(true);
        act->setChecked(!ui->tvAsm->isColumnHidden(column));
        m.addAction(act);
    }

    // Popup menu
    QHeaderView* hv = ui->tvAsm->horizontalHeader();
    QAction* act = m.exec(hv->mapToGlobal(pos));
    if (!act)
        return;
    // hide or show the selected column
    P2AsmModel::column_e column = static_cast<P2AsmModel::column_e>(act->data().toInt());
    ui->tvAsm->setColumnHidden(column, !act->isChecked());
}

void MainWindow::dasmHeaderColums(const QPoint& pos)
{
    QList<P2DasmModel::column_e> columns = m_dmodel->columns();

    QMenu m(tr("Select columns"));
    foreach(P2DasmModel::column_e column, columns) {
        QAction* act = new QAction(m_dmodel->headerData(column, Qt::Horizontal).toString());
        act->setData(column);
        act->setCheckable(true);
        act->setChecked(!ui->tvDasm->isColumnHidden(column));
        m.addAction(act);
    }

    // Popup menu
    QHeaderView* hv = ui->tvDasm->horizontalHeader();
    QAction* act = m.exec(hv->mapToGlobal(pos));
    if (!act)
        return;
    // hide or show the selected column
    P2DasmModel::column_e column = static_cast<P2DasmModel::column_e>(act->data().toInt());
    ui->tvDasm->setColumnHidden(column, !act->isChecked());
}

void MainWindow::symbolsHeaderColums(const QPoint& pos)
{
    QList<P2SymbolsModel::column_e> columns = m_smodel->columns();

    QMenu m(tr("Select columns"));
    foreach(P2SymbolsModel::column_e column, columns) {
        QAction* act = new QAction(m_smodel->headerData(column, Qt::Horizontal).toString());
        act->setData(column);
        act->setCheckable(true);
        act->setChecked(!ui->tvSymbols->isColumnHidden(column));
        m.addAction(act);
    }

    // Popup menu
    QHeaderView* hv = ui->tvSymbols->horizontalHeader();
    QAction* act = m.exec(hv->mapToGlobal(pos));
    if (!act)
        return;
    // hide or show the selected column
    P2SymbolsModel::column_e column = static_cast<P2SymbolsModel::column_e>(act->data().toInt());
    ui->tvSymbols->setColumnHidden(column, !act->isChecked());
}

void MainWindow::hubSingleStep()
{
    m_hub->execute(ncogs*2);
    for (int id = 0; id < ncogs; id++)
        m_vcog[id]->updateView();
}

void MainWindow::openSource(const QString& sourcefile)
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

        dlg.exec();
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

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QTextStream stream(&file);
    QStringList source;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        source += line;
    }
    m_asm->setSource(source);
    m_amodel->invalidate();
    updateAsmColumnSizes();
}

void MainWindow::assemble()
{
    QStringList source = m_asm->source();
    ui->tbErrors->setVisible(false);
    ui->tbErrors->clear();
    ui->tvSymbols->setVisible(false);

    qint64 t0 = QDateTime::currentMSecsSinceEpoch();
    if (m_asm->assemble(source)) {
        qint64 t1 = QDateTime::currentMSecsSinceEpoch();
        QLabel* status = ui->statusBar->findChild<QLabel*>(key_status);
        if (status)
            status->setText(tr("Assembly took %1 ms.").arg(t1 - t0));
        // Inspect result
        const QModelIndex idx = ui->tvAsm->currentIndex();
        m_amodel->invalidate();
        ui->tvAsm->setCurrentIndex(idx);

        m_smodel->setTable(m_asm->symbols());
        ui->tvSymbols->setVisible(true);
#if 0
        P2AsmListing dlg;
        dlg.setListing(m_asm->listing());
        dlg.setSymbols(m_asm->symbols());
        dlg.exec();
#endif
    }
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
    ui->tbErrors->append(error);
    if (!ui->tbErrors->isVisible()) {
        ui->tbErrors->setVisible(true);
        QList<int> sizes;
        sizes += ui->tvAsm->height();
        sizes += 120;
        ui->splSourceResults->setSizes(sizes);
        sizes.clear();
        sizes += ui->tvAsm->width() / 2;
        sizes += ui->tvAsm->width() / 2;
        ui->splResults->setSizes(sizes);
    }
}

void MainWindow::goto_line(const QUrl& url)
{
    QString path = url.path();
    QString frag = url.fragment();
    bool ok;
    int line = frag.toInt(&ok);
    if (path == key_tv_asm) {
        const QModelIndex idx = m_amodel->index(line - 1, P2AsmModel::c_Source);
        ui->tvAsm->setSelectionBehavior(QAbstractItemView::SelectItems);
        ui->tvAsm->setCurrentIndex(idx);
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

void MainWindow::setDasmLowercase(bool check)
{
    ui->action_Dasm_Lowercase->setChecked(check);
    m_dasm->setLowercase(check);
    p2_LONG PC = m_hub->cog(0)->rd_PC();
    int row = static_cast<int>((PC < HUB_ADDR0) ? PC : PC / 4);
    m_dmodel->invalidate();
    ui->tvDasm->selectRow(row);
}

void MainWindow::setupAssembler()
{
    connect(m_asm, SIGNAL(Error(int,int,QString)), SLOT(print_error(int,int,QString)));

    // prevent opening of (external) links
    ui->tbErrors->setOpenLinks(false);
    ui->tbErrors->setOpenExternalLinks(false);
    // but connect to the anchorClicked(QUrl) signal
    connect(ui->tbErrors, SIGNAL(anchorClicked(QUrl)), SLOT(goto_line(QUrl)), Qt::UniqueConnection);

    ui->tvAsm->setFocusPolicy(Qt::StrongFocus);
    ui->tvAsm->setEditTriggers(QAbstractItemView::AnyKeyPressed);

    QAbstractItemDelegate* od = ui->tvAsm->itemDelegateForColumn(P2AsmModel::c_Opcode);
    ui->tvAsm->setItemDelegateForColumn(P2AsmModel::c_Opcode, new P2OpcodeDelegate);
    delete od;

    QAbstractItemDelegate* sd = ui->tvAsm->itemDelegateForColumn(P2AsmModel::c_Source);
    ui->tvAsm->setItemDelegateForColumn(P2AsmModel::c_Source, new P2SourceDelegate);
    delete sd;

    QAbstractItemDelegate* ss = ui->tvSymbols->itemDelegateForColumn(P2SymbolsModel::c_References);
    P2ReferencesDelegate* delegate = new P2ReferencesDelegate;
    connect(delegate, SIGNAL(urlSelected(QUrl)), SLOT(goto_line(QUrl)));
    ui->tvSymbols->setItemDelegateForColumn(P2SymbolsModel::c_References, delegate);
    delete ss;

    ui->tvAsm->setModel(m_amodel);
    updateAsmColumnSizes();

    ui->tvSymbols->setModel(m_smodel);
    updateSymbolsColumnSizes();

    ui->tbErrors->setVisible(false);
    ui->tvSymbols->setVisible(false);
}

void MainWindow::setupDisassembler()
{
    ui->tvDasm->setModel(m_dmodel);
    updateDasmColumnSizes();
}

void MainWindow::setupMenu()
{
    connect(ui->action_Open, SIGNAL(triggered()), SLOT(openSource()));
    connect(ui->action_Go_to_line, SIGNAL(triggered()), SLOT(goto_line_number()));
}

void MainWindow::setupTabWidget()
{
    QTabBar* bar = ui->tabWidget->tabBar();
    // FIXME: how to identify tabs if the order changes?
    bar->setTabData(0, id_asm);
    bar->setTabData(1, id_dasm);
    bar->setTabData(2, id_hub);
}

void MainWindow::setupToolbars()
{
    // HUB toolbar
    connect(ui->action_SingleStep, SIGNAL(triggered()), SLOT(hubSingleStep()));
    ui->toolbarHub->addAction(ui->action_SingleStep);

    // Assembler toolbar
    connect(ui->action_Assemble, SIGNAL(triggered()), SLOT(assemble()));
    ui->toolbarAsm->addAction(ui->action_Assemble);

    ui->toolbarAsm->addSeparator();

    ui->action_Asm_Opcodes_bin->setData(fmt_bin);
    connect(ui->action_Asm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(setAsmOpcodes()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_bin);

    ui->action_Asm_Opcodes_byt->setData(fmt_byt);
    connect(ui->action_Asm_Opcodes_byt, SIGNAL(triggered(bool)), SLOT(setAsmOpcodes()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_byt);

    ui->action_Asm_Opcodes_oct->setData(fmt_oct);
    connect(ui->action_Asm_Opcodes_oct, SIGNAL(triggered(bool)), SLOT(setAsmOpcodes()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_oct);

    ui->action_Asm_Opcodes_dec->setData(fmt_dec);
    connect(ui->action_Asm_Opcodes_dec, SIGNAL(triggered(bool)), SLOT(setAsmOpcodes()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_dec);

    ui->action_Asm_Opcodes_hex->setData(fmt_hex);
    connect(ui->action_Asm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(setAsmOpcodes()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_hex);

    ui->toolbarAsm->addSeparator();

    connect(ui->action_Asm_DecFontSize, SIGNAL(triggered(bool)), SLOT(decAsmFontSize()));
    ui->toolbarAsm->addAction(ui->action_Asm_DecFontSize);

    connect(ui->action_Asm_IncFontSize, SIGNAL(triggered(bool)), SLOT(incAsmFontSize()));
    ui->toolbarAsm->addAction(ui->action_Asm_IncFontSize);

    // Disassembler toolbar
    ui->action_Dasm_Opcodes_bin->setData(fmt_bin);
    connect(ui->action_Dasm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(setDasmOpcodes()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_bin);

    ui->action_Dasm_Opcodes_byt->setData(fmt_byt);
    connect(ui->action_Dasm_Opcodes_byt, SIGNAL(triggered(bool)), SLOT(setDasmOpcodes()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_byt);

    ui->action_Dasm_Opcodes_oct->setData(fmt_oct);
    connect(ui->action_Dasm_Opcodes_oct, SIGNAL(triggered(bool)), SLOT(setDasmOpcodes()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_oct);

    ui->action_Dasm_Opcodes_dec->setData(fmt_dec);
    connect(ui->action_Dasm_Opcodes_dec, SIGNAL(triggered(bool)), SLOT(setDasmOpcodes()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_dec);

    ui->action_Dasm_Opcodes_hex->setData(fmt_hex);
    connect(ui->action_Dasm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(setDasmOpcodes()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_hex);

    ui->toolbarDasm->addSeparator();

    connect(ui->action_Go_to_COG, SIGNAL(triggered()), SLOT(goto_cog()));
    ui->toolbarDasm->addAction(ui->action_Go_to_COG);

    connect(ui->action_Go_to_LUT, SIGNAL(triggered()), SLOT(goto_lut()));
    ui->toolbarDasm->addAction(ui->action_Go_to_LUT);

    connect(ui->action_Go_to_ROM, SIGNAL(triggered()), SLOT(goto_rom()));
    ui->toolbarDasm->addAction(ui->action_Go_to_ROM);

    connect(ui->action_Go_to_address, SIGNAL(triggered()), SLOT(goto_address()));
    ui->toolbarDasm->addAction(ui->action_Go_to_address);

    ui->toolbarDasm->addSeparator();

    connect(ui->action_Dasm_Lowercase, SIGNAL(triggered(bool)), SLOT(setDasmLowercase(bool)));
    ui->toolbarDasm->addAction(ui->action_Dasm_Lowercase);

    ui->toolbarDasm->addSeparator();

    connect(ui->action_Dasm_DecFontSize, SIGNAL(triggered(bool)), SLOT(decDasmFontSize()));
    ui->toolbarDasm->addAction(ui->action_Dasm_DecFontSize);

    connect(ui->action_Dasm_IncFontSize, SIGNAL(triggered(bool)), SLOT(incDasmFontSize()));
    ui->toolbarDasm->addAction(ui->action_Dasm_IncFontSize);
}

void MainWindow::setupStatusbar()
{
    QLabel* status = new QLabel(tr("Status"));
    status->setObjectName(key_status);
    ui->statusBar->addWidget(status);
}

void MainWindow::updateAsmColumnSizes()
{
    QFont font = ui->tvAsm->font();
    QFontMetrics metrics(font);

    QHeaderView* vh = ui->tvAsm->verticalHeader();
    vh->setEnabled(true);
    vh->setDefaultSectionSize(metrics.ascent() + metrics.descent());

    QHeaderView* hh = ui->tvAsm->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    hh->setStretchLastSection(true);
    connect(hh, SIGNAL(customContextMenuRequested(QPoint)), SLOT(asmHeaderColums(QPoint)), Qt::UniqueConnection);

    for (int i = 0; i < m_amodel->columnCount(); i++) {
        QSize size = m_amodel->sizeHint(static_cast<P2AsmModel::column_e>(i));
        ui->tvAsm->setColumnWidth(i, size.width());
    }
}

/**
 * @brief Update tvDasm column sizes
 */
void MainWindow::updateDasmColumnSizes()
{
    QFont font = ui->tvDasm->font();
    QFontMetrics metrics(font);

    QHeaderView* vh = ui->tvDasm->verticalHeader();
    vh->setEnabled(true);
    vh->setDefaultSectionSize(metrics.ascent() + metrics.descent());

    QHeaderView* hh = ui->tvDasm->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    hh->setStretchLastSection(true);
    connect(hh, SIGNAL(customContextMenuRequested(QPoint)), SLOT(dasmHeaderColums(QPoint)), Qt::UniqueConnection);

    for (int i = 0; i < m_dmodel->columnCount(); i++) {
        QSize size = m_dmodel->sizeHint(static_cast<P2DasmModel::column_e>(i));
        ui->tvDasm->setColumnWidth(i, size.width());
    }
}

void MainWindow::updateSymbolsColumnSizes()
{
    QFont font = ui->tvSymbols->font();
    QFontMetrics metrics(font);

    QHeaderView* vh = ui->tvSymbols->verticalHeader();
    vh->setEnabled(true);
    vh->setDefaultSectionSize(metrics.ascent() + metrics.descent());

    QHeaderView* hh = ui->tvSymbols->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    hh->setStretchLastSection(true);
    connect(hh, SIGNAL(customContextMenuRequested(QPoint)), SLOT(symbolsHeaderColums(QPoint)), Qt::UniqueConnection);

    for (int i = 0; i < m_smodel->columnCount(); i++) {
        QSize size = m_smodel->sizeHint(static_cast<P2SymbolsModel::column_e>(i));
        ui->tvSymbols->setColumnWidth(i, size.width());
    }
}

void MainWindow::setupCogView()
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

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
#include <QUrl>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimer>
#include <QSettings>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "gotoaddress.h"
#include "p2hub.h"
#include "p2cog.h"
#include "p2asm.h"
#include "p2dasm.h"
#include "p2asmmodel.h"
#include "p2dasmmodel.h"

static const int ncogs = 8;
static const QLatin1String key_windowGeometry("windowGeometry");
static const QLatin1String key_windowState("windowState");
static const QLatin1String grp_assembler("assembler");
static const QLatin1String grp_disassembler("disassembler");
static const QLatin1String key_opcodes("opcodes");
static const QLatin1String key_lowercase("lowercase");
static const QLatin1String key_current_row("current_row");
static const QLatin1String key_column_address("hide_address");
static const QLatin1String key_column_origin("hide_origin");
static const QLatin1String key_column_tokens("hide_tokens");
static const QLatin1String key_column_opcode("hide_opcode");
static const QLatin1String key_column_errors("hide_errors");
static const QLatin1String key_column_symbols("hide_symbols");
static const QLatin1String key_column_instruction("hide_instruction");
static const QLatin1String key_column_source("hide_source");
static const QLatin1String key_column_description("hide_description");

static const QLatin1String key_source("source");
static const QLatin1String key_directory("directory");
static const QLatin1String key_filename("filename");
static const QLatin1String key_history("directory");

static const QLatin1String tab_hub("P2Hub");
static const QLatin1String tab_asm("P2Asm");
static const QLatin1String tab_dasm("P2Dasm");

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

    restoreSettings();
    setupCogView();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::saveSettings()
{
    QSettings s;
    s.setValue(key_windowGeometry, saveGeometry());
    s.setValue(key_windowState, saveState());

    s.beginGroup(grp_assembler);
    s.setValue(key_opcodes, m_amodel->opcode_format());
    s.setValue(key_column_origin, ui->tvAsm->isColumnHidden(P2AsmModel::c_Origin));
    s.setValue(key_column_opcode, ui->tvAsm->isColumnHidden(P2AsmModel::c_Opcode));
    s.setValue(key_column_tokens, ui->tvAsm->isColumnHidden(P2AsmModel::c_Tokens));
    s.setValue(key_column_symbols, ui->tvAsm->isColumnHidden(P2AsmModel::c_Symbols));
    s.setValue(key_column_errors, ui->tvAsm->isColumnHidden(P2AsmModel::c_Errors));
    s.setValue(key_column_source, ui->tvAsm->isColumnHidden(P2AsmModel::c_Source));
    s.endGroup();

    s.beginGroup(grp_disassembler);
    s.setValue(key_opcodes, m_dmodel->opcode_format());
    s.setValue(key_lowercase, ui->action_Dasm_Lowercase->isChecked());
    s.setValue(key_column_address, ui->tvDasm->isColumnHidden(P2DasmModel::c_Address));
    s.setValue(key_column_opcode, ui->tvDasm->isColumnHidden(P2DasmModel::c_Opcode));
    s.setValue(key_column_instruction, ui->tvDasm->isColumnHidden(P2DasmModel::c_Instruction));
    s.setValue(key_column_description, ui->tvDasm->isColumnHidden(P2DasmModel::c_Description));
    s.setValue(key_current_row, ui->tvDasm->currentIndex().row());
    s.endGroup();

}

void MainWindow::restoreSettings()
{
    QSettings s;
    restoreGeometry(s.value(key_windowGeometry).toByteArray());
    restoreState(s.value(key_windowState).toByteArray());

    s.beginGroup(grp_assembler);
    setAsmOpcodes(s.value(key_opcodes, fmt_bin).toInt());
    ui->tvDasm->selectRow(s.value(key_current_row).toInt());
    ui->tvDasm->setColumnHidden(P2AsmModel::c_Origin, s.value(key_column_origin, false).toBool());
    ui->tvDasm->setColumnHidden(P2AsmModel::c_Opcode, s.value(key_column_opcode, false).toBool());
    ui->tvDasm->setColumnHidden(P2AsmModel::c_Tokens, s.value(key_column_tokens, false).toBool());
    ui->tvDasm->setColumnHidden(P2AsmModel::c_Symbols, s.value(key_column_symbols, false).toBool());
    ui->tvDasm->setColumnHidden(P2AsmModel::c_Errors, s.value(key_column_errors, false).toBool());
    ui->tvDasm->setColumnHidden(P2AsmModel::c_Source, s.value(key_column_source, false).toBool());
    s.endGroup();

    s.beginGroup(grp_disassembler);
    setDasmOpcodes(s.value(key_opcodes, fmt_bin).toInt());
    setDasmLowercase(s.value(key_lowercase).toBool());
    ui->tvDasm->selectRow(s.value(key_current_row).toInt());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Address, s.value(key_column_address, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Opcode, s.value(key_column_opcode, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Instruction, s.value(key_column_instruction, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Description, s.value(key_column_description, false).toBool());
    s.endGroup();
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

void MainWindow::setActionsEnabled(ui_tab_e which, bool enable)
{
    switch (which) {
    case id_hub: // Hub
        ui->action_Go_to_address->setEnabled(enable);
        ui->action_Go_to_COG->setEnabled(enable);
        ui->action_Go_to_LUT->setEnabled(enable);
        ui->action_Go_to_ROM->setEnabled(enable);
        ui->action_SingleStep->setEnabled(enable);
        break;
    case id_asm: // Asm
        ui->action_Assemble->setEnabled(enable);
        ui->action_Asm_Opcodes_bin->setEnabled(enable);
        ui->action_Asm_Opcodes_byt->setEnabled(enable);
        ui->action_Asm_Opcodes_oct->setEnabled(enable);
        ui->action_Asm_Opcodes_hex->setEnabled(enable);
        break;
    case id_dasm: // Dasm
        ui->action_Dasm_Opcodes_bin->setEnabled(enable);
        ui->action_Dasm_Opcodes_byt->setEnabled(enable);
        ui->action_Dasm_Opcodes_oct->setEnabled(enable);
        ui->action_Dasm_Opcodes_hex->setEnabled(enable);
        ui->action_Dasm_Lowercase->setEnabled(enable);
        break;
    }
}

void MainWindow::tabChanged(int idx)
{
    QTabBar* bar = ui->tabWidget->tabBar();
    ui_tab_e tab = static_cast<ui_tab_e>(bar->tabData(idx).toInt());
    setActionsEnabled(id_hub, tab == id_hub);
    setActionsEnabled(id_asm, tab == id_asm);
    setActionsEnabled(id_dasm, tab == id_dasm);
}

void MainWindow::gotoHex(const QString& address)
{
    bool ok;
    p2_LONG addr = address.toUInt(&ok, 16);
    if (ok)
        ui->tvDasm->selectRow(static_cast<int>(addr / 4));
}

void MainWindow::gotoCog()
{
    gotoHex("00000");
}

void MainWindow::gotoLut()
{
    gotoHex("00800");
}

void MainWindow::gotoRom()
{
    gotoHex("fc000");
}

void MainWindow::gotoAddress()
{
    GotoAddress dlg;
    if (QDialog::Accepted == dlg.exec()) {
        gotoHex(dlg.address());
    }
}

void MainWindow::setAsmOpcodes(int mode)
{
    p2_opcode_format_e format = static_cast<p2_opcode_format_e>(mode);
    switch (format) {
    case fmt_bin:
        ui->action_Asm_Opcodes_bin->setChecked(true);
        ui->action_Asm_Opcodes_byt->setChecked(false);
        ui->action_Asm_Opcodes_oct->setChecked(false);
        ui->action_Asm_Opcodes_hex->setChecked(false);
        break;
    case fmt_byt:
        ui->action_Asm_Opcodes_bin->setChecked(false);
        ui->action_Asm_Opcodes_byt->setChecked(true);
        ui->action_Asm_Opcodes_oct->setChecked(false);
        ui->action_Asm_Opcodes_hex->setChecked(false);
        break;
    case fmt_oct:
        ui->action_Asm_Opcodes_bin->setChecked(false);
        ui->action_Asm_Opcodes_byt->setChecked(false);
        ui->action_Asm_Opcodes_oct->setChecked(true);
        ui->action_Asm_Opcodes_hex->setChecked(false);
        break;
    case fmt_hex:
        ui->action_Asm_Opcodes_bin->setChecked(false);
        ui->action_Asm_Opcodes_byt->setChecked(false);
        ui->action_Asm_Opcodes_oct->setChecked(false);
        ui->action_Asm_Opcodes_hex->setChecked(true);
        break;
    }
    m_amodel->setOpcodeFormat(format);
    updateAsmColumnSizes();
}

void MainWindow::setAsmOpcodesBin()
{
    setAsmOpcodes(fmt_bin);
}

void MainWindow::setAsmOpcodesByt()
{
    setAsmOpcodes(fmt_byt);
}

void MainWindow::setAsmOpcodesHex()
{
    setAsmOpcodes(fmt_hex);
}

void MainWindow::setAsmOpcodesOct()
{
    setAsmOpcodes(fmt_oct);
}

void MainWindow::setDasmOpcodes(int mode)
{
    p2_opcode_format_e format = static_cast<p2_opcode_format_e>(mode);
    switch (format) {
    case fmt_bin:
        ui->action_Dasm_Opcodes_bin->setChecked(true);
        ui->action_Dasm_Opcodes_byt->setChecked(false);
        ui->action_Dasm_Opcodes_oct->setChecked(false);
        ui->action_Dasm_Opcodes_hex->setChecked(false);
        break;
    case fmt_byt:
        ui->action_Dasm_Opcodes_bin->setChecked(false);
        ui->action_Dasm_Opcodes_byt->setChecked(true);
        ui->action_Dasm_Opcodes_oct->setChecked(false);
        ui->action_Dasm_Opcodes_hex->setChecked(false);
        break;
    case fmt_oct:
        ui->action_Dasm_Opcodes_bin->setChecked(false);
        ui->action_Dasm_Opcodes_byt->setChecked(false);
        ui->action_Dasm_Opcodes_oct->setChecked(true);
        ui->action_Dasm_Opcodes_hex->setChecked(false);
        break;
    case fmt_hex:
        ui->action_Dasm_Opcodes_bin->setChecked(false);
        ui->action_Dasm_Opcodes_byt->setChecked(false);
        ui->action_Dasm_Opcodes_oct->setChecked(false);
        ui->action_Dasm_Opcodes_hex->setChecked(true);
        break;
    }
    m_dmodel->setOpcodeFormat(format);
    updateDasmColumnSizes();
}

void MainWindow::setDasmOpcodesBin()
{
    setDasmOpcodes(fmt_bin);
}

void MainWindow::setDasmOpcodesByt()
{
    setDasmOpcodes(fmt_oct);
}

void MainWindow::setDasmOpcodesOct()
{
    setDasmOpcodes(fmt_oct);
}

void MainWindow::setDasmOpcodesHex()
{
    setDasmOpcodes(fmt_hex);
}

void MainWindow::asmHeaderColums(const QPoint& pos)
{
    QList<P2AsmModel::column_e> columns;
    columns += P2AsmModel::c_Origin;
    columns += P2AsmModel::c_Opcode;
    columns += P2AsmModel::c_Tokens;
    columns += P2AsmModel::c_Symbols;
    columns += P2AsmModel::c_Errors;
    columns += P2AsmModel::c_Source;

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
    QList<P2DasmModel::column_e> columns;
    columns += P2DasmModel::c_Address;
    columns += P2DasmModel::c_Opcode;
    columns += P2DasmModel::c_Instruction;
    columns += P2DasmModel::c_Description;

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
    qint64 t0 = QDateTime::currentMSecsSinceEpoch();
    if (m_asm->assemble(source)) {
        qint64 t1 = QDateTime::currentMSecsSinceEpoch();
        QLabel* status = ui->statusBar->findChild<QLabel*>(key_status);
        if (status)
            status->setText(tr("Assembly took %1 ms.").arg(t1 - t0));
        // Inspect result
        const int row = ui->tvAsm->currentIndex().row();
        m_amodel->invalidate();
        ui->tvAsm->selectRow(row);
    }
}

void MainWindow::setDasmLowercase(bool check)
{
    ui->action_Dasm_Lowercase->setChecked(check);
    m_dasm->setLowercase(check);
    p2_LONG PC = m_hub->cog(0)->rd_PC();
    int row = static_cast<int>((PC < PC_LONGS) ? PC : PC / 4);
    m_dmodel->invalidate();
    ui->tvDasm->selectRow(row);
}

void MainWindow::setupAssembler()
{
    ui->tvAsm->setModel(m_amodel);
    updateAsmColumnSizes();
    // QString sourcecode = QStringLiteral(":/ROM_Booter_v33_01j.spin2");
    QString sourcecode = QStringLiteral(":/P2-qz80-rr032.spin2");
    openSource(sourcecode);
}

void MainWindow::setupDisassembler()
{
    ui->tvDasm->setModel(m_dmodel);
    updateDasmColumnSizes();
}

void MainWindow::setupMenu()
{
    connect(ui->action_Open, SIGNAL(triggered()), SLOT(openSource()));
}

void MainWindow::setupTabWidget()
{
    QTabBar* bar = ui->tabWidget->tabBar();
    // FIXME: how to identify tabs if the order changes?
    bar->setTabData(0, id_hub);
    bar->setTabData(1, id_asm);
    bar->setTabData(2, id_dasm);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
}

void MainWindow::setupToolbars()
{
    connect(ui->action_SingleStep, SIGNAL(triggered()), SLOT(hubSingleStep()));
    ui->toolbarHub->addAction(ui->action_SingleStep);

    connect(ui->action_Asm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(setAsmOpcodesBin()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_bin);

    connect(ui->action_Asm_Opcodes_byt, SIGNAL(triggered(bool)), SLOT(setAsmOpcodesByt()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_byt);

    connect(ui->action_Asm_Opcodes_oct, SIGNAL(triggered(bool)), SLOT(setAsmOpcodesOct()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_oct);

    connect(ui->action_Asm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(setAsmOpcodesHex()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_hex);

    ui->toolbarAsm->addSeparator();

    connect(ui->action_Assemble, SIGNAL(triggered()), SLOT(assemble()));
    ui->toolbarAsm->addAction(ui->action_Assemble);

    connect(ui->action_Go_to_COG, SIGNAL(triggered()), SLOT(gotoCog()));
    ui->toolbarDasm->addAction(ui->action_Go_to_COG);

    connect(ui->action_Go_to_LUT, SIGNAL(triggered()), SLOT(gotoLut()));
    ui->toolbarDasm->addAction(ui->action_Go_to_LUT);

    connect(ui->action_Go_to_ROM, SIGNAL(triggered()), SLOT(gotoRom()));
    ui->toolbarDasm->addAction(ui->action_Go_to_ROM);

    connect(ui->action_Go_to_address, SIGNAL(triggered()), SLOT(gotoAddress()));
    ui->toolbarDasm->addAction(ui->action_Go_to_address);

    ui->toolbarDasm->addSeparator();

    connect(ui->action_Dasm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(setDasmOpcodesBin()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_bin);

    connect(ui->action_Dasm_Opcodes_byt, SIGNAL(triggered(bool)), SLOT(setDasmOpcodesByt()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_byt);

    connect(ui->action_Dasm_Opcodes_oct, SIGNAL(triggered(bool)), SLOT(setDasmOpcodesOct()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_oct);

    connect(ui->action_Dasm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(setDasmOpcodesHex()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_hex);

    ui->toolbarDasm->addSeparator();

    connect(ui->action_Dasm_Lowercase, SIGNAL(triggered(bool)), SLOT(setDasmLowercase(bool)));
    ui->toolbarDasm->addAction(ui->action_Dasm_Lowercase);

    tabChanged(0);
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
    connect(hh, SIGNAL(customContextMenuRequested(QPoint)), SLOT(asmHeaderColums(QPoint)), Qt::UniqueConnection);
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    hh->setStretchLastSection(true);
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
    QHeaderView* hh = ui->tvDasm->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh, SIGNAL(customContextMenuRequested(QPoint)), SLOT(dasmHeaderColums(QPoint)), Qt::UniqueConnection);
    for (int i = 0; i < m_dmodel->columnCount(); i++) {
        QSize size = m_dmodel->sizeHint(static_cast<P2DasmModel::column_e>(i));
        ui->tvDasm->setColumnWidth(i, size.width());
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

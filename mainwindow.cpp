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
#include <QFile>
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
static const QLatin1String key_column_instruction("hide_instruction");
static const QLatin1String key_column_source("hide_source");
static const QLatin1String key_column_description("hide_description");

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
    s.setValue(key_column_tokens, ui->tvDasm->isColumnHidden(P2AsmModel::c_Tokens));
    s.setValue(key_column_opcode, ui->tvDasm->isColumnHidden(P2AsmModel::c_Opcode));
    s.setValue(key_column_errors, ui->tvDasm->isColumnHidden(P2AsmModel::c_Errors));
    s.setValue(key_column_source, ui->tvDasm->isColumnHidden(P2AsmModel::c_Source));
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
    setDasmOpcodes(s.value(key_opcodes, P2DasmModel::f_bin).toInt());
    setDasmLowercase(s.value(key_lowercase).toBool());
    ui->tvDasm->selectRow(s.value(key_current_row).toInt());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Address, s.value(key_column_address, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Opcode, s.value(key_column_opcode, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Instruction, s.value(key_column_instruction, false).toBool());
    ui->tvDasm->setColumnHidden(P2DasmModel::c_Description, s.value(key_column_description, false).toBool());
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

void MainWindow::tabChanged(int idx)
{
    QTabBar* bar = ui->tabWidget->tabBar();
    QString tab = bar->tabData(idx).toString();
    if (tab == tab_hub) {
        ui->toolbarHub->show();
        ui->toolbarAsm->hide();
        ui->toolbarDasm->hide();
    }
    if (tab == tab_asm) {
        ui->toolbarHub->hide();
        ui->toolbarAsm->show();
        ui->toolbarDasm->hide();
    }
    if (tab == tab_dasm) {
        ui->toolbarHub->hide();
        ui->toolbarAsm->hide();
        ui->toolbarDasm->show();
    }
}

void MainWindow::gotoHex(const QString& address)
{
    bool ok;
    P2LONG addr = address.toUInt(&ok, 16);
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
    P2AsmModel::format_e format = static_cast<P2AsmModel::format_e>(mode);
    switch (format) {
    case P2AsmModel::f_bin:
        ui->action_Asm_Opcodes_bin->setChecked(true);
        ui->action_Asm_Opcodes_hex->setChecked(false);
        ui->action_Asm_Opcodes_oct->setChecked(false);
        break;
    case P2AsmModel::f_hex:
        ui->action_Asm_Opcodes_bin->setChecked(false);
        ui->action_Asm_Opcodes_hex->setChecked(true);
        ui->action_Asm_Opcodes_oct->setChecked(false);
        break;
    case P2AsmModel::f_oct:
        ui->action_Asm_Opcodes_bin->setChecked(false);
        ui->action_Asm_Opcodes_hex->setChecked(false);
        ui->action_Asm_Opcodes_oct->setChecked(true);
        break;
    }
    m_amodel->setOpcodeFormat(format);
    updateAsmColumnSizes();
}

void MainWindow::setAsmOpcodesBinary()
{
    setAsmOpcodes(P2AsmModel::f_bin);
}

void MainWindow::setAsmOpcodesHexDec()
{
    setAsmOpcodes(P2AsmModel::f_hex);
}

void MainWindow::setAsmOpcodesOctal()
{
    setAsmOpcodes(P2AsmModel::f_oct);
}

void MainWindow::setDasmOpcodes(int mode)
{
    P2DasmModel::format_e format = static_cast<P2DasmModel::format_e>(mode);
    switch (format) {
    case P2DasmModel::f_bin:
        ui->action_Dasm_Opcodes_bin->setChecked(true);
        ui->action_Dasm_Opcodes_hex->setChecked(false);
        ui->action_Dasm_Opcodes_oct->setChecked(false);
        break;
    case P2DasmModel::f_hex:
        ui->action_Dasm_Opcodes_bin->setChecked(false);
        ui->action_Dasm_Opcodes_hex->setChecked(true);
        ui->action_Dasm_Opcodes_oct->setChecked(false);
        break;
    case P2DasmModel::f_oct:
        ui->action_Dasm_Opcodes_bin->setChecked(false);
        ui->action_Dasm_Opcodes_hex->setChecked(false);
        ui->action_Dasm_Opcodes_oct->setChecked(true);
        break;
    }
    m_dmodel->setOpcodeFormat(format);
    updateDasmColumnSizes();
}

void MainWindow::setDasmOpcodesBinary()
{
    setDasmOpcodes(P2DasmModel::f_bin);
}

void MainWindow::setDasmOpcodesHexDec()
{
    setDasmOpcodes(P2DasmModel::f_hex);
}

void MainWindow::setDasmOpcodesOctal()
{
    setDasmOpcodes(P2DasmModel::f_oct);
}

void MainWindow::asmHeaderColums(const QPoint& pos)
{
    QList<P2AsmModel::column_e> columns;
    columns += P2AsmModel::c_Origin;
    columns += P2AsmModel::c_Tokens;
    columns += P2AsmModel::c_Opcode;
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
        m_amodel->invalidate();
    }
}

void MainWindow::setDasmLowercase(bool check)
{
    ui->action_Dasm_Lowercase->setChecked(check);
    m_dasm->setLowercase(check);
    P2LONG PC = m_hub->cog(0)->rd_PC();
    int row = static_cast<int>((PC < PC_LONGS) ? PC : PC / 4);
    m_dmodel->invalidate();
    ui->tvDasm->selectRow(row);
}

void MainWindow::setupAssembler()
{
    QFile file(QStringLiteral(":/ROM_Booter_v33_01j.spin2"));
    if (!file.open(QIODevice::ReadOnly))
        return;
    QTextStream stream(&file);
    QStringList source;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        source += line;
    }
    m_asm->setSource(source);
    ui->tvAsm->setModel(m_amodel);
    updateAsmColumnSizes();
    QHeaderView* hh = ui->tvAsm->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh, SIGNAL(customContextMenuRequested(QPoint)), SLOT(asmHeaderColums(QPoint)), Qt::UniqueConnection);
    m_amodel->invalidate();
}

void MainWindow::setupDisassembler()
{
    ui->tvDasm->setModel(m_dmodel);
    updateDasmColumnSizes();
    QHeaderView* hh = ui->tvDasm->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh, SIGNAL(customContextMenuRequested(QPoint)), SLOT(dasmHeaderColums(QPoint)), Qt::UniqueConnection);
}

void MainWindow::setupTabWidget()
{
    QTabBar* bar = ui->tabWidget->tabBar();
    bar->setTabData(0, tab_hub);
    bar->setTabData(1, tab_asm);
    bar->setTabData(2, tab_dasm);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
}

void MainWindow::setupToolbars()
{
    connect(ui->action_Go_to_COG, SIGNAL(triggered()), SLOT(gotoCog()));
    ui->toolbarDasm->addAction(ui->action_Go_to_COG);

    connect(ui->action_Go_to_LUT, SIGNAL(triggered()), SLOT(gotoLut()));
    ui->toolbarDasm->addAction(ui->action_Go_to_LUT);

    connect(ui->action_Go_to_ROM, SIGNAL(triggered()), SLOT(gotoRom()));
    ui->toolbarDasm->addAction(ui->action_Go_to_ROM);

    connect(ui->action_Go_to_address, SIGNAL(triggered()), SLOT(gotoAddress()));
    ui->toolbarDasm->addAction(ui->action_Go_to_address);

    ui->toolbarDasm->addSeparator();

    connect(ui->action_Dasm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(setDasmOpcodesBinary()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_bin);

    connect(ui->action_Dasm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(setDasmOpcodesHexDec()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_hex);

    connect(ui->action_Dasm_Opcodes_oct, SIGNAL(triggered(bool)), SLOT(setDasmOpcodesOctal()));
    ui->toolbarDasm->addAction(ui->action_Dasm_Opcodes_oct);

    ui->toolbarDasm->addSeparator();

    connect(ui->action_Dasm_Lowercase, SIGNAL(triggered(bool)), SLOT(setDasmLowercase(bool)));
    ui->toolbarDasm->addAction(ui->action_Dasm_Lowercase);



    connect(ui->action_SingleStep, SIGNAL(triggered()), SLOT(hubSingleStep()));
    ui->toolbarHub->addAction(ui->action_SingleStep);

    connect(ui->action_Asm_Opcodes_bin, SIGNAL(triggered(bool)), SLOT(setAsmOpcodesBinary()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_bin);

    connect(ui->action_Asm_Opcodes_hex, SIGNAL(triggered(bool)), SLOT(setAsmOpcodesHexDec()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_hex);

    connect(ui->action_Asm_Opcodes_oct, SIGNAL(triggered(bool)), SLOT(setAsmOpcodesOctal()));
    ui->toolbarAsm->addAction(ui->action_Asm_Opcodes_oct);

    ui->toolbarAsm->addSeparator();

    connect(ui->action_Assemble, SIGNAL(triggered()), SLOT(assemble()));
    ui->toolbarAsm->addAction(ui->action_Assemble);

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

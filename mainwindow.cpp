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
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QSettings>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "gotoaddress.h"
#include "p2dasm.h"

static const int ncogs = 4;
static const QLatin1String key_windowGeometry("windowGeometry");
static const QLatin1String key_lowercase("lowercase");
static const QLatin1String key_current_row("current_row");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_vcog()
    , m_hub(ncogs, this)
    , m_dasm(new P2Dasm(m_hub.cog(0)))
    , m_model(new P2DasmModel(m_dasm))
{
    ui->setupUi(this);
    setWindowTitle(QString("%1 v%2").arg(qApp->applicationName()).arg(qApp->applicationVersion()));
    setupToolbar();

    connect(ui->action_Quit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->action_About, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->action_AboutQt5, SIGNAL(triggered(bool)), this, SLOT(aboutQt5()));

    m_hub.load(":/ROM_Booter_v33_01j.bin");

    ui->tvDasm->setModel(m_model);
    // Set column sizes
    for (int column = 0; column < m_model->columnCount(); column++) {
        QSize size = m_model->sizeHint(static_cast<P2DasmModel::column_e>(column));
        ui->tvDasm->setColumnWidth(column, size.width());
    }

    QSettings s;
    restoreGeometry(s.value(key_windowGeometry).toByteArray());
    setLowercase(s.value(key_lowercase).toBool());
    ui->tvDasm->selectRow(s.value(key_current_row).toInt());

    setupCogView();
}

MainWindow::~MainWindow()
{
    QSettings s;
    s.setValue(key_windowGeometry, saveGeometry());
    s.setValue(key_lowercase, ui->action_setLowercase->isChecked());
    s.setValue(key_current_row, ui->tvDasm->currentIndex().row());
    delete ui;
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

void MainWindow::gotoAddress(const QString& address)
{
    bool ok;
    p2_LONG addr = address.toUInt(&ok, 16);
    if (ok)
        ui->tvDasm->selectRow(static_cast<int>(addr / 4));
}

void MainWindow::gotoInputAddress()
{
    GotoAddress dlg;
    if (QDialog::Accepted == dlg.exec()) {
        gotoAddress(dlg.address());
    }
}

void MainWindow::gotoCog()
{
    gotoAddress("00000");
}

void MainWindow::gotoLut()
{
    gotoAddress("00800");
}

void MainWindow::gotoFC000()
{
    gotoAddress("fc000");
}

void MainWindow::setLowercase(bool check)
{
    ui->action_setLowercase->setChecked(check);
    m_dasm->setLowercase(check);
    int row = ui->tvDasm->currentIndex().row();
    m_model->invalidate();
    ui->tvDasm->selectRow(row);
}

void MainWindow::setupToolbar()
{
    connect(ui->action_Go_to_address, SIGNAL(triggered()), SLOT(gotoInputAddress()));
    ui->mainToolBar->addAction(ui->action_Go_to_address);

    connect(ui->action_Go_to_COG, SIGNAL(triggered()), SLOT(gotoCog()));
    ui->mainToolBar->addAction(ui->action_Go_to_COG);

    connect(ui->action_Go_to_LUT, SIGNAL(triggered()), SLOT(gotoLut()));
    ui->mainToolBar->addAction(ui->action_Go_to_LUT);

    connect(ui->action_Go_to_FC000, SIGNAL(triggered()), SLOT(gotoFC000()));
    ui->mainToolBar->addAction(ui->action_Go_to_FC000);

    connect(ui->action_setLowercase, SIGNAL(triggered(bool)), SLOT(setLowercase(bool)));
    ui->mainToolBar->addAction(ui->action_setLowercase);
}

void MainWindow::setupCogView()
{
    m_vcog = QVector<P2CogView*>()
             << ui->cog0 << ui->cog1 << ui->cog2 << ui->cog3
             << ui->cog4 << ui->cog5 << ui->cog6 << ui->cog7
             << ui->cog8 << ui->cog9 << ui->cogA << ui->cogB
             << ui->cogC << ui->cogD << ui->cogE << ui->cogF;
    for (int id = 0; id < 16; id++) {
        P2CogView* vcog = m_vcog[id];
        if (!vcog)
            continue;
        if (id >= ncogs) {
            vcog->hide();
            continue;
        }
        vcog->setCog(m_hub.cog(id));
    }
}

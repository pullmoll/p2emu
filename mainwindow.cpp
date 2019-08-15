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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "csv.h"
#include "p2cog.h"
#include "p2dasm.h"

#if CREATE_SOURCE
//! field #0 in the CSV data
static const QString k_id("id");
//! field #1 in the CSV data
static const QString k_alias("alias");
//! field #2 in the CSV data
static const QString k_group("group");
//! field #3 in the CSV data
static const QString k_encoding("encoding");
//! field #4 in the CSV data
static const QString k_syntax("syntax");
//! field #5 in the CSV data
static const QString k_description("description");
//! field #6 in the CSV data
static const QString k_timing_08_1("timing.8.1");
//! field #7 in the CSV data
static const QString k_timing_08_2("timing.8.2");
//! field #8 in the CSV data
static const QString k_timing_16_1("timing.16.1");
//! field #9 in the CSV data
static const QString k_timing_16_2("timing.16.2");
//! field #10 in the CSV data
static const QString k_regwrite("register_write");
//! field #11 in the CSV data
static const QString k_hub_r_w("hub_r_w");
//! field #12 in the CSV data
static const QString k_stack_r_w("stack_r_w");
//! field #13 in the CSV data
static const QString k_spin_methods("spin_methods");

//! list of keys ordered as they appear in the CSV
static const QStringList k_keys = QStringList()
    << k_id
    << k_alias
    << k_group
    << k_encoding
    << k_syntax
    << k_description
    << k_timing_08_1
    << k_timing_08_2
    << k_timing_16_1
    << k_timing_16_2
    << k_regwrite
    << k_hub_r_w
    << k_stack_r_w
    << k_spin_methods;
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QEventLoop loop(this);
    ui->setupUi(this);

#if CREATE_SOURCE
    csv2source();
#endif

    P2Hub hub;
    P2Dasm da;

    QFile bin(":/TACHYON5r4.binary");
    if (bin.open(QIODevice::ReadOnly)) {
        quint32 addr = 0x000;
        bin.seek(0x200);
        while (!bin.atEnd()) {
            union {
                char bytes[4];
                quint32 word;
            }   data;
            if (4 == bin.read(data.bytes, 4)) {
                hub.wr_COG(0, addr++, data.word);
            }
        }
    }

    P2Cog* cog = hub.cog(0);
    for (quint32 addr = 0; addr < 32768/4; addr++) {
        QString opcode;
        QString string = da.dasm(cog, addr, opcode);
        opcode.resize(40, QChar::Space);
        qDebug("%06x: %s %s", addr, qPrintable(opcode), qPrintable(string));
        loop.processEvents();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

#if CREATE_SOURCE
QString MainWindow::opfunc(const QVariantMap& map, DecoderMap& decoder)
{
    QString syntax = map.value(k_syntax).toString();
    QString encoding = map.value(k_encoding).toString();
    QString op = syntax.split(QChar::Space).first().toLower();
    QString description = map.value(k_description).toString();
    QStringList desclist = description.split(QChar('.'));
    QString brief = desclist.first();
    QString src;
    QTextStream str(&src, QIODevice::WriteOnly);
    desclist = desclist.mid(1);

    QString mask, match;
    foreach(QChar ch, encoding) {
        switch (ch.toLatin1()) {
        case ' ':
            mask += QChar::Space;
            match += QChar::Space;
            break;
        case '0': case '1':
            mask += QChar('1');
            match += ch;
            break;
        default:
            mask += QChar('0');
            match += QChar('0');
        }
    }

    str << QStringLiteral("/**\n");
    str << QStringLiteral(" * @brief ") << brief << QStringLiteral(".\n");
    str << QStringLiteral(" *\n");
    str << QStringLiteral(" * ") << encoding << QStringLiteral("\n");
    str << QStringLiteral(" *\n");
    str << QStringLiteral(" * ") << syntax << QStringLiteral("\n");
    str << QStringLiteral(" *\n");
    foreach(const QString& s, desclist) {
        if (s.trimmed() == QStringLiteral("*"))
            str << QStringLiteral(" * Z = (result == 0).\n");
        else if (!s.trimmed().isEmpty())
            str << QStringLiteral(" * ") << s.trimmed() << QStringLiteral(".\n");
    }
    str << QStringLiteral(" */\n");
    str << QStringLiteral("void propeller2::op_") << op << QStringLiteral("()\n");
    str << QStringLiteral("{\n");
    str << QStringLiteral("}\n");
    str << QStringLiteral("\n");

    decoder.insertMulti(QString("%1_%2").arg(mask).arg(match), Decoder(mask, match, op));

    return src;
}

void MainWindow::csv2source()
{
    CSV csv(":/P2 instruction set.csv");
    QList<QStringList> records;
    DecoderMap dec;
    QFile src("propeller2.txt");
    if (!src.open(QIODevice::WriteOnly))
        return;
    if (csv.read(records)) {
        for (int i = 0; i < records.count(); i++) {
            QStringList fields = records.at(i);

            QVariantMap map;
            for (int j = 0; j < k_keys.count(); j++) {
                map.insert(k_keys[j], fields.value(j));
                // qDebug("%-20s: %s", qPrintable(k_keys[j]), qPrintable(map.value(k_keys[j]).toString()));
            }
            if (i > 0) {
                QString s = opfunc(map, dec);
                qDebug("%s", qPrintable(s));
                src.write(s.toUtf8());
            }
        }
    }
    foreach(const Decoder& d, dec.values())
        qDebug("%08x %08x %s", d.mask(), d.match(), qPrintable(d.op()));
}
#endif

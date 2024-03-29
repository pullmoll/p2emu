/****************************************************************************
 *
 * Propeller2 emulator main
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
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include "p2token.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName(QStringLiteral("Propeller2-Emulator"));
    a.setApplicationVersion(QString("%1.%2.%3").arg(VER_MAJ).arg(VER_MIN).arg(VER_PAT));
    a.setOrganizationName(QStringLiteral("PullMoll"));
    a.setOrganizationDomain(QStringLiteral("https://propeller2.voidlinux.de/"));

#if 0
    QTranslator translator;
     // look up e.g. :/translations/p2emu.de.qm
    const QString filename = QLatin1String("p2emu");
    const QString prefix = QLatin1String(".");
    const QString directory = QLatin1String(":/translations");
    const QString suffix = QLatin1String(".qm");
    bool ok = translator.load(QLocale(), filename, prefix, directory, suffix);
    if (ok)
        a.installTranslator(&translator);
#endif
    MainWindow w;
    w.show();

    return a.exec();
}

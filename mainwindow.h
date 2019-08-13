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

#define CREATE_SOURCE   0   //!< define to 1 to create the inital source for propeller2.cpp

namespace Ui {
class MainWindow;
}

#if CREATE_SOURCE
class Decoder
{
public:
    Decoder(const QString& mask, const QString& match, const QString& op)
        : m_mask(fromBin(mask))
        , m_match(fromBin(match))
        , m_op(op)
    {}
    quint32 mask() const { return m_mask; }
    quint32 match() const { return m_match; }
    QString op() const { return m_op; }

private:
    static quint32 fromBin(const QString& s)
    {
        quint32 res = 0;
        foreach(QChar ch, s) {
            if (ch.toLatin1() == '0')
                res = res << 1;
            if (ch.toLatin1() == '1')
                res = res << 1 | 1u;
        }
        return res;
    }

    quint32 m_mask;
    quint32 m_match;
    QString m_op;
};

typedef QMultiMap<QString,Decoder> DecoderMap;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

#if CREATE_SOURCE
    void csv2source();
    QString opfunc(const QVariantMap& map, DecoderMap& decoder);
#endif
};

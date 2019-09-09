/****************************************************************************
 *
 * Propeller2 assembler data model for QTableView
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
#include <QObject>
#include <QAbstractTableModel>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QIcon>
#include "p2asm.h"

class P2AsmModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum column_e {
        c_Origin,
        c_Address,
        c_Opcode,
        c_Tokens,
        c_Symbols,
        c_Errors,
        c_Source,
    };
    Q_ENUM(column_e)

    explicit P2AsmModel(P2Asm* p2asm, QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    static QList<column_e> columns();
    p2_format_e opcode_format() const;
    QSize sizeHint(const QModelIndex &parent = QModelIndex(), const QString& text = QString()) const;

public slots:
    void invalidate();
    void setOpcodeFormat(p2_format_e format);
    void setFont(const QFont& font);
    void set_highlight(const QModelIndex& index, const P2Symbol& symbol, const P2Word& word);

private:
    P2Asm* m_asm;
    p2_format_e m_format;
    QFont m_font;
    QPixmap m_error_pixmap;
    QIcon m_error;
    QModelIndex m_highlite_index;
    P2Symbol m_highlight_symbol;
    P2Word m_highlight_word;
    QHash<column_e,QString> m_header;
    QHash<column_e,QRgb> m_background;
    QHash<column_e,int> m_head_alignment;
    QHash<column_e,int> m_text_alignment;
    QString opcodeToolTip(const P2Opcode& IR) const;
    QString tokenToolTip(const P2Words& words) const;
    QString symbolsToolTip(const P2SymbolTable& symbols, const QList<P2Symbol>& symrefs) const;
    QString errorsToolTip(const QStringList& list) const;
};

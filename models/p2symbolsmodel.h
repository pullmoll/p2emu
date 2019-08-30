/****************************************************************************
 *
 * Propeller2 symbol table data model for QAbstractTableView
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
#include <QFont>
#include <QFontMetrics>
#include "p2symboltable.h"

class P2SymbolsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum column_e {
        c_Name,
        c_Definition,
        c_References,
        c_Type,
        c_Value,
    };
    Q_ENUM(column_e)

    explicit P2SymbolsModel(const P2SymbolTable& table = P2SymbolTable(), QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    static QList<column_e> columns();
    QSize sizeHint(column_e column, bool header = false, const QString& text = QString()) const;

public slots:
    void invalidate();
    void setFont(const QFont& font);
    bool setTable(const P2SymbolTable& table);

private:
    P2SymbolTable m_table;
    QFont m_font;
    QStringList m_names;
    QHash<column_e,QString> m_header_text;
    QHash<column_e,QString> m_header_tooltip;
    QHash<column_e,QRgb> m_background;
    QHash<column_e,int> m_head_alignment;
    QHash<column_e,int> m_text_alignment;
};

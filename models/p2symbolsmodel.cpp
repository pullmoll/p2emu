/****************************************************************************
 *
 * Propeller2 symbol table data model for QTableView implementation
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
#include "p2symbolsmodel.h"

P2SymbolsModel::P2SymbolsModel(const P2SymbolTable& table, QObject *parent)
    : QAbstractTableModel(parent)
    , m_table(table)
    , m_font()
    , m_names()
    , m_header_text()
    , m_header_tooltip()
    , m_background()
    , m_head_alignment()
    , m_text_alignment()
{

    m_header_text.insert(c_Name,            tr("Name"));
    m_header_tooltip.insert(c_Name,         tr("This column shows the %1.")
                            .arg(tr("name of the symbol")));
    m_background.insert(c_Name,             qRgb(0xff,0xfc,0xf8));
    m_head_alignment.insert(c_Name,         Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Name,         Qt::AlignLeft | Qt::AlignVCenter);

    m_header_text.insert(c_Definition,      tr("Line #"));
    m_header_tooltip.insert(c_Definition,   tr("This column shows the %1.")
                            .arg(tr("source line where the symbol %1")
                                 .arg(tr("is defined"))));
    m_background.insert(c_Definition,       qRgb(0xff,0xf0,0xe0));
    m_head_alignment.insert(c_Definition,   Qt::AlignRight | Qt::AlignVCenter);
    m_text_alignment.insert(c_Definition,   Qt::AlignRight | Qt::AlignVCenter);

    m_header_text.insert(c_References,      tr("Ref(s)"));
    m_header_tooltip.insert(c_References,   tr("This column shows the %1.")
                            .arg(tr("source line where the %1")
                                 .arg(tr("is referenced"))));
    m_background.insert(c_References,       qRgb(0xf8,0xfc,0xff));
    m_head_alignment.insert(c_References,   Qt::AlignRight | Qt::AlignVCenter);
    m_text_alignment.insert(c_References,   Qt::AlignRight | Qt::AlignVCenter);

    m_header_text.insert(c_Type,            tr("Type"));
    m_header_tooltip.insert(c_Type,         tr("This column shows the %1 %2.")
                            .arg(tr("type of the "))
                            .arg(tr("symbol's value")));
    m_head_alignment.insert(c_Type,         Qt::AlignLeft | Qt::AlignVCenter);
    m_background.insert(c_Type,             qRgb(0x10,0xfc,0xff));
    m_text_alignment.insert(c_Type,         Qt::AlignCenter);

    m_header_text.insert(c_Value,           tr("Value"));
    m_header_tooltip.insert(c_Value,        tr("This column shows the %1.")
                            .arg(tr("symbol's value")));
    m_head_alignment.insert(c_Value,        Qt::AlignLeft | Qt::AlignVCenter);
    m_background.insert(c_Value,            qRgb(0xff,0xf0,0xff));
    m_text_alignment.insert(c_Value,        Qt::AlignLeft | Qt::AlignVCenter);

    if (!m_table.isNull())
        m_names = m_table->names();
}

QVariant P2SymbolsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;
    QFont font;
    font.setBold(true);
    QFontMetrics metrics(font);
    const column_e column = static_cast<column_e>(section);
    const int row = section;

    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            result = m_header_text.value(column);
            break;

        case Qt::FontRole:
            result = font;
            break;

        case Qt::SizeHintRole:
            result = sizeHint(createIndex(0,column));
            break;

        case Qt::TextAlignmentRole:
            result = m_head_alignment.value(column);
            break;

        case Qt::ToolTipRole:
            result = m_header_tooltip.value(column);
            break;
        }
        break;

    case Qt::Vertical:
        result = QString::number(row + 1);
        break;
    }
    return result;
}

int P2SymbolsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_names.count();
}

int P2SymbolsModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    const int columns = c_Value + 1;
    return columns;
}

Qt::ItemFlags P2SymbolsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::NoItemFlags;
    if (!index.isValid())
        return flags;

    column_e column = static_cast<column_e>(index.column());
    if (c_References == column)
        flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
    else
        flags |= Qt::ItemIsEnabled;

    return flags;
}

QVariant P2SymbolsModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    QFont font;

    if (!index.isValid())
        return result;

    const column_e column = static_cast<column_e>(index.column());
    const int row = index.row();
    const QString& name = m_names[row];

    const P2Symbol symbol = m_table->symbol(name);
    const bool has_symbol = !symbol.isNull();
    const P2Word& definition = has_symbol ? symbol->definition() : P2Word();
    const int lineno = definition.lineno();
    const P2Atom& atom = has_symbol ? symbol->atom() : P2Atom();
    const p2_Union_e type = atom.type();

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Name:
            result = name;
            break;

        case c_Definition:
            result = QString::number(lineno);
            break;

        case c_References:
            result = QString::number(m_table->references_in(lineno).count());
            break;

        case c_Type:
            if (has_symbol)
                result = symbol->type_name();
            break;

        case c_Value:
            if (has_symbol)
                result = atom.str(false);
            break;
        }
        break;

    case Qt::DecorationRole:
        break;

    case Qt::EditRole:
        switch (column) {
        case c_Name:
            result = QVariant::fromValue(symbol);
            break;

        case c_Definition:
            result = QVariant::fromValue(definition);
            break;

        case c_References:
            result = QVariant::fromValue(symbol);
            break;

        case c_Type:
            result = QVariant::fromValue(type);
            break;

        case c_Value:
            result = QVariant::fromValue(atom);
            break;
        }
        break;

    case Qt::ToolTipRole:
        switch (column) {
        case c_Name:
            result = definition.isValid() ? definition.str() : name;
            break;

        case c_Definition:
            // TODO: defintion tooltip
            break;

        case c_References:
            // TODO: references tooltip
            break;

        case c_Type:
            // TODO: type tooltip
            break;

        case c_Value:
            // TODO: value tooltip
            break;
        }
        break;

    case Qt::FontRole:
        result = font;
        break;

    case Qt::TextAlignmentRole:
        result = m_text_alignment.value(column);
        break;

    case Qt::BackgroundRole:
        result = m_background.value(column);
        break;

    case Qt::SizeHintRole:
        result = sizeHint(index, data(index));
        break;

    case Qt::InitialSortOrderRole:
        break;

    case Qt::UserRole:
        break;
    }
    return result;
}

static const QString template_str_name = QStringLiteral("CON::SOME_REALLY_LONG_NAME XX");
static const QString template_str_definition = QStringLiteral("9999999999 XX");
static const QString template_str_references = QStringLiteral("9999999XXXX XX");
static const QString template_str_type = QStringLiteral("Invalid XX");
static const QString template_str_value = QStringLiteral("$0123456789abcdef XX");

QSize P2SymbolsModel::sizeHint(const QModelIndex& index, QVariant var) const
{
    if (var.isNull()) {
        switch (index.column()) {
        case c_Name:
            var = template_str_name;
            break;

        case c_Definition:
            var = template_str_definition;
            break;

        case c_References:
            var = template_str_references;
            break;

        case c_Type:
            var = template_str_type;
            break;

        case c_Value:
            var = template_str_tokens;
            break;
        }
    }

    QFontMetrics metrics(m_font);
    return metrics.size(Qt::TextSingleLine, var.toString());
}

void P2SymbolsModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

void P2SymbolsModel::setFont(const QFont& font)
{
    m_font = font;
    invalidate();
}

bool P2SymbolsModel::setTable(const P2SymbolTable& table)
{
    beginResetModel();

    m_table = table;
    m_names.clear();
    if (!table.isNull())
        m_names = table->names();

    endResetModel();
    return !m_table.isNull();
}

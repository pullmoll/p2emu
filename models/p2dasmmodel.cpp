/****************************************************************************
 *
 * Propeller2 disassembler data model for QItemView
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
#include <QColor>
#include "p2dasm.h"
#include "p2dasmmodel.h"

static quint32 bin2hex(const QString& str)
{
    quint32 result = 0;
    foreach(QChar ch, str) {
        if (ch == QChar('0'))
            result = result << 1;
        if (ch == QChar('1'))
            result = result << 1 | 1;
    }
    return result;
}

P2DasmModel::P2DasmModel(P2Dasm* dasm, QObject *parent)
    : QAbstractTableModel(parent)
    , m_dasm(dasm)
    , m_format(fmt_bin)
    , m_font()
    , m_header()
    , m_background()
    , m_alignment()
{

    // Header section names
    m_header.insert(c_Address,       tr("Address"));
    m_header.insert(c_Opcode,        tr("Opcode"));
    m_header.insert(c_Instruction,   tr("Instruction"));
    m_header.insert(c_Description,   tr("Description"));

    // Horizontal section alignments
    m_alignment.insert(c_Address,       Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Opcode,        Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Instruction,   Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Description,   Qt::AlignLeft | Qt::AlignVCenter);

    // Horizontal section background colors
    m_background.insert(c_Address,      qRgb(0xff,0xfc,0xf8));
    m_background.insert(c_Opcode,       qRgb(0xf8,0xfc,0xff));
    m_background.insert(c_Instruction,  qRgb(0xff,0xff,0xff));
    m_background.insert(c_Description,  qRgb(0xf8,0xff,0xf8));
}

QVariant P2DasmModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;
    const column_e column = static_cast<column_e>(section);
    QFont font(m_font);
    font.setBold(true);
    QFontMetrics metrics(font);

    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            result = m_header.value(column);
            break;

        case Qt::FontRole:
            result = font;
            break;

        case Qt::SizeHintRole:
            result = sizeHint(column, true);
            break;

        case Qt::TextAlignmentRole:
            result = m_alignment.value(column);
            break;
        }
        break;

    case Qt::Vertical:
        result.clear();
        break;
    }
    return result;
}

int P2DasmModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    // each instruction is 1 long (4 bytes)
    const int rows = m_dasm->memsize() / 4;
    return rows;
}

int P2DasmModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    const int columns = c_Description + 1;
    return columns;
}

QVariant P2DasmModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if (!index.isValid())
        return result;

    const column_e column = static_cast<column_e>(index.column());
    const p2_LONG PC = static_cast<p2_LONG>(index.row());
    const p2_LONG addr = PC * 4;
    QString opcode;
    QString instruction;
    QString description;
    bool known;
    Q_UNUSED(known)

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Address: // Address as COG[xxx], LUT[xxx], or xxxxxx in RAM
            if (PC < 0x200) {
                result = QString("COG[%1]").arg(PC, 3, 16, QChar('0'));
            } else if (PC < 0x400) {
                result = QString("LUT[%1]").arg(PC - 0x200, 3, 16, QChar('0'));
            } else {
                result = QString("%1").arg(addr, 6, 16, QChar('0'));
            }
            break;
        case c_Opcode: // Opcode string
            {
                p2_opcode_u IR;
                known = m_dasm->dasm(PC, &opcode);
                IR.opcode = bin2hex(opcode);
                result = format_opcode(IR, m_format);
            }
            break;
        case c_Instruction: // Disassembled instruction string
            known = m_dasm->dasm(PC, nullptr, &instruction);
            result = instruction;
            break;
        case c_Description: // Comments
            known = m_dasm->dasm(PC, nullptr, nullptr, &description);
            result = description;
            break;
        }
        break;

    case Qt::DecorationRole:
        break;

    case Qt::EditRole:
        break;

    case Qt::ToolTipRole:
        break;

    case Qt::StatusTipRole:
        break;

    case Qt::WhatsThisRole:
        break;

    case Qt::FontRole:
        result = m_font;
        break;

    case Qt::TextAlignmentRole:
        result = m_alignment.value(column);
        break;

    case Qt::BackgroundRole:
        result = QColor(static_cast<QRgb>(m_background.value(column)));
        break;

    case Qt::ForegroundRole:
        break;

    case Qt::CheckStateRole:
        break;

    case Qt::AccessibleTextRole:
        break;

    case Qt::AccessibleDescriptionRole:
        break;

    case Qt::SizeHintRole:
        result = sizeHint(column);
        break;

    case Qt::InitialSortOrderRole:
        break;
    }
    return result;
}

QSize P2DasmModel::sizeHint(P2DasmModel::column_e column, bool header) const
{
    QFont font(m_font);
    font.setBold(header);
    QFontMetrics metrics(font);

    switch (column) {
    case c_Address:
        return metrics.size(Qt::TextSingleLine, template_str_address);

    case c_Opcode:
        switch (m_format) {
        case fmt_bin:
            return metrics.size(Qt::TextSingleLine, template_str_opcode_bin);
        case fmt_byt:
            return metrics.size(Qt::TextSingleLine, template_str_opcode_byt);
        case fmt_oct:
            return metrics.size(Qt::TextSingleLine, template_str_opcode_oct);
        case fmt_hex:
            return metrics.size(Qt::TextSingleLine, template_str_opcode_hex);
        }
        break;

    case c_Instruction:
        return metrics.size(Qt::TextSingleLine, template_str_instruction);

    case c_Description:
        return metrics.size(Qt::TextSingleLine, template_str_description);
    }
    return QSize();
}

void P2DasmModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

void P2DasmModel::setOpcodeFormat(p2_opcode_format_e format)
{
    if (format == m_format)
        return;
    m_format = format;
    invalidate();
}

void P2DasmModel::setFont(const QFont& font)
{
    m_font = font;
    invalidate();
}

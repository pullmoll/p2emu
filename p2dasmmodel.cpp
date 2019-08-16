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

P2DasmModel::P2DasmModel(P2Dasm* dasm, QObject *parent)
    : QAbstractTableModel(parent)
    , m_dasm(dasm)
    , m_font(QStringLiteral("Monospace"), 8, QFont::Normal, false)
    , m_bold(QStringLiteral("Monospace"), 8, QFont::Bold, false)
{
}

QVariant P2DasmModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;
    QFontMetrics metrics(m_bold);
    const column_e column = static_cast<column_e>(section);

    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            switch (column) {
            case c_Address:
                result = tr("Address");
                break;
            case c_Opcode:
                result = tr("Opcode");
                break;
            case c_Instruction:
                result = tr("Instruction");
                break;
            case c_Comment:
                result = tr("Comment");
                break;
            }
            break;

        case Qt::FontRole:
            result = m_bold;
            break;


        case Qt::SizeHintRole:
            result = sizeHint(column, true);
            break;

        case Qt::TextAlignmentRole:
            result = qVariantFromValue(alignment(column));
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
    return m_dasm->memsize() / 4;
}

int P2DasmModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 4;
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
    bool known = m_dasm->dasm(PC, opcode, instruction, &description);
    Q_UNUSED(known)

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Address: // Address as COG[xxx], LUT[xxx], or xxxxxx in RAM
            if (PC < 0x200) {
                result = QString("COG[$%1]").arg(PC, 3, 16, QChar('0'));
            } else if (PC < 0x400) {
                result = QString("LUT[$%1]").arg(PC - 0x200, 3, 16, QChar('0'));
            } else {
                result = QString("$%1").arg(addr, 8, 16, QChar('0'));
            }
            break;
        case c_Opcode: // Opcode string
            result = opcode;
            break;
        case c_Instruction: // Disassembled instruction string
            result = instruction;
            break;
        case c_Comment: // Comments
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
        result = qVariantFromValue(alignment(column));
        break;

    case Qt::BackgroundRole:
        result = background(column);
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

QColor P2DasmModel::background(P2DasmModel::column_e column) const
{
    QColor result;
    switch (column) {
    case c_Address:
        result = QColor(0xff,0xfc,0xf8);
        break;
    case c_Opcode:
        result = QColor(0xf8,0xfc,0xff);
        break;
    case c_Instruction:
        result = QColor(0xff,0xff,0xff);
        break;
    case c_Comment:
        result = QColor(0xf8,0xff,0xf8);
        break;
    }
    return result;
}

Qt::Alignment P2DasmModel::alignment(column_e column) const
{
    Qt::Alignment result;
    switch (column) {
    case c_Address:
        result = Qt::AlignLeft | Qt::AlignVCenter;
        break;
    case c_Opcode:
        result = Qt::AlignLeft | Qt::AlignVCenter;
        break;
    case c_Instruction:
        result = Qt::AlignLeft | Qt::AlignVCenter;
        break;
    case c_Comment:
        result = Qt::AlignLeft | Qt::AlignVCenter;
        break;
    }
    return result;
}

QSize P2DasmModel::sizeHint(column_e column, bool bold) const
{
    QFontMetrics metrics(bold ? m_bold : m_font);
    QSize result;
    switch (column) {
    case c_Address: // Address
        result = metrics.size(Qt::TextSingleLine, QStringLiteral(" COG[$000] "));
        break;
    case c_Opcode: // Opcode string
        result = metrics.size(Qt::TextSingleLine, QStringLiteral(" EEEE_OOOOOOO_CZI_DDDDDDDDD_SSSSSSSSS "));
        break;
    case c_Instruction: // Disassembled instruction string
        result = metrics.size(Qt::TextSingleLine, QStringLiteral(" IF_NC_AND_NZ  INSTRUCTION #$1ff,#$1ff,#7 XORCZ "));
        break;
    case c_Comment:
        result = metrics.size(Qt::TextSingleLine, QStringLiteral(" Some comment string... "));
        break;
    }
    return result;
}

void P2DasmModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

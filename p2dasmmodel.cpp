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

static const QString str_Address = QStringLiteral(" COG[$000] ");
static const QString str_Opcode = QStringLiteral(" EEEE_OOOOOOO_CZI_DDDDDDDDD_SSSSSSSSS ");
static const QString str_Instruction = QStringLiteral(" IF_NC_AND_NZ  INSTRUCTION #$1ff,#$1ff,#7 XORCZ ");
static const QString str_Comment = QStringLiteral(" Some comment string... ");

P2DasmModel::P2DasmModel(P2Dasm* dasm, QObject *parent)
    : QAbstractTableModel(parent)
    , m_dasm(dasm)
    , m_font(QStringLiteral("Monospace"), 8, QFont::Normal, false)
    , m_bold(QStringLiteral("Monospace"), 8, QFont::Bold, false)
    , m_size_normal()
    , m_size_bold()
{

    QFontMetrics metrics_normal(m_font);
    m_size_normal.insert(c_Address,     metrics_normal.size(Qt::TextSingleLine, str_Address));
    m_size_normal.insert(c_Opcode,      metrics_normal.size(Qt::TextSingleLine, str_Opcode));
    m_size_normal.insert(c_Instruction, metrics_normal.size(Qt::TextSingleLine, str_Instruction));
    m_size_normal.insert(c_Comment,     metrics_normal.size(Qt::TextSingleLine, str_Comment));

    QFontMetrics metrics_bold(m_bold);
    m_size_bold.insert(c_Address,       metrics_bold.size(Qt::TextSingleLine, str_Address));
    m_size_bold.insert(c_Opcode,        metrics_bold.size(Qt::TextSingleLine, str_Opcode));
    m_size_bold.insert(c_Instruction,   metrics_bold.size(Qt::TextSingleLine, str_Instruction));
    m_size_bold.insert(c_Comment,       metrics_bold.size(Qt::TextSingleLine, str_Comment));

    m_alignment.insert(c_Address,       Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Opcode,        Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Instruction,   Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Comment,       Qt::AlignLeft | Qt::AlignVCenter);

    m_background.insert(c_Address,      qRgb(0xff,0xfc,0xf8));
    m_background.insert(c_Opcode,       qRgb(0xf8,0xfc,0xff));
    m_background.insert(c_Instruction,  qRgb(0xff,0xff,0xff));
    m_background.insert(c_Comment,      qRgb(0xf8,0xff,0xf8));
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
            result = m_size_bold.value(column);
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
        result = m_alignment.value(column);
        break;

    case Qt::BackgroundRole:
        result = m_background.value(column);
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
        result = m_size_normal.value(column);
        break;

    case Qt::InitialSortOrderRole:
        break;
    }
    return result;
}

void P2DasmModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

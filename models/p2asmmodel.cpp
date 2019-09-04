/****************************************************************************
 *
 * Propeller2 assembler data model for QTableView implementation
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
#include "p2asmmodel.h"
#include "p2util.h"

static const QString style_nowrap = QStringLiteral("style='white-space:nowrap;'");
static const QString style_left = QStringLiteral("style='text-align:left;'");
static const QString style_padding = QStringLiteral("style='padding:0px 4px 0px 4px;'");

P2AsmModel::P2AsmModel(P2Asm* p2asm, QObject *parent)
    : QAbstractTableModel(parent)
    , m_asm(p2asm)
    , m_format(fmt_bin)
    , m_font()
    , m_error_pixmap()
    , m_error()
    , m_head_alignment()
    , m_text_alignment()
{
    m_error_pixmap = QPixmap(":/icons/error.svg");

    m_header.insert(c_Origin,           tr("OrgH"));
    m_background.insert(c_Origin,       qRgb(0xff,0xfc,0xf8));
    m_head_alignment.insert(c_Origin,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Origin,   Qt::AlignLeft | Qt::AlignVCenter);

    m_header.insert(c_Address,          tr("Org"));
    m_background.insert(c_Address,      qRgb(0xff,0xf0,0xe0));
    m_head_alignment.insert(c_Address,  Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Address,  Qt::AlignLeft | Qt::AlignVCenter);

    m_header.insert(c_Opcode,           tr("Opcode"));
    m_background.insert(c_Opcode,       qRgb(0xf8,0xfc,0xff));
    m_head_alignment.insert(c_Opcode,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Opcode,   Qt::AlignLeft | Qt::AlignVCenter);

    m_header.insert(c_Errors,           tr("Errors"));
    m_background.insert(c_Errors,       qRgb(0xff,0xc0,0xc0));
    m_head_alignment.insert(c_Errors,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Errors,   Qt::AlignCenter);

    m_header.insert(c_Tokens,           tr("Tokens"));
    m_background.insert(c_Tokens,       qRgb(0x10,0xfc,0xff));
    m_head_alignment.insert(c_Tokens,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Tokens,   Qt::AlignCenter);

    m_header.insert(c_Symbols,          tr("Symbols"));
    m_background.insert(c_Symbols,      qRgb(0xff,0xf0,0xff));
    m_head_alignment.insert(c_Symbols,  Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Symbols,  Qt::AlignCenter);

    m_header.insert(c_Source,           tr("Source"));
    m_background.insert(c_Source,       qRgb(0xff,0xff,0xff));
    m_head_alignment.insert(c_Source,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Source,   Qt::AlignLeft | Qt::AlignVCenter);
}

QVariant P2AsmModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;
    const column_e column = static_cast<column_e>(section);
    QFontMetrics metrics(m_font);
    const int row = section;

    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            result = m_header.value(column);
            break;

        case Qt::FontRole:
            result = m_font;
            break;

        case Qt::SizeHintRole:
            result = sizeHint(createIndex(0, column));
            break;

        case Qt::TextAlignmentRole:
            result = m_head_alignment.value(column);
            break;

        case Qt::ToolTipRole:
            switch (column) {
            case c_Origin:
                result = tr("Origin of the current instruction.");
                break;
            case c_Address:
                result = tr("Program counter address of the current instruction.");
                break;
            case c_Errors:
                result = tr("Errors while assembling the current line.");
                break;
            case c_Tokens:
                result = tr("Tokens found when parsing the current line.");
                break;
            case c_Opcode:
                result = tr("Opcode of the current instruction, or value of assignment.");
                break;
            case c_Symbols:
                result = tr("Number of defined / referenced symbols.");
                break;
            case c_Source:
                result = tr("Source code line.");
                break;
            }
            break;
        }
        break;

    case Qt::Vertical:
        result = QString::number(row + 1);
        break;
    }
    return result;
}

int P2AsmModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    const int rows = m_asm->count();
    return rows;
}

int P2AsmModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    const int columns = c_Source + 1;
    return columns;
}

QVariant P2AsmModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if (!index.isValid())
        return result;

    const column_e column = static_cast<column_e>(index.column());
    const int row = index.row();
    const int lineno = row + 1;

    const P2SymbolTable symbols = m_asm->symbols();
    const QList<P2Symbol>& symrefs = symbols.isNull() ? QList<P2Symbol>()
                                                      : symbols->references_in(lineno);
    const QStringList& errors = m_asm->errors(lineno);
    const bool has_errors = !errors.isEmpty();
    const P2Words& words = m_asm->words(lineno);

    const bool has_PC = m_asm->has_PC_ORGH(lineno);
    const p2_ORG_ORGH_t PC_orgh = has_PC ? m_asm->get_PC_ORGH(lineno)
                                        : p2_ORG_ORGH_t();
    const p2_LONG PC = PC_orgh.first;
    const p2_LONG ORGH = PC_orgh.second;

    const bool has_IR = m_asm->has_IR(lineno);
    const P2Opcode IR = m_asm->get_IR(lineno);

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Origin: // Address as COG[xxx], LUT[xxx], or xxxxxx in RAM
            if (!has_PC)
                break;
            result = QString("%1").arg(ORGH, 6, 16, QChar('0'));
            break;

        case c_Address:
            if (!has_PC)
                break;
            if (PC < LUT_ADDR0) {
                result = QString("COG:%1").arg(PC / 4, 3, 16, QChar('0'));
            } else if (PC < HUB_ADDR0) {
                result = QString("LUT:%1").arg((PC - LUT_ADDR0) / 4, 3, 16, QChar('0'));
            } else {
                result = QString("%1").arg(PC, 7, 16, QChar('0'));
            }
            break;

        case c_Opcode: // Opcode string
            if (has_IR)
                result = qVariantFromValue(IR);
            break;

        case c_Tokens:
            if (words.isEmpty())
                result = QStringLiteral("-");
            else
                result = QString::number(words.count());
            break;

        case c_Errors:  // Error messages
            if (has_errors)
                result = template_str_errors;
            break;

        case c_Symbols:
            if (!symrefs.isEmpty())
                result = QString::number(symrefs.count());
            break;

        case c_Source:  // Source code
            result = m_asm->source(row);
            break;
        }
        break;

    case Qt::DecorationRole:
        switch (column) {
        case c_Source:
            if (has_errors)
                result = m_error;
            break;
        default:
            result.clear();
        }
        break;

    case Qt::EditRole:
        switch (column) {
        case c_Origin:
            if (has_PC)
                return qVariantFromValue(PC_orgh.second);
            break;

        case c_Address:
            if (has_PC)
                return qVariantFromValue(PC_orgh.first);
            break;

        case c_Opcode:
            if (has_IR)
                return qVariantFromValue(IR);
            break;

        case c_Tokens:
            if (!words.isEmpty())
                return qVariantFromValue(words);
            break;

        case c_Symbols:
            if (!symrefs.isEmpty())
                return qVariantFromValue(symrefs);
            break;

        case c_Errors:
            if (has_errors)
                return errors;
            break;

        case c_Source:
            return m_asm->source(row);
        }
        break;

    case Qt::ToolTipRole:
        switch (column) {
        case c_Origin:
            result = tr("This column shows the address where code/data is emitted to.");
            break;

        case c_Address:
            result = tr("This column shows the address of the program counter (PC).");
            break;

        case c_Opcode:
            if (has_IR)
                result = opcodeToolTip(IR);
            break;

        case c_Tokens:
            if (words.isEmpty())
                break;
            result = tokenToolTip(words);
            break;

        case c_Symbols:
            if (symrefs.isEmpty())
                break;
            result = symbolsToolTip(symbols, symrefs);
            break;

        case c_Errors:
            if (!has_errors)
                break;
            result = errorsToolTip(errors);
            break;

        case c_Source:
            result = tr("This column shows the source code.");
            break;
        }
        break;

    case Qt::StatusTipRole:
        break;

    case Qt::WhatsThisRole:
        break;

    case Qt::FontRole:
        result = m_font;
        break;

    case Qt::TextAlignmentRole:
        result = m_text_alignment.value(column);
        break;

    case Qt::BackgroundRole:
        result = QColor(m_background.value(column));
        switch (column) {
        case c_Errors:
            if (!has_errors)
                result = QColor(m_background.value(c_Source));
            break;
        default:
            ;
        }
        break;

    case Qt::SizeHintRole:
        result = sizeHint(index, data(index).toString());
        break;

    case Qt::InitialSortOrderRole:
        break;

    case Qt::UserRole:
        if (c_Source == column && !words.isEmpty())
            result = qVariantFromValue(words);
        break;
    }
    return result;
}

bool P2AsmModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result = false;

    if (!index.isValid())
        return result;
    const column_e column = static_cast<column_e>(index.column());
    const int row = index.row();
    switch (role) {
    case Qt::EditRole:
        switch (column) {
        case c_Source:
            result = m_asm->set_source(row, value.toString());
            break;
        default:
            result = false;
        }
        break;
    }

    return result;
}

QSize P2AsmModel::sizeHint(const QModelIndex& index, const QString& text) const
{
    const column_e column = static_cast<column_e>(index.column());
    QFontMetrics metrics(m_font);
    QSize size;

    switch (column) {
    case c_Origin:
        size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_origin : text);
        break;

    case c_Address:
        size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_address : text);
        break;

    case c_Opcode:
        switch (m_format) {
        case fmt_bin:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_bin : text);
            break;
        case fmt_byt:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_byt : text);
            break;
        case fmt_dec:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_dec : text);
            break;
        case fmt_hex:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_hex : text);
            break;
        }
        break;

    case c_Symbols:
        size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_symbols : text);
        break;

    case c_Tokens:
        size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_tokens : text);
        break;

    case c_Errors:
        size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_errors : text);
        break;

    case c_Source:
        size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_instruction : text);
        break;
    }
    return size;
}

Qt::ItemFlags P2AsmModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::NoItemFlags;
    if (!index.isValid())
        return flags;

    column_e column = static_cast<column_e>(index.column());
    switch (column) {
    case c_Source:
        flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEnabled;
    }

    return flags;
}

QList<P2AsmModel::column_e> P2AsmModel::columns()
{
    QList<column_e> columns;
    columns += c_Origin;
    columns += c_Address;
    columns += c_Opcode;
    columns += c_Tokens;
    columns += c_Symbols;
    columns += c_Errors;
    columns += c_Source;
    return columns;
}

p2_opcode_format_e P2AsmModel::opcode_format() const
{
    return m_format;
}

void P2AsmModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

void P2AsmModel::setOpcodeFormat(p2_opcode_format_e format)
{
    if (format == m_format)
        return;
    beginResetModel();
    m_format = format;
    endResetModel();
}

void P2AsmModel::setFont(const QFont& font)
{
    beginResetModel();
    m_font = font;
    QFontMetrics metrics(m_font);
    QSize size = QSize(metrics.averageCharWidth(), metrics.ascent() + metrics.descent());
    m_error = QIcon(m_error_pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    endResetModel();
}

const P2Word P2AsmModel::highlight(const QModelIndex& index) const
{
    if (index != m_highlight_index)
        return P2Word();
    return m_highlight_word;
}

void P2AsmModel::setHighlight(const QModelIndex& index, const P2Word& word)
{
    m_highlight_index = index;
    m_highlight_word = word;
}


static const QStringList html_head()
{
    QStringList html;
    html += QStringLiteral("<html>");
    html += QStringLiteral("<body>");
    html += QString("<table>");
    return html;
}

static const QStringList html_end()
{
    QStringList html;
    html += QStringLiteral("</table>");
    html += QStringLiteral("</body");
    html += QStringLiteral("</html>");
    return html;
}

static const QString html_start_tr()
{
    return QString("<tr %1>")
            .arg(style_padding);

}

static const QString html_end_tr()
{
    return QStringLiteral("</tr>");

}

static const QString html_th(const QString& text)
{
    return QString("<th %1 %2 %3><tt>%4</tt></th>")
            .arg(style_padding)
            .arg(style_nowrap)
            .arg(style_left)
            .arg(text);

}

static const QString html_td(const QString& text)
{
    return QString("<td %1 %2><tt>%3</tt></td>")
            .arg(style_padding)
            .arg(style_nowrap)
            .arg(text);
}

QString P2AsmModel::opcodeToolTip(const P2Opcode& IR) const
{
    QStringList html = html_head();
    QString title;
    QStringList lines;

    if (IR.as_IR) {
        title = tr("Opcode");
        lines += format_opcode(IR.u, m_format);
    }

    if (IR.as_EQU) {
        title = tr("Assigment");
        lines += format_data(IR.EQU.to_long(), m_format);
    }

    if (lines.isEmpty() && !IR.DATA.isEmpty()) {
        title = tr("Data");
        lines = P2Atom::format_data(IR.DATA, IR.ORG_ORGH.first);
    }

    // heading
    html += html_start_tr();
    html += html_th(title);
    html += html_end_tr();

    foreach(const QString& line, lines) {
        html += html_start_tr();
        html += html_td(line);
        html += html_end_tr();
    }

    html += html_end();
    return html.join(QChar::LineFeed);
}

QString P2AsmModel::tokenToolTip(const P2Words& words) const
{
    QStringList html = html_head();

    // heading
    html += html_start_tr();
    html += html_th(tr("Token"));
    html += html_th(tr("Type"));
    html += html_th(tr("Position, Length"));
    html += html_th(tr("Source code"));
    html += html_end_tr();

    for (int i = 0; i < words.count(); i++) {
        const P2Word& word = words[i];
        const QString& token_id = QString("%1").arg(word.tok(), 3, 16, QChar('0'));
        const QString& token_str = P2Util::esc(Token.enum_name(word.tok()));
        const QString& token_id_str = QString("%1: %2").arg(token_id).arg(token_str);
        const QString& type_names = P2Util::esc(Token.type_names(word.tok()).join(QChar::Space));
        const QString& pos_len = QString("@%1 +%2").arg(word.pos()).arg(word.len());
        const QString& source = P2Util::esc(word.str());

        html += html_start_tr();
        html += html_td(token_id_str);
        html += html_td(type_names);
        html += html_td(pos_len);
        html += html_td(source);
        html += html_end_tr();
    }
    html += html_end();
    return html.join(QChar::LineFeed);
}

QString P2AsmModel::symbolsToolTip(const P2SymbolTable& symbols, const QList<P2Symbol>& symrefs) const
{
    if (symbols.isNull())
        return QString();

    QStringList html = html_head();
    // heading
    html += html_start_tr();
    html += html_th(tr("Line #"));
    html += html_th(tr("Section::name"));
    html += html_th(tr("Type"));
    html += html_th(tr("Value (dec)"));
    html += html_th(tr("Value (hex)"));
    html += html_th(tr("Value (bin)"));
    html += html_end_tr();

    for (int i = 0; i < symrefs.count(); i++) {
        const P2Symbol& sym = symrefs[i];
        const P2Word& word = symbols->reference(sym.name());
        const P2Atom& atom = sym.atom();
        p2_LONG val = atom.to_long();
        html += html_start_tr();
        html += html_td(QString::number(word.lineno()));
        html += html_td(sym.name());
        html += html_td(sym.type_name());
        html += html_td(format_data_dec(val));
        html += html_td(format_data_hex(val));
        html += html_td(format_data_bin(val));
        html += html_end_tr();
    }
    html += html_end();
    return html.join(QChar::LineFeed);
}

QString P2AsmModel::errorsToolTip(const QStringList& list) const
{
    QStringList html = html_head();
    html += html_start_tr();
    html += html_th(tr("Error message"));
    html += html_end_tr();
    foreach (const QString& error, list) {
        html += html_start_tr();
        html += html_td(error);
        html += html_end_tr();
    }
    html += html_end();
    return html.join(QChar::LineFeed);
}

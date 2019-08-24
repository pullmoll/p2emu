/****************************************************************************
 *
 * Propeller2 assembler table model implementation
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

static const QString style_nowrap = QStringLiteral("style='white-space:nowrap;'");
static const QString style_left = QStringLiteral("style='text-align:left;'");
static const QString style_padding = QStringLiteral("style='padding:0px 4px 0px 4px;'");
static const QString style_background_error = QStringLiteral("style='background:#ffc0c0; border: 1px solid #ddd;'");
static const QString style_background_tokens = QStringLiteral("style='background:#10fcff; border: 1px solid #ddd;'");
static const QString style_background_symbols = QStringLiteral("style='background:#fff0ff; border: 1px solid #ddd;'");

P2AsmModel::P2AsmModel(P2Asm* p2asm, QObject *parent)
    : QAbstractTableModel(parent)
    , m_asm(p2asm)
    , m_format(fmt_bin)
    , m_font(QStringLiteral("Monospace"))
    , m_bold(QStringLiteral("Monospace"))
    , m_error()
    , m_header_alignment()
    , m_text_alignment()
{
    m_font.setPointSize(8);
    m_bold.setPointSize(8);
    m_bold.setBold(true);

    QImage image(":/icons/error.png");
    QPixmap pixmap = QPixmap::fromImage(image.scaled(16,16,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_error = QIcon(pixmap);

    // Header section names
    m_header.insert(c_Origin,       tr("Origin"));
    m_header.insert(c_Errors,       tr("Errors"));
    m_header.insert(c_Tokens,       tr("Tokens"));
    m_header.insert(c_Opcode,       tr("Opcode"));
    m_header.insert(c_Symbols,      tr("Symbols"));
    m_header.insert(c_Source,       tr("Source"));
    // Horizontal section background colors

    m_background.insert(c_Origin,   qRgb(0xff,0xfc,0xf8));
    m_background.insert(c_Opcode,   qRgb(0xf8,0xfc,0xff));
    m_background.insert(c_Tokens,   qRgb(0x10,0xfc,0xff));
    m_background.insert(c_Symbols,  qRgb(0xff,0xf0,0xff));
    m_background.insert(c_Errors,   qRgb(0xff,0xc0,0xc0));
    m_background.insert(c_Source,   qRgb(0xff,0xff,0xff));

    // Horizontal section alignments
    m_header_alignment.insert(c_Origin,     Qt::AlignLeft | Qt::AlignVCenter);
    m_header_alignment.insert(c_Opcode,     Qt::AlignLeft | Qt::AlignVCenter);
    m_header_alignment.insert(c_Tokens,     Qt::AlignLeft | Qt::AlignVCenter);
    m_header_alignment.insert(c_Errors,     Qt::AlignLeft | Qt::AlignVCenter);
    m_header_alignment.insert(c_Symbols,    Qt::AlignLeft | Qt::AlignVCenter);
    m_header_alignment.insert(c_Source,     Qt::AlignLeft | Qt::AlignVCenter);

    // Item alignments
    m_text_alignment.insert(c_Origin,       Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Opcode,       Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Tokens,       Qt::AlignCenter);
    m_text_alignment.insert(c_Errors,       Qt::AlignCenter);
    m_text_alignment.insert(c_Symbols,      Qt::AlignCenter);
    m_text_alignment.insert(c_Source,       Qt::AlignLeft | Qt::AlignVCenter);
}

QVariant P2AsmModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;
    QFontMetrics metrics(m_bold);
    const column_e column = static_cast<column_e>(section);
    const int row = section;

    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            result = m_header.value(column);
            break;

        case Qt::FontRole:
            result = m_bold;
            break;

        case Qt::SizeHintRole:
            result = sizeHint(column);
            break;

        case Qt::TextAlignmentRole:
            result = m_header_alignment.value(column);
            break;

        case Qt::ToolTipRole:
            switch (column) {
            case c_Origin:
                result = tr("Origin of the current instruction.");
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

static const QStringList html_table_init(const QString& bgd)
{
    QStringList html;
    html += QStringLiteral("<html>");
    html += QStringLiteral("<body>");
    html += QString("<table %1>").arg(bgd);
    return html;
}

static const QStringList html_table_exit()
{
    QStringList html;
    html += QStringLiteral("</table>");
    html += QStringLiteral("</body");
    html += QStringLiteral("</html>");
    return html;
}

static const QString html_th(const QString& text)
{
    return QString("<th %1 %2 %3><tt>%4</tt></th>")
            .arg(style_nowrap)
            .arg(style_padding)
            .arg(style_left)
            .arg(text);

}

static const QString html_tr_init()
{
    return QString("<tr %1>")
            .arg(style_padding);

}

static const QString html_tr_exit()
{
    return QStringLiteral("</tr>");

}

static const QString html_td(const QString& text)
{
    return QString("<td %1 %2><tt>%3</tt></td>")
            .arg(style_padding)
            .arg(style_nowrap)
            .arg(text);
}

QString P2AsmModel::tokenToolTip(const P2AsmWords& words, const QString& bgd) const
{
    QStringList html = html_table_init(bgd);

    // heading
    html += html_tr_init();
    html += html_th(tr("Source code word"));
    html += html_th(tr("Token"));
    html += html_th(tr("Type"));
    html += html_tr_exit();

    for (int i = 0; i < words.count(); i++) {
        const P2AsmWord& word = words[i];
        html += html_tr_init();
        html += html_td(word.str());
        html += html_td(Token.string(word.tok()));
        html += html_td(Token.type_names(word.tok()).join(QChar::Space));
        html += html_tr_exit();
    }
    html += html_table_exit();
    return html.join(QChar::LineFeed);
}

QString P2AsmModel::symbolsToolTip(const P2AsmSymTbl& symbols, const QStringList& defined, const QString& bgd) const
{
    QStringList html = html_table_init(bgd);
    // heading
    html += html_tr_init();
    html += html_th(tr("Section::name"));
    html += html_th(tr("Type"));
    html += html_th(tr("Value (dec)"));
    html += html_th(tr("Value (hex)"));
    html += html_th(tr("Value (bin)"));
    html += html_tr_exit();

    foreach (const QString& symbol, defined) {
        P2AsmSymbol sym = symbols.value(symbol);
        p2_LONG val = sym.value<p2_LONG>();
        html += html_tr_init();
        html += html_td(sym.name());
        html += html_td(sym.type_name());
        html += html_td(QString("%1").arg(val, 11, 10));
        html += html_td(QString("$%1").arg(val, 8, 16, QChar('0')));
        html += html_td(QString("%%1").arg(val, 32, 2, QChar('0')));
        html += html_tr_exit();
    }
    html += html_table_exit();
    return html.join(QChar::LineFeed);
}

QString P2AsmModel::errorsToolTip(const QStringList& list, const QString& bgd) const
{
    QStringList html = html_table_init(bgd);
    html += html_tr_init();
    html += html_th(tr("Error message"));
    html += html_tr_exit();
    foreach (const QString& error, list) {
        html += html_tr_init();
        html += html_td(error);
        html += html_tr_exit();
    }
    html += html_table_exit();
    return html.join(QChar::LineFeed);
}

QVariant P2AsmModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if (!index.isValid())
        return result;

    const column_e column = static_cast<column_e>(index.column());
    const int row = index.row();
    const int lineno = row + 1;

    const P2AsmSymTbl& symbols = m_asm->symbols();
    const QStringList& symrefs = symbols.defined_in(lineno);
    const QStringList& errors = m_asm->errors(lineno);

    const P2AsmWords& words = m_asm->words(lineno);
    const QStringList& defined_in = symbols.defined_in(lineno);

    const bool PC_avail = m_asm->PC_available(lineno);
    const p2_LONG PC = PC_avail ? m_asm->PC_value(lineno) : 0;

    const bool IR_avail = m_asm->IR_available(lineno);
    const p2_opcode_u IR = m_asm->IR_value(lineno);

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Origin: // Address as COG[xxx], LUT[xxx], or xxxxxx in RAM
            if (!PC_avail)
                break;
            if (PC < 0x200) {
                result = QString("COG[%1]").arg(PC, 3, 16, QChar('0'));
            } else if (PC < PC_LONGS) {
                result = QString("LUT[%1]").arg(PC - 0x200, 3, 16, QChar('0'));
            } else {
                result = QString("%1").arg(PC, 6, 16, QChar('0'));
            }
            break;

        case c_Tokens:
            if (!words.isEmpty())
                result = QString::number(words.count());
            break;

        case c_Errors:  // Error messages
            if (!errors.isEmpty())
                result = template_str_errors;
            break;

        case c_Symbols:
            if (!symrefs.isEmpty())
                result = QString::number(symrefs.count());
            break;

        case c_Opcode: // Opcode string
            if (PC_avail) {
                result = format_opcode(IR, m_format);
                break;
            }
            if (IR_avail) {
                result = format_data(IR, m_format);
                break;
            }
            break;

        case c_Source:  // Source code
            result = m_asm->source(row);
            break;
        }
        break;

    case Qt::DecorationRole:
        switch (column) {
        case c_Errors:
            if (!m_asm->errors(lineno).isEmpty())
                result = m_error;
            break;
        default:
            result.clear();
        }
        break;

    case Qt::EditRole:
        switch (column) {
        case c_Origin:
            if (m_asm->PC_available(lineno))
                return m_asm->PC_value(lineno);
            break;

        case c_Opcode:
            if (IR_avail)
                return IR.opcode;
            break;

        case c_Tokens:
            if (!words.isEmpty())
                return qVariantFromValue(words);
            break;

        case c_Symbols:
            if (!symrefs.isEmpty())
                return symrefs;
            break;

        case c_Errors:
            if (!errors.isEmpty())
                return errors;
            break;

        case c_Source:
            return m_asm->source(row);
        }
        break;

    case Qt::ToolTipRole:
        switch (column) {
        case c_Tokens:
            if (words.isEmpty())
                break;
            result = tokenToolTip(words, style_background_tokens);
            break;

        case c_Symbols:
            if (defined_in.isEmpty())
                break;
            result = symbolsToolTip(symbols, defined_in, style_background_symbols);
            break;

        case c_Errors:
            if (errors.isEmpty())
                break;
            result = errorsToolTip(errors, style_background_error);
            break;

        default:
            result.clear();
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

    case Qt::UserRole:
        if (c_Source == column && !words.isEmpty())
            result = qVariantFromValue(words);
        break;
    }
    return result;
}

QSize P2AsmModel::sizeHint(P2AsmModel::column_e column) const
{
    QFontMetrics metrics(m_font);

    switch (column) {
    case c_Origin:
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

    case c_Symbols:
        return metrics.size(Qt::TextSingleLine, template_str_symbols);

    case c_Tokens:
        return metrics.size(Qt::TextSingleLine, template_str_tokens);

    case c_Errors:
        return metrics.size(Qt::TextSingleLine, template_str_errors);

    case c_Source:
        return metrics.size(Qt::TextSingleLine, template_str_instruction);
    }
    return QSize();
}

#if OVERRIDE_FLAGS
Qt::ItemFlags P2AsmModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    column_e column = static_cast<column_e>(index.column());
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (c_Source == column)
        flags |= Qt::ItemIsEditable;
    return flags;
}
#endif

void P2AsmModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

void P2AsmModel::setOpcodeFormat(p2_opcode_format_e format)
{
    if (format == m_format)
        return;
    m_format = format;
    switch (format) {
    case fmt_bin:
        m_header.insert(c_Opcode, tr("Opcode bit fields"));
        break;
    case fmt_byt:
        m_header.insert(c_Opcode, tr("Opcode BYTES hex"));
        break;
    case fmt_oct:
        m_header.insert(c_Opcode, tr("Opcode LONG oct"));
        break;
    case fmt_hex:
        m_header.insert(c_Opcode, tr("Opcode LONG hex"));
        break;
    }
    invalidate();
}

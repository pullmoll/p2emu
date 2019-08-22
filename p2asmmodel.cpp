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

static const QString str_address = QStringLiteral("COG[000] ");
static const QString str_opcode_binary = QStringLiteral("EEEE_OOOOOOO_CZI_DDDDDDDDD_SSSSSSSSS ");
static const QString str_opcode_hexdec = QStringLiteral("FFFFFFFF ");
static const QString str_opcode_octal = QStringLiteral("37777777777 ");
static const QString str_tokens = QStringLiteral("T");
static const QString str_symbols = QStringLiteral("S");
static const QString str_errors = QStringLiteral("😞");
static const QString str_instruction = QStringLiteral(" IF_NC_AND_NZ  INSTRUCTION #$1ff,#$1ff,#7 XORCZ AND SOME MORE COLUMNS ");

static const QString style_nowrap = QStringLiteral("style='white-space:nowrap;'");
static const QString style_left = QStringLiteral("style='text-align:left;'");
static const QString style_padding = QStringLiteral("style='padding:2px;'");
static const QString style_background_error = QStringLiteral("style='background:#ffc0c0;'");
static const QString style_background_tokens = QStringLiteral("style='background:#10fcff;'");
static const QString style_background_symbols = QStringLiteral("style='background:#fff0ff;'");

P2AsmModel::P2AsmModel(P2Asm* p2asm, QObject *parent)
    : QAbstractTableModel(parent)
    , m_asm(p2asm)
    , m_opcode_format(f_bin)
    , m_font()
    , m_bold()
    , m_error()
    , m_size_normal()
    , m_size_bold()
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

    updateSizes();
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
            result = m_size_bold.value(column);
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
        result = row + 1;
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

QString P2AsmModel::tokenToolTip(const p2_token_v& tokenv, const QStringList& words, const QString& bgd) const
{
    QStringList types;
    QStringList tokens;

    foreach(const p2_token_e tok, tokenv) {
        types += Token.typeName(tok);
        tokens += Token.string(tok);
    }

    QStringList html = html_table_init(bgd);

    // heading
    html += html_tr_init();
    html += html_th(tr("Type"));
    html += html_th(tr("Token"));
    html += html_th(tr("Source"));
    html += html_tr_exit();

    for (int i = 0; i < types.count(); i++) {
        html += html_tr_init();
        html += html_td(types.value(i));
        html += html_td(tokens.value(i));
        html += html_td(words.value(i));
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
    html += html_th(tr("Symbol name"));
    html += html_th(tr("Value (dec)"));
    html += html_th(tr("Value (hex)"));
    html += html_th(tr("Value (bin)"));
    html += html_tr_exit();

    foreach (const QString& symbol, defined) {
        P2AsmSymbol sym = symbols.value(symbol);
        P2LONG val = sym.value<P2LONG>();
        html += html_tr_init();
        html += html_td(sym.name());
        html += html_td(QString("%1").arg(val));
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

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Origin: // Address as COG[xxx], LUT[xxx], or xxxxxx in RAM
            if (m_asm->hPC().contains(lineno)) {
                const P2LONG PC = m_asm->hPC().value(lineno);
                if (PC < 0x200) {
                    result = QString("COG[%1]").arg(PC, 3, 16, QChar('0'));
                } else if (PC < PC_LONGS) {
                    result = QString("LUT[%1]").arg(PC - 0x200, 3, 16, QChar('0'));
                } else {
                    result = QString("%1").arg(PC, 6, 16, QChar('0'));
                }
            }
            break;

        case c_Tokens:
            {
                p2_token_v tokens = m_asm->hTokens().value(lineno);
                if (!tokens.isEmpty())
                    result = QString::number(tokens.count());
            }
            break;

        case c_Errors:  // Error messages
            {
                if (m_asm->hErrors().contains(lineno))
                    result = str_errors;
            }
            break;

        case c_Symbols:
            {
                QStringList symrefs = m_asm->symbols().defined_in(lineno);
                if (!symrefs.isEmpty())
                    result = QString::number(symrefs.count());
            }
            break;

        case c_Opcode: // Opcode string
            if (m_asm->hPC().contains(lineno)) {
                const p2_opcode_u IR = m_asm->hIR().value(lineno);
                switch (m_opcode_format) {
                case f_bin:
                    result = QString("%1_%2_%3%4%5_%6_%7")
                             .arg(IR.op.cond, 4, 2, QChar('0'))
                             .arg(IR.op.inst, 7, 2, QChar('0'))
                             .arg(IR.op.wc, 1, 2, QChar('0'))
                             .arg(IR.op.wz, 1, 2, QChar('0'))
                             .arg(IR.op.im, 1, 2, QChar('0'))
                             .arg(IR.op.dst, 9, 2, QChar('0'))
                             .arg(IR.op.src, 9, 2, QChar('0'));
                    break;
                case f_oct:
                    result = QString("%1").arg(IR.opcode, 11, 8, QChar('0'));
                    break;
                case f_hex:
                    result = QString("%1").arg(IR.opcode, 8, 16, QChar('0'));
                    break;
                }
            } else if (m_asm->hIR().contains(lineno)) {
                const p2_opcode_u IR = m_asm->hIR().value(lineno);
                switch (m_opcode_format) {
                case f_bin:
                    result = QString("%1_%2_%3_%4")
                             .arg((IR.opcode >> 24) & 0xff, 8, 2, QChar('0'))
                             .arg((IR.opcode >> 16) & 0xff, 8, 2, QChar('0'))
                             .arg((IR.opcode >>  8) & 0xff, 8, 2, QChar('0'))
                             .arg((IR.opcode >>  0) & 0xff, 8, 2, QChar('0'));
                    break;
                case f_oct:
                    result = QString("%1").arg(IR.opcode, 11, 8, QChar('0'));
                    break;
                case f_hex:
                    result = QString("%1").arg(IR.opcode, 8, 16, QChar('0'));
                    break;
                }
            }
            break;

        case c_Source:  // Source code
            result = m_asm->source().value(row);
            break;
        }
        break;

    case Qt::DecorationRole:
        switch (column) {
        case c_Errors:
            if (m_asm->hErrors().contains(lineno))
                result = m_error;
            break;
        default:
            result.clear();
        }
        break;

    case Qt::EditRole:
        break;

    case Qt::ToolTipRole:
        switch (column) {
        case c_Tokens:
            {
                const p2_token_v& tokens = m_asm->hTokens().value(lineno);
                const QStringList& words = m_asm->hWords().value(lineno);
                if (words.isEmpty())
                    break;
                result = tokenToolTip(tokens, words, style_background_tokens);
            }
            break;

        case c_Symbols:
            {
                const P2AsmSymTbl& symbols = m_asm->symbols();
                QStringList defined = symbols.defined_in(lineno);
                if (defined.isEmpty())
                    break;
                result = symbolsToolTip(symbols, defined, style_background_symbols);
            }
            break;

        case c_Errors:
            {
                QStringList list = m_asm->hErrors().values(lineno);
                if (list.isEmpty())
                    break;
                result = errorsToolTip(list, style_background_error);
            }
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
        result = m_size_normal.value(column);
        break;

    case Qt::InitialSortOrderRole:
        break;
    }
    return result;
}

bool P2AsmModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

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

void P2AsmModel::invalidate()
{
    beginResetModel();
    updateSizes();
    endResetModel();
}

void P2AsmModel::setOpcodeFormat(p2_opcode_format_e format)
{
    if (format == m_opcode_format)
        return;
    m_opcode_format = format;
    invalidate();
}

void P2AsmModel::updateSizes()
{
    QFontMetrics metrics_n(m_font);
    QFontMetrics metrics_b(m_bold);

    m_size_normal.insert(c_Origin,      metrics_n.size(Qt::TextSingleLine, str_address));
    m_size_bold.insert(c_Origin,        metrics_b.size(Qt::TextSingleLine, str_address));

    m_size_normal.insert(c_Tokens,   metrics_n.size(Qt::TextSingleLine, str_tokens));
    m_size_bold.insert(c_Tokens,     metrics_b.size(Qt::TextSingleLine, str_tokens));

    switch (m_opcode_format) {
    case f_bin:
        m_header.insert(c_Opcode, QStringLiteral("EEEE_IIIIIII_CZI_DDDDDDDDD_SSSSSSSSS"));
        m_size_normal.insert(c_Opcode,  metrics_n.size(Qt::TextSingleLine, str_opcode_binary));
        m_size_bold.insert(c_Opcode,    metrics_b.size(Qt::TextSingleLine, str_opcode_binary));
        break;
    case f_hex:
        m_header.insert(c_Opcode, tr("Op (hex)"));
        m_size_normal.insert(c_Opcode,  metrics_n.size(Qt::TextSingleLine, str_opcode_hexdec));
        m_size_bold.insert(c_Opcode,    metrics_b.size(Qt::TextSingleLine, str_opcode_hexdec));
        break;
    case f_oct:
        m_header.insert(c_Opcode, tr("Op (oct)"));
        m_size_normal.insert(c_Opcode,  metrics_n.size(Qt::TextSingleLine, str_opcode_octal));
        m_size_bold.insert(c_Opcode,    metrics_b.size(Qt::TextSingleLine, str_opcode_octal));
        break;
    }
    m_size_normal.insert(c_Errors, metrics_n.size(Qt::TextSingleLine, str_errors));
    m_size_bold.insert(c_Errors,   metrics_b.size(Qt::TextSingleLine, str_errors));

    m_size_normal.insert(c_Symbols,metrics_n.size(Qt::TextSingleLine, str_symbols));
    m_size_bold.insert(c_Symbols,  metrics_b.size(Qt::TextSingleLine, str_symbols));

    m_size_normal.insert(c_Source, metrics_n.size(Qt::TextSingleLine, str_instruction));
    m_size_bold.insert(c_Source,   metrics_b.size(Qt::TextSingleLine, str_instruction));
}

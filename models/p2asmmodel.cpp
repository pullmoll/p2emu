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
#include "p2html.h"

static const QLatin1String style("style");
static const QLatin1String style_nowrap("white-space:nowrap;");
static const QLatin1String style_left("text-align:left;");
static const QLatin1String style_padding("padding:0px 4px 0px 4px;");

P2AsmModel::P2AsmModel(P2Asm* p2asm, QObject *parent)
    : QAbstractTableModel(parent)
    , m_asm(p2asm)
    , m_format(fmt_bin)
    , m_font()
    , m_error_pixmap()
    , m_error()
    , m_highlite_index()
    , m_highlight_symbol()
    , m_highlight_word()
    , m_head_alignment()
    , m_text_alignment()
{
    m_error_pixmap = QPixmap(":/icons/error.svg");

    m_header.insert(c_Origin,           tr("OrgH"));
    m_background.insert(c_Origin,       qRgb(0xff,0xfc,0xf8));
    m_head_alignment.insert(c_Origin,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Origin,   Qt::AlignLeft | Qt::AlignTop);

    m_header.insert(c_Address,          tr("Org"));
    m_background.insert(c_Address,      qRgb(0xff,0xf0,0xe0));
    m_head_alignment.insert(c_Address,  Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Address,  Qt::AlignLeft | Qt::AlignTop);

    m_header.insert(c_Opcode,           tr("Opcode"));
    m_background.insert(c_Opcode,       qRgb(0xf8,0xfc,0xff));
    m_head_alignment.insert(c_Opcode,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Opcode,   Qt::AlignLeft | Qt::AlignTop);

    m_header.insert(c_Errors,           tr("Errors"));
    m_background.insert(c_Errors,       qRgb(0xff,0xc0,0xc0));
    m_head_alignment.insert(c_Errors,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Errors,   Qt::AlignHCenter | Qt::AlignTop);

    m_header.insert(c_Tokens,           tr("Tokens"));
    m_background.insert(c_Tokens,       qRgb(0x10,0xfc,0xff));
    m_head_alignment.insert(c_Tokens,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Tokens,   Qt::AlignHCenter | Qt::AlignTop);

    m_header.insert(c_Symbols,          tr("Symbols"));
    m_background.insert(c_Symbols,      qRgb(0xff,0xf0,0xff));
    m_head_alignment.insert(c_Symbols,  Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Symbols,  Qt::AlignHCenter | Qt::AlignTop);

    m_header.insert(c_Source,           tr("Source"));
    m_background.insert(c_Source,       qRgb(0xff,0xff,0xff));
    m_head_alignment.insert(c_Source,   Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Source,   Qt::AlignLeft | Qt::AlignTop);
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

    const P2SymbolTable& symbols = m_asm->symbols();

    const bool has_symbols = !symbols.isNull();
    const QStringList& errors = m_asm->errors(lineno);

    const bool has_errors = !errors.isEmpty();
    const P2Words& words = m_asm->words(lineno);

    const bool has_ORIGIN = m_asm->has_ORIGIN(lineno);
    const p2_ORIGIN_t origin = has_ORIGIN ? m_asm->get_ORIGIN(lineno)
                                       : p2_ORIGIN_t();
    const p2_LONG cogaddr = origin._cog;
    const p2_LONG hubaddr = origin._hub;

    const bool has_IR = m_asm->has_IR(lineno);
    const P2Opcode& IR = m_asm->get_IR(lineno);

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Origin: // Address as COG[xxx], LUT[xxx], or xxxxxx in RAM
            if (!has_ORIGIN)
                break;
            result = QString("%1").arg(hubaddr, 6, 16, QChar('0'));
            break;

        case c_Address:
            if (!has_ORIGIN)
                break;
            if (cogaddr < LUT_ADDR0) {
                result = QString("COG:%1").arg(cogaddr / 4, 3, 16, QChar('0'));
            } else if (cogaddr < HUB_ADDR0) {
                result = QString("LUT:%1").arg((cogaddr - LUT_ADDR0) / 4, 3, 16, QChar('0'));
            } else {
                result = QString("%1").arg(cogaddr, 6, 16, QChar('0'));
            }
            break;

        case c_Opcode: // Opcode string
            if (has_IR)
                result = QVariant::fromValue(IR);
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
            if (has_symbols) {
                QList<P2Symbol> refs = symbols->references_in(lineno);
                if (refs.isEmpty())
                    result = QStringLiteral("-");
                else
                    result = QString::number(refs.count());
            }
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
            if (has_ORIGIN)
                result = QVariant::fromValue(origin._hub);
            break;

        case c_Address:
            if (has_ORIGIN)
                result = QVariant::fromValue(origin._cog);
            break;

        case c_Opcode:
            if (has_IR)
                result = QVariant::fromValue(IR);
            break;

        case c_Tokens:
            if (!words.isEmpty())
                result = QVariant::fromValue(words);
            break;

        case c_Symbols:
            if (has_symbols)
                break;
            result = QVariant::fromValue(symbols->references_in(lineno));
            break;

        case c_Errors:
            if (has_errors)
                result = errors;
            break;

        case c_Source:
            return m_asm->source(row);
        }
        break;

    case Qt::ToolTipRole:
        switch (column) {
        case c_Origin:
            result = tr("This column shows the HUB address.");
            break;

        case c_Address:
            result = tr("This column shows the COG address.");
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
            if (has_symbols)
                result = symbolsToolTip(symbols, symbols->references_in(lineno));
            break;

        case c_Errors:
            if (has_errors)
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
            result = QVariant::fromValue(words);
        break;

    case Qt::UserRole+1:
        if (has_symbols && index == m_highlite_index)
            result = QVariant::fromValue(m_highlight_word);
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
        case fmt_bit:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_byt : text);
            break;
        case fmt_dec:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_dec : text);
            break;
        case fmt_hex:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_hex : text);
            break;
        case fmt_doc:
            size = metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_opcode_doc : text);
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

p2_FORMAT_e P2AsmModel::opcode_format() const
{
    return m_format;
}

void P2AsmModel::invalidate()
{
    beginResetModel();
    endResetModel();
}

void P2AsmModel::setOpcodeFormat(p2_FORMAT_e format)
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

void P2AsmModel::set_highlight(const QModelIndex& index, const P2Symbol& symbol, const P2Word& word)
{
    m_highlite_index = index;
    m_highlight_symbol = symbol;
    m_highlight_word = word;
}

QString P2AsmModel::opcodeToolTip(const P2Opcode& IR) const
{
    QDomDocument doc;

    QDomElement table = p2_html(doc, "table");
    QDomElement tr, th, td, tt;
    QString title1;
    QString title2;
    QStringList column1;
    QStringList column2;

    if (IR.is_instruction()) {
        title1 = this->tr("Opcode");
        column1 += P2Opcode::format_opcode(IR, m_format).split(QChar::LineFeed);
        title2 = this->tr("Brief");
        column2 += P2Opcode::format_opcode(IR, fmt_doc).split(QChar::LineFeed);
    }

    if (IR.is_assign()) {
        title1 = this->tr("Assigment");
        column1 += P2Opcode::format_assign(IR, m_format).split(QChar::LineFeed);
    }

    if (IR.is_data()) {
        title1 = this->tr("Data");
        int limit = 32;
        QStringList lines = P2Opcode::format_data(IR, m_format, &limit).split(QChar::LineFeed);
        if (limit != 32)
            lines += this->tr("%1 more …").arg(limit - 32);
        column1 += lines;
    }

    // heading
    tr = p2_html(doc, "tr");

    th = p2_html(doc, "th");
    tt = p2_html(doc, "tt");
    tt.appendChild(p2_text(doc, title1));
    th.appendChild(tt);
    tr.appendChild(th);

    if (!title2.isEmpty()) {
        th = p2_html(doc, "th");
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, title2));
        th.appendChild(tt);
        tr.appendChild(th);
    }

    table.appendChild(tr);

    // table rows
    for (int i = 0; i < column1.count(); i++) {
        const QString& line = column1[i];
        tr = p2_html(doc, "tr");
        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, line));
        td.appendChild(tt);
        tr.appendChild(td);

        if (!column2.isEmpty()) {
            td = p2_html(doc, "td");
            td.setAttribute(style, style_nowrap);
            tt = p2_html(doc, "tt");
            tt.appendChild(p2_text(doc, column2.value(i)));
            td.appendChild(tt);
            tr.appendChild(td);
        }
        table.appendChild(tr);
    }
    doc.appendChild(table);
    return doc.toString(1);
}

QString P2AsmModel::tokenToolTip(const P2Words& words) const
{
    QDomDocument doc;

    QStringList headings;
    headings += tr("Token");
    headings += tr("Type");
    headings += tr("Pos");
    headings += tr("Len");
    headings += tr("Source code");

    QDomElement table = p2_html(doc, "table");
    QDomElement tr, th, td, tt;

    // heading
    tr = p2_html(doc, "tr");

    foreach(const QString& heading, headings) {
        th = p2_html(doc, "th");
        th.setAttribute(style, QString("%1 %2").arg(style_left).arg(style_nowrap));
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, heading));
        th.appendChild(tt);
        tr.appendChild(th);
    }

    table.appendChild(tr);

    // table rows
    for (int i = 0; i < words.count(); i++) {
        const P2Word& word = words[i];

        tr = p2_html(doc, "tr");
        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, Token.enum_name(word.tok())));
        td.appendChild(tt);
        tr.appendChild(td);

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, Token.type_names(word.tok()).join(QChar::Space)));
        td.appendChild(tt);
        tr.appendChild(td);

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, QString("@%1").arg(word.pos())));
        td.appendChild(tt);
        tr.appendChild(td);

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, QString("+%1").arg(word.len())));
        td.appendChild(tt);
        tr.appendChild(td);

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, word.str()));
        td.appendChild(tt);
        tr.appendChild(td);

        table.appendChild(tr);
    }
    doc.appendChild(table);
    return doc.toString(1);
}

QString P2AsmModel::symbolsToolTip(const P2SymbolTable& symbols, const QList<P2Symbol>& symrefs) const
{
    if (symbols.isNull())
        return QString();

    QDomDocument doc;

    QDomElement table = p2_html(doc, "table");
    QDomElement tr, th, td, tt;
    QDomText text;

    // heading
    tr = p2_html(doc, "tr");

    th = p2_html(doc, "th");
    th.setAttribute(style, QString("%1 %2").arg(style_left).arg(style_nowrap));
    tt = p2_html(doc, "tt");
    tt.appendChild(p2_text(doc, this->tr("Line #")));
    th.appendChild(tt);
    tr.appendChild(th);

    th = p2_html(doc, "th");
    th.setAttribute(style, QString("%1 %2").arg(style_left).arg(style_nowrap));
    tt = p2_html(doc, "tt");
    tt.appendChild(p2_text(doc, this->tr("Section::name")));
    th.appendChild(tt);
    tr.appendChild(th);

    th = p2_html(doc, "th");
    th.setAttribute(style, QString("%1 %2").arg(style_left).arg(style_nowrap));
    tt = p2_html(doc, "tt");
    tt.appendChild(p2_text(doc, this->tr("Type")));
    th.appendChild(tt);
    tr.appendChild(th);

    th = p2_html(doc, "th");
    th.setAttribute(style, QString("%1 %2").arg(style_left).arg(style_nowrap));
    tt = p2_html(doc, "tt");
    tt.appendChild(p2_text(doc, this->tr("Value")));
    th.appendChild(tt);
    tr.appendChild(th);

    table.appendChild(tr);

    // table rows
    for (int i = 0; i < symrefs.count(); i++) {
        const P2Symbol symbol = symrefs[i];
        const P2Word& word = symbols->reference(symbol->name());
        const P2Union& value = symbol->value();

        tr = p2_html(doc, "tr");

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, QString::number(word.lineno())));
        td.appendChild(tt);
        tr.appendChild(td);

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, symbol->name()));
        td.appendChild(tt);
        tr.appendChild(td);

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, symbol->type_name()));
        td.appendChild(tt);
        tr.appendChild(td);

        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, value.str(false, fmt_hex)));
        td.appendChild(tt);
        tr.appendChild(td);

        table.appendChild(tr);
    }
    doc.appendChild(table);
    return doc.toString(1);
}

QString P2AsmModel::errorsToolTip(const QStringList& list) const
{
    QDomDocument doc;

    QDomElement table = p2_html(doc, "table");
    QDomElement tr, th, td, tt;

    // heading
    tr = p2_html(doc, "tr");
    th = p2_html(doc, "th");
    th.setAttribute(style, QString("%1 %2").arg(style_left).arg(style_nowrap));
    tt = p2_html(doc, "tt");
    tt.appendChild(p2_text(doc, this->tr("Error message")));
    th.appendChild(tt);
    tr.appendChild(th);
    table.appendChild(tr);

    foreach (const QString& error, list) {
        tr = p2_html(doc, "tr");
        td = p2_html(doc, "td");
        td.setAttribute(style, style_nowrap);
        tt = p2_html(doc, "tt");
        tt.appendChild(p2_text(doc, error));
        td.appendChild(tt);
        tr.appendChild(td);
        table.appendChild(tr);
    }
    doc.appendChild(table);
    return doc.toString(1);
}

#include "p2symbolsmodel.h"

P2SymbolsModel::P2SymbolsModel(const P2SymbolTable& table, QObject *parent)
    : QAbstractTableModel(parent)
    , m_table(table)
    , m_font()
    , m_names()
    , m_header()
    , m_background()
    , m_header_alignment()
    , m_text_alignment()
{

    // Header section names
    m_header.insert(c_Name,         tr("Name"));
    m_header.insert(c_Definition,   tr("Line #"));
    m_header.insert(c_References,   tr("Ref(s)"));
    m_header.insert(c_Type,         tr("Type"));
    m_header.insert(c_Value,        tr("Value"));
    // Horizontal section background colors

    m_background.insert(c_Name,         qRgb(0xff,0xfc,0xf8));
    m_background.insert(c_Definition,   qRgb(0xff,0xf0,0xe0));
    m_background.insert(c_References,   qRgb(0xf8,0xfc,0xff));
    m_background.insert(c_Type,         qRgb(0x10,0xfc,0xff));
    m_background.insert(c_Value,        qRgb(0xff,0xf0,0xff));

    // Horizontal section alignments
    m_header_alignment.insert(c_Name,       Qt::AlignLeft | Qt::AlignVCenter);
    m_header_alignment.insert(c_Definition, Qt::AlignRight | Qt::AlignVCenter);
    m_header_alignment.insert(c_References, Qt::AlignRight | Qt::AlignVCenter);
    m_header_alignment.insert(c_Type,       Qt::AlignLeft | Qt::AlignVCenter);
    m_header_alignment.insert(c_Value,      Qt::AlignLeft | Qt::AlignVCenter);

    // Item alignments
    m_text_alignment.insert(c_Name,         Qt::AlignLeft | Qt::AlignVCenter);
    m_text_alignment.insert(c_Definition,   Qt::AlignRight | Qt::AlignVCenter);
    m_text_alignment.insert(c_References,   Qt::AlignRight | Qt::AlignVCenter);
    m_text_alignment.insert(c_Type,         Qt::AlignCenter);
    m_text_alignment.insert(c_Value,        Qt::AlignLeft | Qt::AlignVCenter);

    if (!m_table.isNull())
        m_names = m_table->names();

}

QVariant P2SymbolsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;
    QFont font;
    const column_e column = static_cast<column_e>(section);
    font.setBold(true);
    QFontMetrics metrics(font);
    const int row = section;

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
            result = m_header_alignment.value(column);
            break;

        case Qt::ToolTipRole:
            switch (column) {
            case c_Name:
                result = tr("Name of this symbol.");
                break;
            case c_Definition:
                result = tr("Definition is in source line number.");
                break;
            case c_References:
                result = tr("Referenced in source line number(s).");
                break;
            case c_Type:
                result = tr("Type of the symbol's data.");
                break;
            case c_Value:
                result = tr("Value of the symbol's data.");
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
    const QString& name = m_names.at(row);
    const int defined_in = m_table->defined_where(name);
    const QList<int> references = m_table->references(name);
    const P2Symbol& symbol = m_table->symbol(name);
    const P2Atom& atom = symbol.value<P2Atom>();
    const P2Atom::Type type = atom.type();

    QVariantList refs;
    foreach (int ref, references)
        refs += QString::number(ref);

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Name:
            result = name;
            break;

        case c_Definition:
            result = QString::number(defined_in);
            break;

        case c_References: // Opcode string
            result = refs.value(0);
            break;

        case c_Type:
            result = symbol.type_name();
            break;

        case c_Value:
            switch (type) {
            case P2Atom::Invalid:
                break;
            case P2Atom::Bool:
                result = atom.to_bool() ? QStringLiteral("true")
                                        : QStringLiteral("false");
                break;
            case P2Atom::Byte:
                result = QString("$%1").arg(atom.to_byte(), 2, 16, QChar('0'));
                break;
            case P2Atom::Word:
                result = QString("$%1").arg(atom.to_word(), 4, 16, QChar('0'));
                break;
            case P2Atom::PC:
                result = QString("$%1").arg(atom.to_word(), 8, 16, QChar('0'));
                break;
            case P2Atom::Long:
                result = QString("$%1").arg(atom.to_word(), 8, 16, QChar('0'));
                break;
            case P2Atom::Quad:
                result = QString("$%1").arg(atom.to_word(), 16, 16, QChar('0'));
                break;
            case P2Atom::String:
                result = atom.to_string();
                break;
            }
            break;
        }
        break;

    case Qt::DecorationRole:
        break;

    case Qt::EditRole:
        switch (column) {
        case c_Name:
            result = name;
            break;

        case c_Definition:
            result = defined_in;
            break;

        case c_References:
            if (index.parent().isValid()) {
                result = refs[row];
            } else {
                result = refs;
            }
            break;

        case c_Type:
            result = type;
            break;

        case c_Value:
            result = atom.to_long();
            break;
        }
        break;

    case Qt::ToolTipRole:
        switch (column) {
        case c_Name:
            result = tr("This column shows the %1").arg(tr("name of the symbol."));
            break;

        case c_Definition:
            result = tr("This column shows the %1").arg(tr("source line where the symbol is defined."));
            break;

        case c_References:
            result = tr("This column shows the %1").arg(tr("source line numbers where the symbol is referenced."));
            break;

        case c_Type:
            result = tr("This column shows the %1").arg(tr("type of the symbol's value."));
            break;

        case c_Value:
            result = tr("This column shows the %1").arg(tr("symbol's value."));
            break;
        }
        break;

    case Qt::StatusTipRole:
        break;

    case Qt::WhatsThisRole:
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

    case Qt::ForegroundRole:
        break;

    case Qt::CheckStateRole:
        break;

    case Qt::AccessibleTextRole:
        break;

    case Qt::AccessibleDescriptionRole:
        break;

    case Qt::SizeHintRole:
        result = sizeHint(column, false, data(index, Qt::DisplayRole).toString());
        break;

    case Qt::InitialSortOrderRole:
        break;

    case Qt::UserRole:
        break;
    }
    return result;
}

QList<P2SymbolsModel::column_e> P2SymbolsModel::columns()
{
    QList<column_e> columns;
    columns += c_Name;
    columns += c_Definition;
    columns += c_References;
    columns += c_Type;
    columns += c_Value;
    return columns;

}

static const QString template_str_name = QStringLiteral("CON::SOME_REALLY_LONG_NAME");
static const QString template_str_definition = QStringLiteral("9999999 ");
static const QString template_str_references = QStringLiteral("[9999999*] ");
static const QString template_str_type = QStringLiteral("Invalid ");
static const QString template_str_value = QStringLiteral("$0123456789abcdef ");

QSize P2SymbolsModel::sizeHint(P2SymbolsModel::column_e column, bool header, const QString& text) const
{
    QFont font(m_font);
    font.setBold(header);
    QFontMetrics metrics(font);

    switch (column) {
    case c_Name:
        return metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_name : text);

    case c_Definition:
        return metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_definition : text);

    case c_References:
        return metrics.size(0, text.isEmpty() ? template_str_references : text);

    case c_Type:
        return metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_type : text);

    case c_Value:
        return metrics.size(Qt::TextSingleLine, text.isEmpty() ? template_str_tokens : text);
    }
    return QSize();
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

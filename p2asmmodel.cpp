#include "p2asm.h"
#include "p2asmmodel.h"

static const QString str_address = QStringLiteral("COG[000] ");
static const QString str_tokens = QStringLiteral("T");
static const QString str_opcode_binary = QStringLiteral("EEEE_OOOOOOO_CZI_DDDDDDDDD_SSSSSSSSS ");
static const QString str_opcode_hexdec = QStringLiteral("FFFFFFFF ");
static const QString str_opcode_octal = QStringLiteral("37777777777 ");
static const QString str_errors = QStringLiteral("E");
static const QString str_instruction = QStringLiteral(" IF_NC_AND_NZ  INSTRUCTION #$1ff,#$1ff,#7 XORCZ AND SOME MORE COLUMNS ");

P2AsmModel::P2AsmModel(P2Params* params, QObject *parent)
    : QAbstractTableModel(parent)
    , m_params(params)
    , m_opcode_format(f_bin)
    , m_font(QStringLiteral("Monospace"), 8, QFont::Normal, false)
    , m_bold(QStringLiteral("Monospace"), 8, QFont::Bold, false)
    , m_error()
    , m_size_normal()
    , m_size_bold()
{
    QImage image(":/icons/error.png");
    QPixmap pixmap = QPixmap::fromImage(image.scaled(16,16,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_error = QIcon(pixmap);
    // Header section names
    m_header.insert(c_Origin,       tr("Origin"));
    m_header.insert(c_Tokens,       tr("Tokens"));
    m_header.insert(c_Errors,       tr("Errors"));
    m_header.insert(c_Opcode,       tr("Opcode"));
    m_header.insert(c_Source,       tr("Source"));

    // Horizontal section alignments
    m_alignment.insert(c_Origin,    Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Tokens,    Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Errors,    Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Opcode,    Qt::AlignLeft | Qt::AlignVCenter);
    m_alignment.insert(c_Source,    Qt::AlignLeft | Qt::AlignVCenter);

    // Horizontal section background colors
    m_background.insert(c_Origin,   qRgb(0xff,0xfc,0xf8));
    m_background.insert(c_Tokens,   qRgb(0xc0,0xc0,0xff));
    m_background.insert(c_Errors,   qRgb(0xff,0xf0,0xf0));
    m_background.insert(c_Opcode,   qRgb(0xf8,0xfc,0xff));
    m_background.insert(c_Source,   qRgb(0xff,0xff,0xff));

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
            result = m_alignment.value(column);
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

    const int rows = m_params->source.count();
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

    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case c_Origin: // Address as COG[xxx], LUT[xxx], or xxxxxx in RAM
            if (m_params->h_PC.contains(lineno)) {
                const P2LONG PC = m_params->h_PC.value(lineno);
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
                P2Tokens tokens = m_params->h_tokens.value(lineno);
                if (!tokens.isEmpty())
                    result = QString::number(tokens.count());
            }
            break;

        case c_Errors:  // Error messages
            {
                if (m_params->h_errors.contains(lineno))
                    result = m_error;
            }
            break;

        case c_Opcode: // Opcode string
            if (m_params->h_PC.contains(lineno)) {
                const p2_opcode_u IR = m_params->h_IR.value(lineno);
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
            } else if (m_params->h_IR.contains(lineno)) {
                const p2_opcode_u IR = m_params->h_IR.value(lineno);
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
            result = m_params->source.value(row);
            break;
        }
        break;

    case Qt::DecorationRole:
        switch (column) {
        case c_Errors:
            if (m_params->error.contains(lineno))
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
                P2Tokens tokens = m_params->h_tokens.value(lineno);
                QStringList tt;
                foreach(const p2_token_e tok, tokens)
                    if (tok != t_invalid)
                        tt += Token.string(tok);
                if (tt.count() > 1)
                    result = tr("Tokens: %1").arg(tt.join(QChar::Space));
                else if (tt.count() > 0)
                    result = tr("Token: %1").arg(tt.join(QChar::Space));
            }
            break;
        case c_Errors:
            {
                QStringList errors = m_params->h_errors.values(lineno);
                if (!errors.isEmpty()) {
                    QStringList html;
                    html += QStringLiteral("<html>");
                    html += QString("<table style=\"%1\">")
                            .arg(QStringLiteral("background:#fff0f0;"));
                    foreach (const QString& error, errors) {
                        html += QStringLiteral("<tr>");
                        html += QString("<td style=\"%1\">%2</td>").arg("white-space:nowrap;").arg(error);
                        html += QStringLiteral("</tr>");
                    }
                    html += QStringLiteral("</table>");
                    html += QStringLiteral("</html>");
                    result = html.join(QChar::LineFeed);
                }
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
        result = m_alignment.value(column);
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

void P2AsmModel::setOpcodeFormat(P2AsmModel::format_e format)
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

    m_size_normal.insert(c_Source, metrics_n.size(Qt::TextSingleLine, str_instruction));
    m_size_bold.insert(c_Source,   metrics_b.size(Qt::TextSingleLine, str_instruction));
}

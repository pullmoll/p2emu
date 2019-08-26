#include "p2asmsourcedelegate.h"
#include "p2asmmodel.h"
#include <QPainter>

P2AsmSourceDelegate::P2AsmSourceDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void P2AsmSourceDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const P2AsmModel* model = qobject_cast<const P2AsmModel*>(index.model());
    QVariant v_words = model->data(index, Qt::UserRole);
    const P2Words words = qvariant_cast<P2Words>(v_words);

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const QString line = model->data(index).toString();
    const int ll = line.length() ? line.length() : 1;

    QRect rect = option.rect;
    QVector<QRect> br(ll);
    const int flags = static_cast<int>(opt.displayAlignment) |
                      Qt::TextSingleLine | Qt::TextDontClip | Qt::TextExpandTabs | Qt::TextForceLeftToRight;
    const int w = opt.fontMetrics.size(flags, line).width();
    const int x = rect.x();
    int pos, len;


    painter->save();
    painter->setClipRect(rect);

    const bool highlight = opt.state & QStyle::State_HasFocus ? true : false;

    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setFont(opt.font);
    painter->setPen(p2_palette(color_source, highlight));

    // paint all text character wise to collect the bounding rects
    pos = 0;
    foreach(const QChar ch, line) {
        rect.setX(x + pos * w / ll);
        painter->drawText(rect, flags, ch, &br[pos]);
        pos++;
    }

    // re-draw tokenized words
    foreach(const P2Word& word, words) {
        pos = word.pos();
        len = word.len();
        const QString text = line.mid(pos, len);

        QPalette pal;

        painter->setPen(p2_palette(color_source, highlight));

        p2_token_e tok = word.tok();
        switch (tok) {
        case t_comment:
            painter->setPen(p2_palette(color_comment, highlight));
            break;

        case t_comma:
            painter->setPen(p2_palette(color_comma, highlight));
            break;

        case t_string:
            painter->setPen(p2_palette(color_string, highlight));
            break;

        case t_bin_const:
            painter->setPen(p2_palette(color_bin_const, highlight));
            break;

        case t_byt_const:
            painter->setPen(p2_palette(color_byt_const, highlight));
            break;

        case t_oct_const:
            painter->setPen(p2_palette(color_oct_const, highlight));
            break;

        case t_dec_const:
            painter->setPen(p2_palette(color_dec_const, highlight));
            break;

        case t_hex_const:
            painter->setPen(p2_palette(color_hex_const, highlight));
            break;

        case t_locsym:
            painter->setPen(p2_palette(color_locsym, highlight));
            break;

        case t_symbol:
            painter->setPen(p2_palette(color_symbol, highlight));
            break;

        default:
            if (Token.is_type(tok, tm_section))
                painter->setPen(p2_palette(color_section, highlight));
            if (Token.is_type(tok, tm_conditional))
                painter->setPen(p2_palette(color_conditional, highlight));
            if (Token.is_type(tok, tm_inst))
                painter->setPen(p2_palette(color_instruction, highlight));
            if (Token.is_type(tok, tm_wcz_suffix))
                painter->setPen(p2_palette(color_wcz_suffix, highlight));
            if (Token.is_type(tok, tm_expression))
                painter->setPen(p2_palette(color_expression, highlight));
            break;
        }

        // draw the character
        foreach(const QChar ch, text)
            painter->drawText(br[pos++], flags, ch);
    }
    if (highlight) {
        painter->setBackgroundMode(Qt::OpaqueMode);
        QPen pen(QColor(0x00,0x30,0x30));
        painter->setPen(pen);
        qreal size = pen.width();
        QRectF rect = QRectF(opt.rect).adjusted(size,size,2*size,2*size);

        painter->setOpacity(0.1);
        painter->fillRect(rect, QColor(0x00,0xcf,0xef));

        painter->setOpacity(0.5);
        painter->drawRect(rect);
    }

    painter->restore();
}

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
    if (line.isEmpty())
        return;
    const int ll = line.length();

    QRect rect = opt.rect;
    QVector<QRect> br(ll);
    const int flags = Qt::AlignLeft | Qt::AlignVCenter |
                      Qt::TextSingleLine | Qt::TextDontClip | Qt::TextExpandTabs | Qt::TextForceLeftToRight;
    const int w = opt.fontMetrics.size(flags, line).width();
    const int x = rect.x();
    int pos, len;

    painter->save();

    // painter->setClipRect(rect);
    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setFont(opt.font);
    painter->setPen(p2_palette(color_source));

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

        painter->setPen(p2_palette(color_source));

        p2_token_e tok = word.tok();
        switch (tok) {
        case t_comment:
            painter->setPen(p2_palette(color_comment));
            break;

        case t_comma:
            painter->setPen(p2_palette(color_comma));
            break;

        case t_string:
            painter->setPen(p2_palette(color_string));
            break;

        case t_bin_const:
            painter->setPen(p2_palette(color_bin_const));
            break;

        case t_byt_const:
            painter->setPen(p2_palette(color_byt_const));
            break;

        case t_oct_const:
            painter->setPen(p2_palette(color_oct_const));
            break;

        case t_dec_const:
            painter->setPen(p2_palette(color_dec_const));
            break;

        case t_hex_const:
            painter->setPen(p2_palette(color_hex_const));
            break;

        case t_locsym:
            painter->setPen(p2_palette(color_locsym));
            break;

        case t_symbol:
            painter->setPen(p2_palette(color_symbol));
            break;

        default:
            if (Token.is_type(tok, tt_section))
                painter->setPen(p2_palette(color_section));
            if (Token.is_type(tok, tt_conditional))
                painter->setPen(p2_palette(color_conditional));
            if (Token.is_type(tok, tt_inst))
                painter->setPen(p2_palette(color_instruction));
            if (Token.is_type(tok, tt_wcz_suffix))
                painter->setPen(p2_palette(color_wcz_suffix));
            if (Token.is_type(tok, tm_expression))
                painter->setPen(p2_palette(color_expression));
            break;
        }

        // draw the character
        foreach(const QChar ch, text)
            painter->drawText(br[pos++], flags, ch);
    }
    painter->restore();
}

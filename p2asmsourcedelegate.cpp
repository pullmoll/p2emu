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
    const P2AsmWords words = qvariant_cast<P2AsmWords>(v_words);

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
    painter->setPen(QColor(0x00,0x00,0x00));

    // paint all text character wise to collect the bounding rects
    pos = 0;
    foreach(const QChar ch, line) {
        rect.setX(x + pos * w / ll);
        painter->drawText(rect, flags, ch, &br[pos]);
        pos++;
    }

    // re-draw tokenized words
    foreach(const P2AsmWord& word, words) {
        pos = word.pos();
        len = word.len();
        const QString text = line.mid(pos, len);

        QPalette pal;

        painter->setPen(QColor(0x00,0x00,0x00));

        p2_token_e tok = word.tok();
        switch (tok) {
        case t_comma:
            painter->setPen(QColor(0x00,0xe0,0xff));
            break;

        case t_string:
            painter->setPen(QColor(0x00,0xe0,0xff));
            break;

        case t_bin_const:
        case t_byt_const:
        case t_oct_const:
        case t_dec_const:
        case t_hex_const:
            painter->setPen(QColor(0x00,0x00,0xff));
            break;

        case t_locsym:
            painter->setPen(QColor(0xff,0x80,0xe0));
            break;

        case t_symbol:
            painter->setPen(QColor(0xff,0xc0,0x20));
            break;

        case t_expression:
            painter->setPen(QColor(0xff,0xc0,0x20));
            break;

        default:
            if (Token.is_type(tok, tt_conditional))
                painter->setPen(QColor(0x40,0xa0,0xaf));
            if (Token.is_type(tok, tt_inst))
                painter->setPen(QColor(0x00,0x80,0x8f));
            if (Token.is_type(tok, tt_wcz_suffix))
                painter->setPen(QColor(0xa0,0x40,0xaf));
            break;
        }

        // draw the character
        foreach(const QChar ch, text)
            painter->drawText(br[pos++], flags, ch);
    }
    painter->restore();
}

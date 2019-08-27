#include "p2opcodedelegate.h"
#include "p2asmmodel.h"
#include <QPainter>

P2OpcodeDelegate::P2OpcodeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

void P2OpcodeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const P2AsmModel* model = qobject_cast<const P2AsmModel*>(index.model());
    Q_ASSERT(model);
    QVariant v_words = model->data(index, Qt::UserRole);
    const P2Words words = qvariant_cast<P2Words>(v_words);

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    p2_opcode_format_e format = model->opcode_format();
    const p2_opcode_u IR = qvariant_cast<p2_opcode_u>(model->data(index, Qt::EditRole));
    const QString opcode = model->data(index).toString();

    QRect rect = option.rect;
    const int flags = static_cast<int>(opt.displayAlignment) |
                      Qt::TextSingleLine | Qt::TextDontClip | Qt::TextExpandTabs | Qt::TextForceLeftToRight;

    painter->save();
    painter->setClipRect(rect);

    // fill the background
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->fillRect(opt.rect, opt.backgroundBrush);

    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setFont(opt.font);
    const bool highlight = opt.state & QStyle::State_HasFocus ? true : false;
    painter->setPen(p2_palette(color_source, highlight));

    opt.text = opcode;
    painter->drawText(rect, flags, opt.text);

    if (highlight) {
        painter->setBackgroundMode(Qt::OpaqueMode);
        QPen pen(QColor(0x00,0x30,0x30));
        painter->setPen(pen);
        qreal size = pen.width();
        QRectF rect = QRectF(opt.rect).adjusted(size,size,3*size,3*size);

        painter->setOpacity(0.1);
        painter->fillRect(rect, QColor(0x00,0xcf,0xef));

        painter->setOpacity(0.5);
        painter->drawRect(rect);
    }

    painter->restore();

}
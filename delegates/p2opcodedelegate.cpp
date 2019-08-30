#include <QPainter>
#include <QComboBox>
#include "p2opcodedelegate.h"
#include "p2asmmodel.h"

P2OpcodeDelegate::P2OpcodeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void P2OpcodeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const P2AsmModel* model = qobject_cast<const P2AsmModel*>(index.model());
    Q_ASSERT(model);    // assert the model is really P2AsmModel

    QVariant var = model->data(index, Qt::EditRole);
    if (var.isNull())
        return;

    const P2Opcode IR = qvariant_cast<P2Opcode>(var);
    p2_opcode_format_e format = model->opcode_format();
    QString text;

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    if (IR.as_IR)
        text = format_opcode(IR.u, format);

    if (IR.as_EQU)
        text = format_data(IR.EQU.to_long(), format);

    if (text.isEmpty() && !IR.DATA.isEmpty()) {
        const QStringList& lines = P2Atom::format_data(IR.DATA, IR.PC_ORGH.first);
        text = lines.value(0);
        if (lines.count() > 1)
            text += QStringLiteral("…");
    }

    QRect rect = option.rect;
    const int flags = static_cast<int>(opt.displayAlignment) |
                      Qt::TextDontClip | Qt::TextExpandTabs | Qt::TextForceLeftToRight;

    painter->save();
    painter->setClipRect(rect);

    // fill the background
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->fillRect(opt.rect, opt.backgroundBrush);

    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setFont(opt.font);
    const bool highlight = opt.state & QStyle::State_HasFocus ? true : false;
    painter->setPen(p2_palette(color_source, highlight));

    opt.text = text;
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

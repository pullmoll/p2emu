/****************************************************************************
 *
 * Propeller2 assembler opcode column delegate
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
#include <QPainter>
#include <QComboBox>
#include "p2colors.h"
#include "p2opcodedelegate.h"
#include "p2asmmodel.h"

P2OpcodeDelegate::P2OpcodeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QStringList P2OpcodeDelegate::opcodeLines(const QModelIndex& index) const
{
    const QAbstractItemModel* model = index.model();
    QVariant data = model->data(index, Qt::EditRole);

    const P2Opcode IR = qvariant_cast<P2Opcode>(data);
    p2_FORMAT_e format = qobject_cast<const P2AsmModel *>(model)
                         ? qobject_cast<const P2AsmModel *>(model)->opcode_format()
                         : fmt_hex;
    QStringList text;

    if (IR.is_instruction())
        text += P2Opcode::format_opcode(IR, format);

    if (IR.is_assign())
        text += P2Opcode::format_assign(IR, format);

    if (IR.is_data()) {
        text += P2Opcode::format_data(IR, format);
    }
    return text;
}

void P2OpcodeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const QStringList& lines = opcodeLines(index);
    QString text = lines.value(0);
    if (lines.count() > 1)
        text += QStringLiteral("…");

    QRect rect = option.rect;
    const int flags = static_cast<int>(opt.displayAlignment) |
                      Qt::TextDontClip | Qt::TextExpandTabs | Qt::TextForceLeftToRight;
    const bool focus = opt.state.testFlag(QStyle::State_HasFocus);

    painter->save();
    painter->setClipRect(rect);

    // fill the background
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->fillRect(opt.rect, opt.backgroundBrush);

    QPalette pal = Colors.palette(P2Colors::p2_pal_source);
    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setFont(opt.font);
    painter->setPen(pal.windowText().color());

    painter->drawText(rect, flags, text);

    if (focus) {
        const QRect box = opt.rect;
        QColor bgd = Colors.color("Light Blue");
        bgd.setAlpha(50);
        const QColor tl = bgd.lighter(140);
        const QColor br = bgd.darker(140);
        // painter->setBackgroundMode(Qt::OpaqueMode);
        painter->fillRect(box, bgd);
        painter->setPen(tl);
        painter->drawLine(box.bottomLeft(), box.topLeft());
        painter->drawLine(box.topLeft(), box.topRight());
        painter->setPen(br);
        painter->drawLine(box.bottomLeft(), box.bottomRight());
        painter->drawLine(box.bottomRight(), box.topRight());
    }

    painter->restore();
}

QSize P2OpcodeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStringList lines = opcodeLines(index);
    const int flags = static_cast<int>(option.displayAlignment) |
                      Qt::TextDontClip | Qt::TextExpandTabs | Qt::TextForceLeftToRight;
    QFontMetrics metrics(option.font);
    return metrics.boundingRect(option.rect, flags, lines.join(QChar::LineFeed)).size();
}

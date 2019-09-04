/****************************************************************************
 *
 * Propeller2 assembler source code delegate
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
#include "p2sourcedelegate.h"
#include "p2asmmodel.h"
#include "p2colors.h"
#include <QPainter>

P2SourceDelegate::P2SourceDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void P2SourceDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const P2AsmModel* model = qobject_cast<const P2AsmModel*>(index.model());
    Q_ASSERT(model);
    QVariant v_words = model->data(index, Qt::UserRole);
    const P2Words words = qvariant_cast<P2Words>(v_words);
    const P2Word hword = model->highlight(index);

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    opt.showDecorationSelected = true;

    const bool focus = opt.state.testFlag(QStyle::State_HasFocus);
    const QString line = model->data(index).toString();
    const int ll = line.length() ? line.length() : 1;

    QRect rect = option.rect;
    QVector<QRect> bounding(ll);
    const int flags = static_cast<int>(opt.displayAlignment) |
                      Qt::TextSingleLine | Qt::TextDontClip | Qt::TextExpandTabs | Qt::TextForceLeftToRight;
    const int lw = opt.fontMetrics.size(flags, line).width();
    const int x0 = rect.x();


    painter->save();
    painter->setClipRect(rect);

    // fill the background
    painter->setBackgroundMode(Qt::OpaqueMode);
    QBrush brush = opt.backgroundBrush;
    if (focus) {
        QColor color = Colors.color(QStringLiteral("Light Blue"));
        color.setAlpha(80);
        brush.setColor(color.lighter(110));
    }
    painter->fillRect(opt.rect, brush);

    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setFont(opt.font);
    painter->setPen(Qt::transparent);

    // paint all text character wise to collect the bounding rects
    int pos = 0;
    foreach(const QChar ch, line) {
        rect.setX(x0 + pos * lw / ll);
        painter->drawText(rect, flags, ch, &bounding[pos]);
        pos++;
    }

    // redraw tokenized words
    foreach(const P2Word& word, words) {
        const int len = word.len();
        int pos = word.pos();
        const QStringRef ref(&line, pos, len);

        QPalette pal;
        p2_token_e tok = word.tok();
        painter->setPen(Colors.palette_color(tok, focus));

        // draw the character
        QRect box;
        foreach(const QChar ch, ref) {
            QRect br = bounding[pos++];
            painter->drawText(br, flags, ch);
            box = box.united(br);
        }

        if (word == hword) {
            painter->setPen(QColor(0x00,0x00,0xff));
            painter->drawRect(box);
        }
    }

    if (focus) {
        QColor tl = opt.backgroundBrush.color().darker(120);
        QColor br = opt.backgroundBrush.color().darker(110);
        painter->setPen(tl);
        painter->drawLine(opt.rect.bottomLeft(), opt.rect.topLeft());
        painter->drawLine(opt.rect.topLeft(), opt.rect.topRight());
        painter->setPen(br);
        painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
        painter->drawLine(opt.rect.bottomRight(), opt.rect.topRight());
    }

    painter->restore();
}

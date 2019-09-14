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
    const QAbstractItemModel* model = index.model();
    const P2Words words = qvariant_cast<P2Words>(model->data(index, Qt::UserRole));
    const P2Word highlite = qvariant_cast<P2Word>(model->data(index, Qt::UserRole+1));

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    opt.showDecorationSelected = true;

    const bool focus = opt.state.testFlag(QStyle::State_HasFocus);
    const QString line = model->data(index).toString();
    const int ll = line.length() ? line.length() : 1;

    QRect rect = option.rect;
    QVector<QRect> bounding(ll);
    const int flags = static_cast<int>(opt.displayAlignment) |
                      Qt::TextSingleLine |
                      Qt::TextDontClip |
                      Qt::TextExpandTabs |
                      Qt::TextForceLeftToRight;
    const int lw = opt.fontMetrics.size(flags, line).width();
    const int x0 = rect.x();


    painter->save();
    painter->setClipRect(rect);

    // fill the background
    painter->setBackgroundMode(Qt::OpaqueMode);
    QPalette palette = Colors.palette(P2Colors::p2_pal_source);
    palette.setCurrentColorGroup(QPalette::Normal);
    opt.backgroundBrush = palette.base();
    painter->fillRect(opt.rect, opt.backgroundBrush);

    painter->setFont(opt.font);
    painter->setBackgroundMode(Qt::TransparentMode);
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
        const QStringRef ref = word.ref();
        const int len = ref.length();
        int pos = ref.position();
        if (pos + len > ref.string()->length())
            continue;

        // draw the character
        p2_TOKEN_e tok = word.tok();
        palette = Colors.palette(tok);

        if (highlite.isValid() && highlite.pos() == pos) {
            QRect box;
            foreach(const QRect& r, bounding.mid(pos, len))
                box = box.united(r);
            QColor lighter = palette.windowText().color();
            QColor darker = palette.window().color();
            painter->fillRect(box, lighter);
            painter->setPen(darker);
        } else {
            painter->setPen(palette.windowText().color());
        }

        pos = word.pos();
        foreach(const QChar ch, ref) {
            QRect br = bounding[pos++];
            painter->drawText(br, flags, ch);
        }
    }

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

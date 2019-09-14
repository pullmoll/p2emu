/****************************************************************************
 *
 * Propeller2 emulator color palette for source delegate
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
#pragma once
#include <QVariant>
#include <QColor>
#include <QPalette>
#include <QHash>
#include "p2token.h"

class P2Colors
{
public:
    P2Colors();

    enum p2_palette_e {
        p2_pal_background,
        p2_pal_source,
        p2_pal_comment,
        p2_pal_str_const,
        p2_pal_bin_const,
        p2_pal_byt_const,
        p2_pal_dec_const,
        p2_pal_hex_const,
        p2_pal_real_const,
        p2_pal_locsym,
        p2_pal_symbol,
        p2_pal_expression,
        p2_pal_section,
        p2_pal_conditional,
        p2_pal_modcz_param,
        p2_pal_instruction,
        p2_pal_wcz_suffix,
    };

    QStringList color_names(bool sort_by_hue_sat_lum = false) const;
    QColor color(const QString& name) const;
    QColor color_at(int idx) const;
    int color_index(const QString& name) const;
    QString closest(QColor color) const;

    QString palette_name(p2_palette_e id = p2_pal_source) const;
    QStringList palette_names() const;
    p2_palette_e palette_key(const QString& name) const;
    const QHash<p2_palette_e, QRgb>& hash(bool reset_default = false);
    QPalette palette(p2_palette_e pal) const;
    QPalette palette(p2_TOKEN_e tok) const;
    QRgb palette_color(p2_palette_e pal) const;
    QColor palette_color(p2_TOKEN_e tok) const;

    static p2_palette_e pal_for_token(const p2_TOKEN_e tok);

public slots:
    void set_palette_color(p2_palette_e pal, const QRgb& rgba);
    void set_palette_color(p2_palette_e pal, const QString& name);
    void set_palette(const QHash<p2_palette_e, QRgb>& palette_color);

private:
    QHash<QString,QRgb> m_name_color;
    QHash<QRgb,QString> m_color_name;
    QHash<p2_palette_e,QRgb> m_palette;
    QStringList m_color_names_lexicographic;
    QStringList m_color_names_hue_sat_lum;
    QVector<QColor> m_color_values;
    QVector<QColor> m_colors_hue_sat_lum;
    QMap<p2_palette_e,QString> m_palette_names;
    mutable QHash<p2_palette_e,QRgb> m_default_colors;
    void setup_tables();
    void reset_palette();
};

Q_DECLARE_METATYPE(P2Colors::p2_palette_e);

extern P2Colors Colors;

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
#include <QSettings>
#include "p2token.h"

typedef enum {
    p2_col_Alice_Blue,
    p2_col_Antique_White,
    p2_col_Aqua,
    p2_col_Aquamarine,
    p2_col_Azure,
    p2_col_Beige,
    p2_col_Bisque,
    p2_col_Black,
    p2_col_Blanched_Almond,
    p2_col_Blue,
    p2_col_Blue_Violet,
    p2_col_Brown,
    p2_col_Burlywood,
    p2_col_Cadet_Blue,
    p2_col_Chartreuse,
    p2_col_Chocolate,
    p2_col_Coral,
    p2_col_Cornflower_Blue,
    p2_col_Cornsilk,
    p2_col_Crimson,
    p2_col_Cyan,
    p2_col_Dark_Blue,
    p2_col_Dark_Cyan,
    p2_col_Dark_Goldenrod,
    p2_col_Dark_Gray,
    p2_col_Dark_Green,
    p2_col_Dark_Khaki,
    p2_col_Dark_Magenta,
    p2_col_Dark_Olive_Green,
    p2_col_Dark_Orange,
    p2_col_Dark_Orchid,
    p2_col_Dark_Red,
    p2_col_Dark_Salmon,
    p2_col_Dark_Sea_Green,
    p2_col_Dark_Slate_Blue,
    p2_col_Dark_Slate_Gray,
    p2_col_Dark_Turquoise,
    p2_col_Dark_Violet,
    p2_col_Deep_Pink,
    p2_col_Deep_Sky_Blue,
    p2_col_Dim_Gray,
    p2_col_Dodger_Blue,
    p2_col_Firebrick,
    p2_col_Floral_White,
    p2_col_Forest_Green,
    p2_col_Fuchsia,
    p2_col_Gainsboro,
    p2_col_Ghost_White,
    p2_col_Gold,
    p2_col_Goldenrod,
    p2_col_Gray,
    p2_col_Web_Gray,
    p2_col_Green,
    p2_col_Web_Green,
    p2_col_Green_Yellow,
    p2_col_Honeydew,
    p2_col_Hot_Pink,
    p2_col_Indian_Red,
    p2_col_Indigo,
    p2_col_Ivory,
    p2_col_Khaki,
    p2_col_Lavender,
    p2_col_Lavender_Blush,
    p2_col_Lawn_Green,
    p2_col_Lemon_Chiffon,
    p2_col_Light_Blue,
    p2_col_Light_Coral,
    p2_col_Light_Cyan,
    p2_col_Light_Goldenrod,
    p2_col_Light_Gray,
    p2_col_Light_Green,
    p2_col_Light_Pink,
    p2_col_Light_Salmon,
    p2_col_Light_Sea_Green,
    p2_col_Light_Sky_Blue,
    p2_col_Light_Slate_Gray,
    p2_col_Light_Steel_Blue,
    p2_col_Light_Yellow,
    p2_col_Lime,
    p2_col_Lime_Green,
    p2_col_Linen,
    p2_col_Magenta,
    p2_col_Maroon,
    p2_col_Web_Maroon,
    p2_col_Medium_Aquamarine,
    p2_col_Medium_Blue,
    p2_col_Medium_Orchid,
    p2_col_Medium_Purple,
    p2_col_Medium_Sea_Green,
    p2_col_Medium_Slate_Blue,
    p2_col_Medium_Spring_Green,
    p2_col_Medium_Turquoise,
    p2_col_Medium_Violet_Red,
    p2_col_Midnight_Blue,
    p2_col_Mint_Cream,
    p2_col_Misty_Rose,
    p2_col_Moccasin,
    p2_col_Navajo_White,
    p2_col_Navy_Blue,
    p2_col_Old_Lace,
    p2_col_Olive,
    p2_col_Olive_Drab,
    p2_col_Orange,
    p2_col_Orange_Red,
    p2_col_Orchid,
    p2_col_Pale_Goldenrod,
    p2_col_Pale_Green,
    p2_col_Pale_Turquoise,
    p2_col_Pale_Violet_Red,
    p2_col_Papaya_Whip,
    p2_col_Peach_Puff,
    p2_col_Peru,
    p2_col_Pink,
    p2_col_Plum,
    p2_col_Powder_Blue,
    p2_col_Purple,
    p2_col_Web_Purple,
    p2_col_Rebecca_Purple,
    p2_col_Red,
    p2_col_Rosy_Brown,
    p2_col_Royal_Blue,
    p2_col_Saddle_Brown,
    p2_col_Salmon,
    p2_col_Sandy_Brown,
    p2_col_Sea_Green,
    p2_col_Seashell,
    p2_col_Sienna,
    p2_col_Silver,
    p2_col_Sky_Blue,
    p2_col_Slate_Blue,
    p2_col_Slate_Gray,
    p2_col_Snow,
    p2_col_Spring_Green,
    p2_col_Steel_Blue,
    p2_col_Tan,
    p2_col_Teal,
    p2_col_Thistle,
    p2_col_Tomato,
    p2_col_Turquoise,
    p2_col_Violet,
    p2_col_Wheat,
    p2_col_White,
    p2_col_White_Smoke,
    p2_col_Yellow,
    p2_col_Yellow_Green,
}   p2_color_e;
Q_DECLARE_METATYPE(p2_color_e);

typedef enum {
    p2_pal_invalid,
    p2_pal_background = 512,
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
}   p2_palette_e;
Q_DECLARE_METATYPE(p2_palette_e);

typedef QHash<p2_palette_e, p2_color_e> p2_palette_hash_t;

class P2Colors
{
public:
    P2Colors();

    QStringList color_names(bool sort_by_hue_sat_lum = false) const;
    QString color_name(p2_color_e col) const;
    QRgb rgba(const p2_color_e col) const;
    QColor color(const p2_color_e col) const;
    QColor color(const QString& colorname) const;
    QColor color_at(int idx) const;
    p2_color_e color_key(const QString& colorname) const;
    p2_color_e closest(QColor color) const;

    QString palette_name(p2_palette_e id = p2_pal_source) const;
    QStringList palette_names() const;
    p2_palette_e palette_key(const QString& name) const;
    const p2_palette_hash_t& palette_hash(bool reset_default = false);
    QPalette palette(p2_palette_e pal) const;
    QPalette palette(p2_TOKEN_e tok) const;
    QColor palette_color(p2_palette_e pal) const;
    QColor palette_color(p2_TOKEN_e tok) const;

    static p2_palette_e pal_for_token(const p2_TOKEN_e tok);

    void save_palette(QSettings& s) const;
    void restore_palette(QSettings& s);

public slots:
    void set_palette_color(p2_palette_e pal, const p2_color_e col);
    void set_palette_color(p2_palette_e pal, const QRgb rgba);
    void set_palette_color(p2_palette_e pal, const QString name);
    void set_palette(const p2_palette_hash_t& palette_color);

private:
    QHash<p2_color_e,QString> m_color_names;
    QHash<QRgb,p2_color_e> m_color_index;
    QVector<p2_color_e> m_color_lexicographic;
    QVector<p2_color_e> m_color_hue_sat_lum;
    p2_palette_hash_t m_current_palette;
    p2_palette_hash_t m_default_palette;
    void setup_tables();
    void reset_palette();
};

extern P2Colors Colors;

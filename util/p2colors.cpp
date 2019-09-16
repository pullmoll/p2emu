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
#include <QGuiApplication>
#include <QDataStream>
#include <cmath>
#include "p2colors.h"
#include "p2util.h"

static QHash<p2_color_e,QString> g_col_name;
static QHash<p2_color_e,QRgb> g_col_rgba;
static QHash<p2_palette_e,QString> g_pal_name;

P2Colors Colors;

static bool color_compare_lexicographic(const p2_color_e& c1, const p2_color_e& c2)
{
    QString val1 = g_col_name.value(c1);
    QString val2 = g_col_name.value(c2);
    return val1 < val2;
}

static bool color_compare_hue_sat_lum(const p2_color_e& c1, const p2_color_e& c2)
{
    const QRgb rgba1 = g_col_rgba.value(c1);
    const QRgb rgba2 = g_col_rgba.value(c2);
    qreal h1, v1, s1, a1;
    qreal h2, v2, s2, a2;
    QColor(rgba1).getHsvF(&h1, &s1, &v1, &a1);
    QColor(rgba2).getHsvF(&h2, &s2, &v2, &a2);
    const qreal val1 = (1000.0 * h1 + 100.0 * v1 + 10.0 * s1 + (1.0 - a1));
    const qreal val2 = (1000.0 * h2 + 100.0 * v2 + 10.0 * s2 + (1.0 - a2));
    return val1 < val2;
}

P2Colors::P2Colors()
    : m_color_names()
    , m_color_index()
    , m_color_lexicographic()
    , m_color_hue_sat_lum()
    , m_current_palette()
    , m_default_palette()
{
    setup_tables();
    reset_palette();
}

/**
 * @brief Return a list of color names
 * @param sort_by_hue_sat_lum if true, the list returned is sorted by hue, saturation, and luminance
 * @return QStringList with names
 */
QStringList P2Colors::color_names(bool sort_by_hue_sat_lum) const
{
    QStringList list;
    if (sort_by_hue_sat_lum) {
        foreach (const p2_color_e col, m_color_hue_sat_lum)
            list += g_col_name.value(col);
    } else {
        foreach (const p2_color_e col, m_color_lexicographic)
            list += g_col_name.value(col);
    }
    return list;
}

/**
 * @brief Return the technical name of a palette enumeration value %pal
 * @param pal palette index from p2_palette_e
 * @return QString with the technical name
 */
QString P2Colors::color_name(p2_color_e col) const
{
    return g_col_name.value(col);
}

QRgb P2Colors::rgba(const p2_color_e col) const
{
    return g_col_rgba.value(col);
}

/**
 * @brief Return the QColor value for a color %col
 * @param col enumeration value from p2_color_e
 * @return QColor value, or black if unknown
 */
QColor P2Colors::color(const p2_color_e col) const
{
    return g_col_rgba.value(col);
}

/**
 * @brief Return the QColor value for a color name
 * @param col enumeration value from p2_color_e
 * @return QColor value, or black if unknown
 */
QColor P2Colors::color(const QString& colorname) const
{
    return g_col_rgba.value(color_key(colorname));
}

/**
 * @brief Return the index for a color name
 * @param name of the color
 * @return index in the names, or -1 if not found
 */
p2_color_e P2Colors::color_key(const QString& colorname) const
{
    p2_color_e col = g_col_name.key(colorname);
    return col;
}

/**
 * @brief Return the name of a color closest to the QColor
 * @param color some QColor
 * @return QString name of the closes color from our table
 */
p2_color_e P2Colors::closest(QColor color) const
{
    if (m_color_index.contains(color.rgba()))
        return m_color_index[color.rgba()];

    int i = 0;
    p2_color_e bestcol = p2_col_White;
    qreal bestdist = -1.0;
    i = 0;
    foreach(const p2_color_e col, m_color_hue_sat_lum) {
        QColor known(g_col_rgba.value(col));
        const qreal dr = known.redF() - color.redF();
        const qreal dg = known.greenF() - color.greenF();
        const qreal db = known.blueF() - color.blueF();
        const qreal da = known.alphaF() - color.alphaF();
        qreal dist = sqrt(sqrt(sqrt(dr*dr + dg*dg) + db*db) + da*da);
        if (bestdist < 0 || dist < bestdist) {
            bestdist = dist;
            bestcol = col;
        }
        i++;
    }
    return bestcol;
}

/**
 * @brief Return the technical name of a palette enumeration value %pal
 * @param pal palette index from p2_palette_e
 * @return QString with the technical name
 */
QString P2Colors::palette_name(p2_palette_e pal) const
{
    return g_pal_name.value(pal);
}

/**
 * @brief Return a list of all palette names
 * @return QStringList with the names
 */
QStringList P2Colors::palette_names() const
{
    return g_pal_name.values();
}

/**
 * @brief Return the palette enumeration value for a technical name
 * @param name QString as returned by %palette_name(pal)
 * @return palette enumeration value
 */
p2_palette_e P2Colors::palette_key(const QString& name) const
{
    return g_pal_name.key(name, p2_pal_source);
}

/**
 * @brief Return the current palette hash
 * @param reset_default if true, reset to the default palette first
 * @return QHash of palette enumeration values and their QColor
 */
const p2_palette_hash_t& P2Colors::palette_hash(bool reset_default)
{
    if (reset_default)
        reset_palette();
    return m_current_palette;
}

/**
 * @brief Return a modified QPalette for %pal
 * @param pal p2_palette_e value
 * @return QPalette adjusted to color for %pal
 */
QPalette P2Colors::palette(p2_palette_e pal) const
{
    QPalette qpal = qApp->palette();
    const QColor background = palette_color(p2_pal_background);
    const QColor color = palette_color(pal);
    const int gray = qGray(color.rgb());

    qpal.setBrush(QPalette::Base, QBrush(background));
    qpal.setColor(QPalette::Active, QPalette::WindowText, color);
    qpal.setColor(QPalette::Inactive, QPalette::WindowText, color.lighter(150));
    qpal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(gray,gray,gray));
    return qpal;
}

/**
 * @brief Return a modified QPalette for %tok
 * @param tok parser token enumeration value
 * @return QPalette adjusted to color for %tok
 */
QPalette P2Colors::palette(p2_TOKEN_e tok) const
{
    return palette(pal_for_token(tok));
}

/**
 * @brief Return a palette color for the palette enumeration value %pal
 * @param pal palette index from p2_palette_e
 * @return QColor for the index
 */
QColor P2Colors::palette_color(p2_palette_e pal) const
{
    return QColor(g_col_rgba.value(m_current_palette.value(pal)));
}

/**
 * @brief Return a palette color for the specified parser token %tok
 * @param tok parser token enumeration value
 * @param darker if true, make the color darker
 * @return QColor from the palette
 */
QColor P2Colors::palette_color(p2_TOKEN_e tok) const
{
    return palette_color(pal_for_token(tok));
}

/**
 * @brief Set the entire palette to a new hash
 * @param palette const reference to a QHash of palette enumeration values and their QColor
 */
void P2Colors::set_palette(const p2_palette_hash_t& palette)
{
    m_current_palette = palette;
}

/**
 * @brief Set a single palette color
 * @param pal palette index from p2_palette_e
 * @param col color index from p2_color_e
 */
void P2Colors::set_palette_color(p2_palette_e pal, const p2_color_e col)
{
    m_current_palette.insert(pal, col);
}

/**
 * @brief Set a single palette color
 * @param pal palette index from p2_palette_e
 * @param rgba QRgb value to set
 */
void P2Colors::set_palette_color(p2_palette_e pal, const QRgb rgba)
{
    m_current_palette.insert(pal, g_col_rgba.key(rgba));
}

/**
 * @brief Set a single palette color
 * @param pal palette index from p2_palette_e
 * @param name color name
 */
void P2Colors::set_palette_color(p2_palette_e pal, const QString name)
{
    m_current_palette.insert(pal, color_key(name));
}

/**
 * @brief Return a p2_palette_e value for the token %tok
 * @param tok token value to map
 * @return p2_palette_e value to use
 */
p2_palette_e P2Colors::pal_for_token(const p2_TOKEN_e tok)
{
    p2_palette_e pal = p2_pal_source;

    switch (tok) {
    case t_comment_curly:
    case t_comment_eol:
    case t_comment_lcurly:
    case t_comment_rcurly:
        pal = p2_pal_comment;
        break;

    case t_str_const:
        pal = p2_pal_str_const;
        break;

    case t_bin_const:
        pal = p2_pal_bin_const;
        break;

    case t_byt_const:
        pal = p2_pal_byt_const;
        break;

    case t_dec_const:
        pal = p2_pal_dec_const;
        break;

    case t_hex_const:
        pal = p2_pal_hex_const;
        break;

    case t_real_const:
        pal = p2_pal_real_const;
        break;

    case t_locsym:
        pal = p2_pal_locsym;
        break;

    case t_symbol:
        pal = p2_pal_symbol;
        break;

    default:
        if (Token.is_type(tok, tm_section))
            pal = p2_pal_section;

        if (Token.is_type(tok, tm_conditional))
            pal = p2_pal_conditional;

        if (Token.is_type(tok, tm_mnemonic))
            pal = p2_pal_instruction;

        if (Token.is_type(tok, tm_modcz_param))
            pal = p2_pal_modcz_param;

        if (Token.is_type(tok, tm_wcz_suffix))
            pal = p2_pal_wcz_suffix;

        if (Token.is_type(tok, tm_expression))
            pal = p2_pal_expression;
        break;
    }

    return pal;
}

void P2Colors::save_palette(QSettings& s) const
{
    foreach(const p2_palette_e pal, m_current_palette.keys()) {
        p2_color_e col = m_current_palette.value(pal);
        QString pal_name = g_pal_name.value(pal);
        QString col_name = g_col_name.value(col);
        s.setValue(pal_name, col_name);
    }
}

void P2Colors::restore_palette(QSettings& s)
{
    m_current_palette = m_default_palette;
    foreach(const QString& key, s.allKeys()) {
        QString pal_name = key;
        QString col_name = s.value(key).toString();
        p2_palette_e pal = palette_key(pal_name);
        p2_color_e col = color_key(col_name);
        m_current_palette.insert(pal, col);
    }
}

/**
 * @brief Setup and sort the tables
 */
void P2Colors::setup_tables()
{
    g_col_name = {
        {p2_col_Alice_Blue,          QLatin1String("Alice Blue")},
        {p2_col_Antique_White,       QLatin1String("Antique White")},
        {p2_col_Aqua,                QLatin1String("Aqua")},
        {p2_col_Aquamarine,          QLatin1String("Aquamarine")},
        {p2_col_Azure,               QLatin1String("Azure")},
        {p2_col_Beige,               QLatin1String("Beige")},
        {p2_col_Bisque,              QLatin1String("Bisque")},
        {p2_col_Black,               QLatin1String("Black")},
        {p2_col_Blanched_Almond,     QLatin1String("Blanched Almond")},
        {p2_col_Blue,                QLatin1String("Blue")},
        {p2_col_Blue_Violet,         QLatin1String("Blue Violet")},
        {p2_col_Brown,               QLatin1String("Brown")},
        {p2_col_Burlywood,           QLatin1String("Burlywood")},
        {p2_col_Cadet_Blue,          QLatin1String("Cadet Blue")},
        {p2_col_Chartreuse,          QLatin1String("Chartreuse")},
        {p2_col_Chocolate,           QLatin1String("Chocolate")},
        {p2_col_Coral,               QLatin1String("Coral")},
        {p2_col_Cornflower_Blue,     QLatin1String("Cornflower Blue")},
        {p2_col_Cornsilk,            QLatin1String("Cornsilk")},
        {p2_col_Crimson,             QLatin1String("Crimson")},
        {p2_col_Cyan,                QLatin1String("Cyan")},
        {p2_col_Dark_Blue,           QLatin1String("Dark Blue")},
        {p2_col_Dark_Cyan,           QLatin1String("Dark Cyan")},
        {p2_col_Dark_Goldenrod,      QLatin1String("Dark Goldenrod")},
        {p2_col_Dark_Gray,           QLatin1String("Dark Gray")},
        {p2_col_Dark_Green,          QLatin1String("Dark Green")},
        {p2_col_Dark_Khaki,          QLatin1String("Dark Khaki")},
        {p2_col_Dark_Magenta,        QLatin1String("Dark Magenta")},
        {p2_col_Dark_Olive_Green,    QLatin1String("Dark Olive Green")},
        {p2_col_Dark_Orange,         QLatin1String("Dark Orange")},
        {p2_col_Dark_Orchid,         QLatin1String("Dark Orchid")},
        {p2_col_Dark_Red,            QLatin1String("Dark Red")},
        {p2_col_Dark_Salmon,         QLatin1String("Dark Salmon")},
        {p2_col_Dark_Sea_Green,      QLatin1String("Dark Sea Green")},
        {p2_col_Dark_Slate_Blue,     QLatin1String("Dark Slate Blue")},
        {p2_col_Dark_Slate_Gray,     QLatin1String("Dark Slate Gray")},
        {p2_col_Dark_Turquoise,      QLatin1String("Dark Turquoise")},
        {p2_col_Dark_Violet,         QLatin1String("Dark Violet")},
        {p2_col_Deep_Pink,           QLatin1String("Deep Pink")},
        {p2_col_Deep_Sky_Blue,       QLatin1String("Deep Sky Blue")},
        {p2_col_Dim_Gray,            QLatin1String("Dim Gray")},
        {p2_col_Dodger_Blue,         QLatin1String("Dodger Blue")},
        {p2_col_Firebrick,           QLatin1String("Firebrick")},
        {p2_col_Floral_White,        QLatin1String("Floral White")},
        {p2_col_Forest_Green,        QLatin1String("Forest Green")},
        {p2_col_Fuchsia,             QLatin1String("Fuchsia")},
        {p2_col_Gainsboro,           QLatin1String("Gainsboro")},
        {p2_col_Ghost_White,         QLatin1String("Ghost White")},
        {p2_col_Gold,                QLatin1String("Gold")},
        {p2_col_Goldenrod,           QLatin1String("Goldenrod")},
        {p2_col_Gray,                QLatin1String("Gray")},
        {p2_col_Web_Gray,            QLatin1String("Web Gray")},
        {p2_col_Green,               QLatin1String("Green")},
        {p2_col_Web_Green,           QLatin1String("Web Green")},
        {p2_col_Green_Yellow,        QLatin1String("Green Yellow")},
        {p2_col_Honeydew,            QLatin1String("Honeydew")},
        {p2_col_Hot_Pink,            QLatin1String("Hot Pink")},
        {p2_col_Indian_Red,          QLatin1String("Indian Red")},
        {p2_col_Indigo,              QLatin1String("Indigo")},
        {p2_col_Ivory,               QLatin1String("Ivory")},
        {p2_col_Khaki,               QLatin1String("Khaki")},
        {p2_col_Lavender,            QLatin1String("Lavender")},
        {p2_col_Lavender_Blush,      QLatin1String("Lavender Blush")},
        {p2_col_Lawn_Green,          QLatin1String("Lawn Green")},
        {p2_col_Lemon_Chiffon,       QLatin1String("Lemon Chiffon")},
        {p2_col_Light_Blue,          QLatin1String("Light Blue")},
        {p2_col_Light_Coral,         QLatin1String("Light Coral")},
        {p2_col_Light_Cyan,          QLatin1String("Light Cyan")},
        {p2_col_Light_Goldenrod,     QLatin1String("Light Goldenrod")},
        {p2_col_Light_Gray,          QLatin1String("Light Gray")},
        {p2_col_Light_Green,         QLatin1String("Light Green")},
        {p2_col_Light_Pink,          QLatin1String("Light Pink")},
        {p2_col_Light_Salmon,        QLatin1String("Light Salmon")},
        {p2_col_Light_Sea_Green,     QLatin1String("Light Sea Green")},
        {p2_col_Light_Sky_Blue,      QLatin1String("Light Sky Blue")},
        {p2_col_Light_Slate_Gray,    QLatin1String("Light Slate Gray")},
        {p2_col_Light_Steel_Blue,    QLatin1String("Light Steel Blue")},
        {p2_col_Light_Yellow,        QLatin1String("Light Yellow")},
        {p2_col_Lime,                QLatin1String("Lime")},
        {p2_col_Lime_Green,          QLatin1String("Lime Green")},
        {p2_col_Linen,               QLatin1String("Linen")},
        {p2_col_Magenta,             QLatin1String("Magenta")},
        {p2_col_Maroon,              QLatin1String("Maroon")},
        {p2_col_Web_Maroon,          QLatin1String("Web Maroon")},
        {p2_col_Medium_Aquamarine,   QLatin1String("Medium Aquamarine")},
        {p2_col_Medium_Blue,         QLatin1String("Medium Blue")},
        {p2_col_Medium_Orchid,       QLatin1String("Medium Orchid")},
        {p2_col_Medium_Purple,       QLatin1String("Medium Purple")},
        {p2_col_Medium_Sea_Green,    QLatin1String("Medium Sea Green")},
        {p2_col_Medium_Slate_Blue,   QLatin1String("Medium Slate Blue")},
        {p2_col_Medium_Spring_Green, QLatin1String("Medium Spring Green")},
        {p2_col_Medium_Turquoise,    QLatin1String("Medium Turquoise")},
        {p2_col_Medium_Violet_Red,   QLatin1String("Medium Violet Red")},
        {p2_col_Midnight_Blue,       QLatin1String("Midnight Blue")},
        {p2_col_Mint_Cream,          QLatin1String("Mint Cream")},
        {p2_col_Misty_Rose,          QLatin1String("Misty Rose")},
        {p2_col_Moccasin,            QLatin1String("Moccasin")},
        {p2_col_Navajo_White,        QLatin1String("Navajo White")},
        {p2_col_Navy_Blue,           QLatin1String("Navy Blue")},
        {p2_col_Old_Lace,            QLatin1String("Old Lace")},
        {p2_col_Olive,               QLatin1String("Olive")},
        {p2_col_Olive_Drab,          QLatin1String("Olive Drab")},
        {p2_col_Orange,              QLatin1String("Orange")},
        {p2_col_Orange_Red,          QLatin1String("Orange Red")},
        {p2_col_Orchid,              QLatin1String("Orchid")},
        {p2_col_Pale_Goldenrod,      QLatin1String("Pale Goldenrod")},
        {p2_col_Pale_Green,          QLatin1String("Pale Green")},
        {p2_col_Pale_Turquoise,      QLatin1String("Pale Turquoise")},
        {p2_col_Pale_Violet_Red,     QLatin1String("Pale Violet Red")},
        {p2_col_Papaya_Whip,         QLatin1String("Papaya Whip")},
        {p2_col_Peach_Puff,          QLatin1String("Peach Puff")},
        {p2_col_Peru,                QLatin1String("Peru")},
        {p2_col_Pink,                QLatin1String("Pink")},
        {p2_col_Plum,                QLatin1String("Plum")},
        {p2_col_Powder_Blue,         QLatin1String("Powder Blue")},
        {p2_col_Purple,              QLatin1String("Purple")},
        {p2_col_Web_Purple,          QLatin1String("Web Purple")},
        {p2_col_Rebecca_Purple,      QLatin1String("Rebecca Purple")},
        {p2_col_Red,                 QLatin1String("Red")},
        {p2_col_Rosy_Brown,          QLatin1String("Rosy Brown")},
        {p2_col_Royal_Blue,          QLatin1String("Royal Blue")},
        {p2_col_Saddle_Brown,        QLatin1String("Saddle Brown")},
        {p2_col_Salmon,              QLatin1String("Salmon")},
        {p2_col_Sandy_Brown,         QLatin1String("Sandy Brown")},
        {p2_col_Sea_Green,           QLatin1String("Sea Green")},
        {p2_col_Seashell,            QLatin1String("Seashell")},
        {p2_col_Sienna,              QLatin1String("Sienna")},
        {p2_col_Silver,              QLatin1String("Silver")},
        {p2_col_Sky_Blue,            QLatin1String("Sky Blue")},
        {p2_col_Slate_Blue,          QLatin1String("Slate Blue")},
        {p2_col_Slate_Gray,          QLatin1String("Slate Gray")},
        {p2_col_Snow,                QLatin1String("Snow")},
        {p2_col_Spring_Green,        QLatin1String("Spring Green")},
        {p2_col_Steel_Blue,          QLatin1String("Steel Blue")},
        {p2_col_Tan,                 QLatin1String("Tan")},
        {p2_col_Teal,                QLatin1String("Teal")},
        {p2_col_Thistle,             QLatin1String("Thistle")},
        {p2_col_Tomato,              QLatin1String("Tomato")},
        {p2_col_Turquoise,           QLatin1String("Turquoise")},
        {p2_col_Violet,              QLatin1String("Violet")},
        {p2_col_Wheat,               QLatin1String("Wheat")},
        {p2_col_White,               QLatin1String("White")},
        {p2_col_White_Smoke,         QLatin1String("White Smoke")},
        {p2_col_Yellow,              QLatin1String("Yellow")},
        {p2_col_Yellow_Green,        QLatin1String("Yellow Green")},
    };

    g_col_rgba = {
        {p2_col_Alice_Blue,          qRgba(0xF0,0xF8,0xFF,0xFF)},
        {p2_col_Antique_White,       qRgba(0xFA,0xEB,0xD7,0xFF)},
        {p2_col_Aqua,                qRgba(0x00,0xFF,0xFF,0xFF)},
        {p2_col_Aquamarine,          qRgba(0x7F,0xFF,0xD4,0xFF)},
        {p2_col_Azure,               qRgba(0xF0,0xFF,0xFF,0xFF)},
        {p2_col_Beige,               qRgba(0xF5,0xF5,0xDC,0xFF)},
        {p2_col_Bisque,              qRgba(0xFF,0xE4,0xC4,0xFF)},
        {p2_col_Black,               qRgba(0x00,0x00,0x00,0xFF)},
        {p2_col_Blanched_Almond,     qRgba(0xFF,0xEB,0xCD,0xFF)},
        {p2_col_Blue,                qRgba(0x00,0x00,0xFF,0xFF)},
        {p2_col_Blue_Violet,         qRgba(0x8A,0x2B,0xE2,0xFF)},
        {p2_col_Brown,               qRgba(0xA5,0x2A,0x2A,0xFF)},
        {p2_col_Burlywood,           qRgba(0xDE,0xB8,0x87,0xFF)},
        {p2_col_Cadet_Blue,          qRgba(0x5F,0x9E,0xA0,0xFF)},
        {p2_col_Chartreuse,          qRgba(0x7F,0xFF,0x00,0xFF)},
        {p2_col_Chocolate,           qRgba(0xD2,0x69,0x1E,0xFF)},
        {p2_col_Coral,               qRgba(0xFF,0x7F,0x50,0xFF)},
        {p2_col_Cornflower_Blue,     qRgba(0x64,0x95,0xED,0xFF)},
        {p2_col_Cornsilk,            qRgba(0xFF,0xF8,0xDC,0xFF)},
        {p2_col_Crimson,             qRgba(0xDC,0x14,0x3C,0xFF)},
        {p2_col_Cyan,                qRgba(0x00,0xFF,0xFF,0xFF)},
        {p2_col_Dark_Blue,           qRgba(0x00,0x00,0x8B,0xFF)},
        {p2_col_Dark_Cyan,           qRgba(0x00,0x8B,0x8B,0xFF)},
        {p2_col_Dark_Goldenrod,      qRgba(0xB8,0x86,0x0B,0xFF)},
        {p2_col_Dark_Gray,           qRgba(0xA9,0xA9,0xA9,0xFF)},
        {p2_col_Dark_Green,          qRgba(0x00,0x64,0x00,0xFF)},
        {p2_col_Dark_Khaki,          qRgba(0xBD,0xB7,0x6B,0xFF)},
        {p2_col_Dark_Magenta,        qRgba(0x8B,0x00,0x8B,0xFF)},
        {p2_col_Dark_Olive_Green,    qRgba(0x55,0x6B,0x2F,0xFF)},
        {p2_col_Dark_Orange,         qRgba(0xFF,0x8C,0x00,0xFF)},
        {p2_col_Dark_Orchid,         qRgba(0x99,0x32,0xCC,0xFF)},
        {p2_col_Dark_Red,            qRgba(0x8B,0x00,0x00,0xFF)},
        {p2_col_Dark_Salmon,         qRgba(0xE9,0x96,0x7A,0xFF)},
        {p2_col_Dark_Sea_Green,      qRgba(0x8F,0xBC,0x8F,0xFF)},
        {p2_col_Dark_Slate_Blue,     qRgba(0x48,0x3D,0x8B,0xFF)},
        {p2_col_Dark_Slate_Gray,     qRgba(0x2F,0x4F,0x4F,0xFF)},
        {p2_col_Dark_Turquoise,      qRgba(0x00,0xCE,0xD1,0xFF)},
        {p2_col_Dark_Violet,         qRgba(0x94,0x00,0xD3,0xFF)},
        {p2_col_Deep_Pink,           qRgba(0xFF,0x14,0x93,0xFF)},
        {p2_col_Deep_Sky_Blue,       qRgba(0x00,0xBF,0xFF,0xFF)},
        {p2_col_Dim_Gray,            qRgba(0x69,0x69,0x69,0xFF)},
        {p2_col_Dodger_Blue,         qRgba(0x1E,0x90,0xFF,0xFF)},
        {p2_col_Firebrick,           qRgba(0xB2,0x22,0x22,0xFF)},
        {p2_col_Floral_White,        qRgba(0xFF,0xFA,0xF0,0xFF)},
        {p2_col_Forest_Green,        qRgba(0x22,0x8B,0x22,0xFF)},
        {p2_col_Fuchsia,             qRgba(0xFF,0x01,0xFF,0xFF)},
        {p2_col_Gainsboro,           qRgba(0xDC,0xDC,0xDC,0xFF)},
        {p2_col_Ghost_White,         qRgba(0xF8,0xF8,0xFF,0xFF)},
        {p2_col_Gold,                qRgba(0xFF,0xD7,0x00,0xFF)},
        {p2_col_Goldenrod,           qRgba(0xDA,0xA5,0x20,0xFF)},
        {p2_col_Gray,                qRgba(0xBE,0xBE,0xBE,0xFF)},
        {p2_col_Web_Gray,            qRgba(0x80,0x80,0x80,0xFF)},
        {p2_col_Green,               qRgba(0x00,0xFF,0x00,0xFF)},
        {p2_col_Web_Green,           qRgba(0x00,0x80,0x00,0xFF)},
        {p2_col_Green_Yellow,        qRgba(0xAD,0xFF,0x2F,0xFF)},
        {p2_col_Honeydew,            qRgba(0xF0,0xFF,0xF0,0xFF)},
        {p2_col_Hot_Pink,            qRgba(0xFF,0x69,0xB4,0xFF)},
        {p2_col_Indian_Red,          qRgba(0xCD,0x5C,0x5C,0xFF)},
        {p2_col_Indigo,              qRgba(0x4B,0x00,0x82,0xFF)},
        {p2_col_Ivory,               qRgba(0xFF,0xFF,0xF0,0xFF)},
        {p2_col_Khaki,               qRgba(0xF0,0xE6,0x8C,0xFF)},
        {p2_col_Lavender,            qRgba(0xE6,0xE6,0xFA,0xFF)},
        {p2_col_Lavender_Blush,      qRgba(0xFF,0xF0,0xF5,0xFF)},
        {p2_col_Lawn_Green,          qRgba(0x7C,0xFC,0x00,0xFF)},
        {p2_col_Lemon_Chiffon,       qRgba(0xFF,0xFA,0xCD,0xFF)},
        {p2_col_Light_Blue,          qRgba(0xAD,0xD8,0xE6,0xFF)},
        {p2_col_Light_Coral,         qRgba(0xF0,0x80,0x80,0xFF)},
        {p2_col_Light_Cyan,          qRgba(0xE0,0xFF,0xFF,0xFF)},
        {p2_col_Light_Goldenrod,     qRgba(0xFA,0xFA,0xD2,0xFF)},
        {p2_col_Light_Gray,          qRgba(0xD3,0xD3,0xD3,0xFF)},
        {p2_col_Light_Green,         qRgba(0x90,0xEE,0x90,0xFF)},
        {p2_col_Light_Pink,          qRgba(0xFF,0xB6,0xC1,0xFF)},
        {p2_col_Light_Salmon,        qRgba(0xFF,0xA0,0x7A,0xFF)},
        {p2_col_Light_Sea_Green,     qRgba(0x20,0xB2,0xAA,0xFF)},
        {p2_col_Light_Sky_Blue,      qRgba(0x87,0xCE,0xFA,0xFF)},
        {p2_col_Light_Slate_Gray,    qRgba(0x77,0x88,0x99,0xFF)},
        {p2_col_Light_Steel_Blue,    qRgba(0xB0,0xC4,0xDE,0xFF)},
        {p2_col_Light_Yellow,        qRgba(0xFF,0xFF,0xE0,0xFF)},
        {p2_col_Lime,                qRgba(0x00,0xFF,0x01,0xFF)},
        {p2_col_Lime_Green,          qRgba(0x32,0xCD,0x32,0xFF)},
        {p2_col_Linen,               qRgba(0xFA,0xF0,0xE6,0xFF)},
        {p2_col_Magenta,             qRgba(0xFF,0x00,0xFF,0xFF)},
        {p2_col_Maroon,              qRgba(0xB0,0x30,0x60,0xFF)},
        {p2_col_Web_Maroon,          qRgba(0x80,0x00,0x00,0xFF)},
        {p2_col_Medium_Aquamarine,   qRgba(0x66,0xCD,0xAA,0xFF)},
        {p2_col_Medium_Blue,         qRgba(0x00,0x00,0xCD,0xFF)},
        {p2_col_Medium_Orchid,       qRgba(0xBA,0x55,0xD3,0xFF)},
        {p2_col_Medium_Purple,       qRgba(0x93,0x70,0xDB,0xFF)},
        {p2_col_Medium_Sea_Green,    qRgba(0x3C,0xB3,0x71,0xFF)},
        {p2_col_Medium_Slate_Blue,   qRgba(0x7B,0x68,0xEE,0xFF)},
        {p2_col_Medium_Spring_Green, qRgba(0x00,0xFA,0x9A,0xFF)},
        {p2_col_Medium_Turquoise,    qRgba(0x48,0xD1,0xCC,0xFF)},
        {p2_col_Medium_Violet_Red,   qRgba(0xC7,0x15,0x85,0xFF)},
        {p2_col_Midnight_Blue,       qRgba(0x19,0x19,0x70,0xFF)},
        {p2_col_Mint_Cream,          qRgba(0xF5,0xFF,0xFA,0xFF)},
        {p2_col_Misty_Rose,          qRgba(0xFF,0xE4,0xE1,0xFF)},
        {p2_col_Moccasin,            qRgba(0xFF,0xE4,0xB5,0xFF)},
        {p2_col_Navajo_White,        qRgba(0xFF,0xDE,0xAD,0xFF)},
        {p2_col_Navy_Blue,           qRgba(0x00,0x00,0x80,0xFF)},
        {p2_col_Old_Lace,            qRgba(0xFD,0xF5,0xE6,0xFF)},
        {p2_col_Olive,               qRgba(0x80,0x80,0x00,0xFF)},
        {p2_col_Olive_Drab,          qRgba(0x6B,0x8E,0x23,0xFF)},
        {p2_col_Orange,              qRgba(0xFF,0xA5,0x00,0xFF)},
        {p2_col_Orange_Red,          qRgba(0xFF,0x45,0x00,0xFF)},
        {p2_col_Orchid,              qRgba(0xDA,0x70,0xD6,0xFF)},
        {p2_col_Pale_Goldenrod,      qRgba(0xEE,0xE8,0xAA,0xFF)},
        {p2_col_Pale_Green,          qRgba(0x98,0xFB,0x98,0xFF)},
        {p2_col_Pale_Turquoise,      qRgba(0xAF,0xEE,0xEE,0xFF)},
        {p2_col_Pale_Violet_Red,     qRgba(0xDB,0x70,0x93,0xFF)},
        {p2_col_Papaya_Whip,         qRgba(0xFF,0xEF,0xD5,0xFF)},
        {p2_col_Peach_Puff,          qRgba(0xFF,0xDA,0xB9,0xFF)},
        {p2_col_Peru,                qRgba(0xCD,0x85,0x3F,0xFF)},
        {p2_col_Pink,                qRgba(0xFF,0xC0,0xCB,0xFF)},
        {p2_col_Plum,                qRgba(0xDD,0xA0,0xDD,0xFF)},
        {p2_col_Powder_Blue,         qRgba(0xB0,0xE0,0xE6,0xFF)},
        {p2_col_Purple,              qRgba(0xA0,0x20,0xF0,0xFF)},
        {p2_col_Web_Purple,          qRgba(0x80,0x00,0x80,0xFF)},
        {p2_col_Rebecca_Purple,      qRgba(0x66,0x33,0x99,0xFF)},
        {p2_col_Red,                 qRgba(0xFF,0x00,0x00,0xFF)},
        {p2_col_Rosy_Brown,          qRgba(0xBC,0x8F,0x8F,0xFF)},
        {p2_col_Royal_Blue,          qRgba(0x41,0x69,0xE1,0xFF)},
        {p2_col_Saddle_Brown,        qRgba(0x8B,0x45,0x13,0xFF)},
        {p2_col_Salmon,              qRgba(0xFA,0x80,0x72,0xFF)},
        {p2_col_Sandy_Brown,         qRgba(0xF4,0xA4,0x60,0xFF)},
        {p2_col_Sea_Green,           qRgba(0x2E,0x8B,0x57,0xFF)},
        {p2_col_Seashell,            qRgba(0xFF,0xF5,0xEE,0xFF)},
        {p2_col_Sienna,              qRgba(0xA0,0x52,0x2D,0xFF)},
        {p2_col_Silver,              qRgba(0xC0,0xC0,0xC0,0xFF)},
        {p2_col_Sky_Blue,            qRgba(0x87,0xCE,0xEB,0xFF)},
        {p2_col_Slate_Blue,          qRgba(0x6A,0x5A,0xCD,0xFF)},
        {p2_col_Slate_Gray,          qRgba(0x70,0x80,0x90,0xFF)},
        {p2_col_Snow,                qRgba(0xFF,0xFA,0xFA,0xFF)},
        {p2_col_Spring_Green,        qRgba(0x00,0xFF,0x7F,0xFF)},
        {p2_col_Steel_Blue,          qRgba(0x46,0x82,0xB4,0xFF)},
        {p2_col_Tan,                 qRgba(0xD2,0xB4,0x8C,0xFF)},
        {p2_col_Teal,                qRgba(0x00,0x80,0x80,0xFF)},
        {p2_col_Thistle,             qRgba(0xD8,0xBF,0xD8,0xFF)},
        {p2_col_Tomato,              qRgba(0xFF,0x63,0x47,0xFF)},
        {p2_col_Turquoise,           qRgba(0x40,0xE0,0xD0,0xFF)},
        {p2_col_Violet,              qRgba(0xEE,0x82,0xEE,0xFF)},
        {p2_col_Wheat,               qRgba(0xF5,0xDE,0xB3,0xFF)},
        {p2_col_White,               qRgba(0xFF,0xFF,0xFF,0xFF)},
        {p2_col_White_Smoke,         qRgba(0xF5,0xF5,0xF5,0xFF)},
        {p2_col_Yellow,              qRgba(0xFF,0xFF,0x00,0xFF)},
        {p2_col_Yellow_Green,        qRgba(0x9A,0xCD,0x32,0xFF)},
    };

    g_pal_name = {
        {p2_pal_background,       QLatin1String("Background")},
        {p2_pal_comment,          QLatin1String("Comment")},
        {p2_pal_instruction,      QLatin1String("Instruction")},
        {p2_pal_conditional,      QLatin1String("Conditional")},
        {p2_pal_wcz_suffix,       QLatin1String("WCZ_suffix")},
        {p2_pal_section,          QLatin1String("Section")},
        {p2_pal_modcz_param,      QLatin1String("MODCZ_parameter")},
        {p2_pal_symbol,           QLatin1String("Symbol")},
        {p2_pal_locsym,           QLatin1String("Local_symbol")},
        {p2_pal_expression,       QLatin1String("Expression")},
        {p2_pal_str_const,        QLatin1String("String_constant")},
        {p2_pal_bin_const,        QLatin1String("Binary_constant")},
        {p2_pal_byt_const,        QLatin1String("Byte_constant")},
        {p2_pal_dec_const,        QLatin1String("Decimal_constant")},
        {p2_pal_hex_const,        QLatin1String("Hexadecimal_constant")},
        {p2_pal_real_const,       QLatin1String("Real_constant")},
        {p2_pal_source,           QLatin1String("Source")}
    };

    // Build the reverse hash for color to name lookup
    foreach(const QRgb& rgba, g_col_rgba.keys())
        m_color_index.insert(rgba, g_col_rgba.key(rgba));

    m_color_lexicographic = g_col_rgba.keys().toVector();
    // m_color_names_lexicographic is sorted lexicographically
    std::sort(m_color_lexicographic.begin(), m_color_lexicographic.end(), color_compare_lexicographic);

    m_color_hue_sat_lum = g_col_rgba.keys().toVector();
    // m_colors_hue_sat_lum is sorted by hue, saturation, and luminance
    std::sort(m_color_hue_sat_lum.begin(), m_color_hue_sat_lum.end(), color_compare_hue_sat_lum);
}

/**
 * @brief Reset the palette to its defaults
 */
void P2Colors::reset_palette()
{
    m_default_palette.insert(p2_pal_background,      p2_col_White);
    m_default_palette.insert(p2_pal_source,          p2_col_Black);
    m_default_palette.insert(p2_pal_comment,         p2_col_Dark_Olive_Green);
    m_default_palette.insert(p2_pal_str_const,       p2_col_Cadet_Blue);
    m_default_palette.insert(p2_pal_bin_const,       p2_col_Dark_Blue);
    m_default_palette.insert(p2_pal_byt_const,       p2_col_Deep_Sky_Blue);
    m_default_palette.insert(p2_pal_dec_const,       p2_col_Sky_Blue);
    m_default_palette.insert(p2_pal_hex_const,       p2_col_Blue);
    m_default_palette.insert(p2_pal_real_const,      p2_col_Powder_Blue);
    m_default_palette.insert(p2_pal_locsym,          p2_col_Orange_Red);
    m_default_palette.insert(p2_pal_symbol,          p2_col_Dark_Orange);
    m_default_palette.insert(p2_pal_expression,      p2_col_Orange);
    m_default_palette.insert(p2_pal_section,         p2_col_Cyan);
    m_default_palette.insert(p2_pal_conditional,     p2_col_Violet);
    m_default_palette.insert(p2_pal_instruction,     p2_col_Dark_Cyan);
    m_default_palette.insert(p2_pal_modcz_param,     p2_col_Medium_Violet_Red);
    m_default_palette.insert(p2_pal_wcz_suffix,      p2_col_Pale_Violet_Red);

    m_current_palette = m_default_palette;
}

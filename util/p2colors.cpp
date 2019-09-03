#include <cmath>
#include "p2colors.h"

class P2Colors P2Colors;

P2Colors::P2Colors()
    : m_named_colors()
    , m_palette()
{
    m_named_colors = {
        {QStringLiteral("Alice Blue"),          qRgb(0xF0,0xF8,0xFF)},
        {QStringLiteral("Antique White"),       qRgb(0xFA,0xEB,0xD7)},
        {QStringLiteral("Aqua"),                qRgb(0x00,0xFF,0xFF)},
        {QStringLiteral("Aquamarine"),          qRgb(0x7F,0xFF,0xD4)},
        {QStringLiteral("Azure"),               qRgb(0xF0,0xFF,0xFF)},
        {QStringLiteral("Beige"),               qRgb(0xF5,0xF5,0xDC)},
        {QStringLiteral("Bisque"),              qRgb(0xFF,0xE4,0xC4)},
        {QStringLiteral("Black"),               qRgb(0x00,0x00,0x00)},
        {QStringLiteral("Blanched Almond"),     qRgb(0xFF,0xEB,0xCD)},
        {QStringLiteral("Blue"),                qRgb(0x00,0x00,0xFF)},
        {QStringLiteral("Blue Violet"),         qRgb(0x8A,0x2B,0xE2)},
        {QStringLiteral("Brown"),               qRgb(0xA5,0x2A,0x2A)},
        {QStringLiteral("Burlywood"),           qRgb(0xDE,0xB8,0x87)},
        {QStringLiteral("Cadet Blue"),          qRgb(0x5F,0x9E,0xA0)},
        {QStringLiteral("Chartreuse"),          qRgb(0x7F,0xFF,0x00)},
        {QStringLiteral("Chocolate"),           qRgb(0xD2,0x69,0x1E)},
        {QStringLiteral("Coral"),               qRgb(0xFF,0x7F,0x50)},
        {QStringLiteral("Cornflower Blue"),     qRgb(0x64,0x95,0xED)},
        {QStringLiteral("Cornsilk"),            qRgb(0xFF,0xF8,0xDC)},
        {QStringLiteral("Crimson"),             qRgb(0xDC,0x14,0x3C)},
        {QStringLiteral("Cyan"),                qRgb(0x00,0xFF,0xFF)},
        {QStringLiteral("Dark Blue"),           qRgb(0x00,0x00,0x8B)},
        {QStringLiteral("Dark Cyan"),           qRgb(0x00,0x8B,0x8B)},
        {QStringLiteral("Dark Goldenrod"),      qRgb(0xB8,0x86,0x0B)},
        {QStringLiteral("Dark Gray"),           qRgb(0xA9,0xA9,0xA9)},
        {QStringLiteral("Dark Green"),          qRgb(0x00,0x64,0x00)},
        {QStringLiteral("Dark Khaki"),          qRgb(0xBD,0xB7,0x6B)},
        {QStringLiteral("Dark Magenta"),        qRgb(0x8B,0x00,0x8B)},
        {QStringLiteral("Dark Olive Green"),    qRgb(0x55,0x6B,0x2F)},
        {QStringLiteral("Dark Orange"),         qRgb(0xFF,0x8C,0x00)},
        {QStringLiteral("Dark Orchid"),         qRgb(0x99,0x32,0xCC)},
        {QStringLiteral("Dark Red"),            qRgb(0x8B,0x00,0x00)},
        {QStringLiteral("Dark Salmon"),         qRgb(0xE9,0x96,0x7A)},
        {QStringLiteral("Dark Sea Green"),      qRgb(0x8F,0xBC,0x8F)},
        {QStringLiteral("Dark Slate Blue"),     qRgb(0x48,0x3D,0x8B)},
        {QStringLiteral("Dark Slate Gray"),     qRgb(0x2F,0x4F,0x4F)},
        {QStringLiteral("Dark Turquoise"),      qRgb(0x00,0xCE,0xD1)},
        {QStringLiteral("Dark Violet"),         qRgb(0x94,0x00,0xD3)},
        {QStringLiteral("Deep Pink"),           qRgb(0xFF,0x14,0x93)},
        {QStringLiteral("Deep Sky Blue"),       qRgb(0x00,0xBF,0xFF)},
        {QStringLiteral("Dim Gray"),            qRgb(0x69,0x69,0x69)},
        {QStringLiteral("Dodger Blue"),         qRgb(0x1E,0x90,0xFF)},
        {QStringLiteral("Firebrick"),           qRgb(0xB2,0x22,0x22)},
        {QStringLiteral("Floral White"),        qRgb(0xFF,0xFA,0xF0)},
        {QStringLiteral("Forest Green"),        qRgb(0x22,0x8B,0x22)},
        {QStringLiteral("Fuchsia"),             qRgb(0xFF,0x00,0xFF)},
        {QStringLiteral("Gainsboro"),           qRgb(0xDC,0xDC,0xDC)},
        {QStringLiteral("Ghost White"),         qRgb(0xF8,0xF8,0xFF)},
        {QStringLiteral("Gold"),                qRgb(0xFF,0xD7,0x00)},
        {QStringLiteral("Goldenrod"),           qRgb(0xDA,0xA5,0x20)},
        {QStringLiteral("Gray"),                qRgb(0xBE,0xBE,0xBE)},
        {QStringLiteral("Web Gray"),            qRgb(0x80,0x80,0x80)},
        {QStringLiteral("Green"),               qRgb(0x00,0xFF,0x00)},
        {QStringLiteral("Web Green"),           qRgb(0x00,0x80,0x00)},
        {QStringLiteral("Green Yellow"),        qRgb(0xAD,0xFF,0x2F)},
        {QStringLiteral("Honeydew"),            qRgb(0xF0,0xFF,0xF0)},
        {QStringLiteral("Hot Pink"),            qRgb(0xFF,0x69,0xB4)},
        {QStringLiteral("Indian Red"),          qRgb(0xCD,0x5C,0x5C)},
        {QStringLiteral("Indigo"),              qRgb(0x4B,0x00,0x82)},
        {QStringLiteral("Ivory"),               qRgb(0xFF,0xFF,0xF0)},
        {QStringLiteral("Khaki"),               qRgb(0xF0,0xE6,0x8C)},
        {QStringLiteral("Lavender"),            qRgb(0xE6,0xE6,0xFA)},
        {QStringLiteral("Lavender Blush"),      qRgb(0xFF,0xF0,0xF5)},
        {QStringLiteral("Lawn Green"),          qRgb(0x7C,0xFC,0x00)},
        {QStringLiteral("Lemon Chiffon"),       qRgb(0xFF,0xFA,0xCD)},
        {QStringLiteral("Light Blue"),          qRgb(0xAD,0xD8,0xE6)},
        {QStringLiteral("Light Coral"),         qRgb(0xF0,0x80,0x80)},
        {QStringLiteral("Light Cyan"),          qRgb(0xE0,0xFF,0xFF)},
        {QStringLiteral("Light Goldenrod"),     qRgb(0xFA,0xFA,0xD2)},
        {QStringLiteral("Light Gray"),          qRgb(0xD3,0xD3,0xD3)},
        {QStringLiteral("Light Green"),         qRgb(0x90,0xEE,0x90)},
        {QStringLiteral("Light Pink"),          qRgb(0xFF,0xB6,0xC1)},
        {QStringLiteral("Light Salmon"),        qRgb(0xFF,0xA0,0x7A)},
        {QStringLiteral("Light Sea Green"),     qRgb(0x20,0xB2,0xAA)},
        {QStringLiteral("Light Sky Blue"),      qRgb(0x87,0xCE,0xFA)},
        {QStringLiteral("Light Slate Gray"),    qRgb(0x77,0x88,0x99)},
        {QStringLiteral("Light Steel Blue"),    qRgb(0xB0,0xC4,0xDE)},
        {QStringLiteral("Light Yellow"),        qRgb(0xFF,0xFF,0xE0)},
        {QStringLiteral("Lime"),                qRgb(0x00,0xFF,0x00)},
        {QStringLiteral("Lime Green"),          qRgb(0x32,0xCD,0x32)},
        {QStringLiteral("Linen"),               qRgb(0xFA,0xF0,0xE6)},
        {QStringLiteral("Magenta"),             qRgb(0xFF,0x00,0xFF)},
        {QStringLiteral("Maroon"),              qRgb(0xB0,0x30,0x60)},
        {QStringLiteral("Web Maroon"),          qRgb(0x80,0x00,0x00)},
        {QStringLiteral("Medium Aquamarine"),   qRgb(0x66,0xCD,0xAA)},
        {QStringLiteral("Medium Blue"),         qRgb(0x00,0x00,0xCD)},
        {QStringLiteral("Medium Orchid"),       qRgb(0xBA,0x55,0xD3)},
        {QStringLiteral("Medium Purple"),       qRgb(0x93,0x70,0xDB)},
        {QStringLiteral("Medium Sea Green"),    qRgb(0x3C,0xB3,0x71)},
        {QStringLiteral("Medium Slate Blue"),   qRgb(0x7B,0x68,0xEE)},
        {QStringLiteral("Medium Spring Green"), qRgb(0x00,0xFA,0x9A)},
        {QStringLiteral("Medium Turquoise"),    qRgb(0x48,0xD1,0xCC)},
        {QStringLiteral("Medium Violet Red"),   qRgb(0xC7,0x15,0x85)},
        {QStringLiteral("Midnight Blue"),       qRgb(0x19,0x19,0x70)},
        {QStringLiteral("Mint Cream"),          qRgb(0xF5,0xFF,0xFA)},
        {QStringLiteral("Misty Rose"),          qRgb(0xFF,0xE4,0xE1)},
        {QStringLiteral("Moccasin"),            qRgb(0xFF,0xE4,0xB5)},
        {QStringLiteral("Navajo White"),        qRgb(0xFF,0xDE,0xAD)},
        {QStringLiteral("Navy Blue"),           qRgb(0x00,0x00,0x80)},
        {QStringLiteral("Old Lace"),            qRgb(0xFD,0xF5,0xE6)},
        {QStringLiteral("Olive"),               qRgb(0x80,0x80,0x00)},
        {QStringLiteral("Olive Drab"),          qRgb(0x6B,0x8E,0x23)},
        {QStringLiteral("Orange"),              qRgb(0xFF,0xA5,0x00)},
        {QStringLiteral("Orange Red"),          qRgb(0xFF,0x45,0x00)},
        {QStringLiteral("Orchid"),              qRgb(0xDA,0x70,0xD6)},
        {QStringLiteral("Pale Goldenrod"),      qRgb(0xEE,0xE8,0xAA)},
        {QStringLiteral("Pale Green"),          qRgb(0x98,0xFB,0x98)},
        {QStringLiteral("Pale Turquoise"),      qRgb(0xAF,0xEE,0xEE)},
        {QStringLiteral("Pale Violet Red"),     qRgb(0xDB,0x70,0x93)},
        {QStringLiteral("Papaya Whip"),         qRgb(0xFF,0xEF,0xD5)},
        {QStringLiteral("Peach Puff"),          qRgb(0xFF,0xDA,0xB9)},
        {QStringLiteral("Peru"),                qRgb(0xCD,0x85,0x3F)},
        {QStringLiteral("Pink"),                qRgb(0xFF,0xC0,0xCB)},
        {QStringLiteral("Plum"),                qRgb(0xDD,0xA0,0xDD)},
        {QStringLiteral("Powder Blue"),         qRgb(0xB0,0xE0,0xE6)},
        {QStringLiteral("Purple"),              qRgb(0xA0,0x20,0xF0)},
        {QStringLiteral("Web Purple"),          qRgb(0x80,0x00,0x80)},
        {QStringLiteral("Rebecca Purple"),      qRgb(0x66,0x33,0x99)},
        {QStringLiteral("Red"),                 qRgb(0xFF,0x00,0x00)},
        {QStringLiteral("Rosy Brown"),          qRgb(0xBC,0x8F,0x8F)},
        {QStringLiteral("Royal Blue"),          qRgb(0x41,0x69,0xE1)},
        {QStringLiteral("Saddle Brown"),        qRgb(0x8B,0x45,0x13)},
        {QStringLiteral("Salmon"),              qRgb(0xFA,0x80,0x72)},
        {QStringLiteral("Sandy Brown"),         qRgb(0xF4,0xA4,0x60)},
        {QStringLiteral("Sea Green"),           qRgb(0x2E,0x8B,0x57)},
        {QStringLiteral("Seashell"),            qRgb(0xFF,0xF5,0xEE)},
        {QStringLiteral("Sienna"),              qRgb(0xA0,0x52,0x2D)},
        {QStringLiteral("Silver"),              qRgb(0xC0,0xC0,0xC0)},
        {QStringLiteral("Sky Blue"),            qRgb(0x87,0xCE,0xEB)},
        {QStringLiteral("Slate Blue"),          qRgb(0x6A,0x5A,0xCD)},
        {QStringLiteral("Slate Gray"),          qRgb(0x70,0x80,0x90)},
        {QStringLiteral("Snow"),                qRgb(0xFF,0xFA,0xFA)},
        {QStringLiteral("Spring Green"),        qRgb(0x00,0xFF,0x7F)},
        {QStringLiteral("Steel Blue"),          qRgb(0x46,0x82,0xB4)},
        {QStringLiteral("Tan"),                 qRgb(0xD2,0xB4,0x8C)},
        {QStringLiteral("Teal"),                qRgb(0x00,0x80,0x80)},
        {QStringLiteral("Thistle"),             qRgb(0xD8,0xBF,0xD8)},
        {QStringLiteral("Tomato"),              qRgb(0xFF,0x63,0x47)},
        {QStringLiteral("Turquoise"),           qRgb(0x40,0xE0,0xD0)},
        {QStringLiteral("Violet"),              qRgb(0xEE,0x82,0xEE)},
        {QStringLiteral("Wheat"),               qRgb(0xF5,0xDE,0xB3)},
        {QStringLiteral("White"),               qRgb(0xFF,0xFF,0xFF)},
        {QStringLiteral("White Smoke"),         qRgb(0xF5,0xF5,0xF5)},
        {QStringLiteral("Yellow"),              qRgb(0xFF,0xFF,0x00)},
        {QStringLiteral("Yellow Green"),        qRgb(0x9A,0xCD,0x32)}
    };

    m_default_color_source       = color("Black");
    m_default_color_comment      = color("Light Green");
    m_default_color_str_const    = color("Cadet Blue");
    m_default_color_bin_const    = color("Dark Blue");
    m_default_color_byt_const    = color("Deep Sky Blue");
    m_default_color_dec_const    = color("Sky Blue");
    m_default_color_hex_const    = color("Blue");
    m_default_color_real_const   = color("Powder Blue");
    m_default_color_locsym       = color("Hot Pink");
    m_default_color_symbol       = color("Orange");
    m_default_color_expression   = color("Orange Red");
    m_default_color_section      = color("Cyan");
    m_default_color_conditional  = color("Violet");
    m_default_color_instruction  = color("Dark Cyan");
    m_default_color_modzc_param  = color("Medium Violet Red");
    m_default_color_wcz_suffix   = color("Pale Violet Red");

    m_palette.insert(p2_pal_source, m_default_color_source);
    m_palette.insert(p2_pal_comment, m_default_color_comment);
    m_palette.insert(p2_pal_str_const, m_default_color_str_const);
    m_palette.insert(p2_pal_bin_const, m_default_color_bin_const);
    m_palette.insert(p2_pal_byt_const, m_default_color_byt_const);
    m_palette.insert(p2_pal_dec_const, m_default_color_dec_const);
    m_palette.insert(p2_pal_hex_const, m_default_color_hex_const);
    m_palette.insert(p2_pal_real_const, m_default_color_real_const);
    m_palette.insert(p2_pal_locsym, m_default_color_locsym);
    m_palette.insert(p2_pal_symbol, m_default_color_symbol);
    m_palette.insert(p2_pal_expression, m_default_color_expression);
    m_palette.insert(p2_pal_section, m_default_color_section);
    m_palette.insert(p2_pal_conditional, m_default_color_conditional);
    m_palette.insert(p2_pal_modzc_param, m_default_color_modzc_param);
    m_palette.insert(p2_pal_instruction, m_default_color_instruction);
    m_palette.insert(p2_pal_wcz_suffix, m_default_color_wcz_suffix);
}

QStringList P2Colors::named_colors() const
{
    return m_named_colors.keys();
}


QRgb P2Colors::color(const QString& name)
{
    if (m_named_colors.contains(name))
        return m_named_colors[name];
    return qRgb(0x00,0x00,0x00);
}

QRgb P2Colors::color_at(int idx)
{
    if (idx < 0 || idx >= m_named_colors.count())
        return qRgb(0x00,0x00,0x00);
    return color(m_named_colors.keys().value(idx));
}

int P2Colors::color_index(const QString& name)
{
    return m_named_colors.keys().indexOf(name);
}

QString P2Colors::closest_color(QColor color)
{
    int minidx = -1;
    qreal mindist = 99999.0;
    const QStringList colorNames = m_named_colors.keys();
    for (int i = 0; !qFuzzyIsNull(mindist) && i < colorNames.size(); ++i) {
        QColor known(colorNames[i]);
        int dr = qAbs(known.red() - color.red());
        int dg = qAbs(known.green() - color.green());
        int db = qAbs(known.blue() - color.blue());
        qreal dist = sqrt(sqrt(dr*dr+dg*dg)+db*db);
        if (dist < mindist) {
            mindist = dist;
            minidx = i;
        }
    }
    if (mindist < 10.0)
        return colorNames[minidx];
    return color.name();
}

QColor P2Colors::palette(p2_palette_e pal, bool highlight)
{
    QColor color = m_palette.value(pal, m_default_color_source);
    if (highlight)
        color = color.darker(120);
    return color;
}

QColor P2Colors::palette(p2_token_e tok, bool highlight)
{
    QColor color = palette(p2_pal_source, highlight);
    switch (tok) {
    case t_comment:
    case t_comment_eol:
    case t_comment_lcurly:
    case t_comment_rcurly:
        color = palette(p2_pal_comment, highlight);
        break;

    case t_str_const:
        color = palette(p2_pal_str_const, highlight);
        break;

    case t_bin_const:
        color = palette(p2_pal_bin_const, highlight);
        break;

    case t_byt_const:
        color = palette(p2_pal_byt_const, highlight);
        break;

    case t_dec_const:
        color = palette(p2_pal_dec_const, highlight);
        break;

    case t_hex_const:
        color = palette(p2_pal_hex_const, highlight);
        break;

    case t_real_const:
        color = palette(p2_pal_real_const, highlight);
        break;

    case t_locsym:
        color = palette(p2_pal_locsym, highlight);
        break;

    case t_symbol:
        color = palette(p2_pal_symbol, highlight);
        break;

    default:
        if (Token.is_type(tok, tm_section))
            color = palette(p2_pal_section, highlight);

        if (Token.is_type(tok, tm_conditional))
            color = palette(p2_pal_conditional, highlight);

        if (Token.is_type(tok, tm_mnemonic))
            color = palette(p2_pal_instruction, highlight);

        if (Token.is_type(tok, tm_wcz_suffix))
            color = palette(p2_pal_wcz_suffix, highlight);

        if (Token.is_type(tok, tm_expression))
            color = palette(p2_pal_expression, highlight);
        break;
    }
    return color;
}

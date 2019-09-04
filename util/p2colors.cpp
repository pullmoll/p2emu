#include <cmath>
#include "p2colors.h"
#include "p2util.h"

P2Colors Colors;

static bool color_compare(const QColor& c1, const QColor& c2)
{
    qreal h1, v1, s1, a1;
    qreal h2, v2, s2, a2;
    c1.getHsvF(&h1, &s1, &v1, &a1);
    c2.getHsvF(&h2, &s2, &v2, &a2);
    const qreal val1 = (1000.0 * h1 + 100.0 * v1 + 10.0 * s1 + a1) * a1;
    const qreal val2 = (1000.0 * h2 + 100.0 * v2 + 10.0 * s2 + a2) * a2;
    return val1 < val2;
}

P2Colors::P2Colors()
    : m_named_colors()
    , m_palette()
    , m_color_names()
    , m_color_values()
    , m_colors_hue_sat_lum()
    , m_palette_names()
{
    m_named_colors = {
        {QStringLiteral("Alice Blue"),          QColor(0xF0,0xF8,0xFF,0xFF)},
        {QStringLiteral("Antique White"),       QColor(0xFA,0xEB,0xD7,0xFF)},
        {QStringLiteral("Aqua"),                QColor(0x00,0xFF,0xFF,0xFF)},
        {QStringLiteral("Aquamarine"),          QColor(0x7F,0xFF,0xD4,0xFF)},
        {QStringLiteral("Azure"),               QColor(0xF0,0xFF,0xFF,0xFF)},
        {QStringLiteral("Beige"),               QColor(0xF5,0xF5,0xDC,0xFF)},
        {QStringLiteral("Bisque"),              QColor(0xFF,0xE4,0xC4,0xFF)},
        {QStringLiteral("Black"),               QColor(0x00,0x00,0x00,0xFF)},
        {QStringLiteral("Blanched Almond"),     QColor(0xFF,0xEB,0xCD,0xFF)},
        {QStringLiteral("Blue"),                QColor(0x00,0x00,0xFF,0xFF)},
        {QStringLiteral("Blue Violet"),         QColor(0x8A,0x2B,0xE2,0xFF)},
        {QStringLiteral("Brown"),               QColor(0xA5,0x2A,0x2A,0xFF)},
        {QStringLiteral("Burlywood"),           QColor(0xDE,0xB8,0x87,0xFF)},
        {QStringLiteral("Cadet Blue"),          QColor(0x5F,0x9E,0xA0,0xFF)},
        {QStringLiteral("Chartreuse"),          QColor(0x7F,0xFF,0x00,0xFF)},
        {QStringLiteral("Chocolate"),           QColor(0xD2,0x69,0x1E,0xFF)},
        {QStringLiteral("Coral"),               QColor(0xFF,0x7F,0x50,0xFF)},
        {QStringLiteral("Cornflower Blue"),     QColor(0x64,0x95,0xED,0xFF)},
        {QStringLiteral("Cornsilk"),            QColor(0xFF,0xF8,0xDC,0xFF)},
        {QStringLiteral("Crimson"),             QColor(0xDC,0x14,0x3C,0xFF)},
        {QStringLiteral("Cyan"),                QColor(0x00,0xFF,0xFF,0xFF)},
        {QStringLiteral("Dark Blue"),           QColor(0x00,0x00,0x8B,0xFF)},
        {QStringLiteral("Dark Cyan"),           QColor(0x00,0x8B,0x8B,0xFF)},
        {QStringLiteral("Dark Goldenrod"),      QColor(0xB8,0x86,0x0B,0xFF)},
        {QStringLiteral("Dark Gray"),           QColor(0xA9,0xA9,0xA9,0xFF)},
        {QStringLiteral("Dark Green"),          QColor(0x00,0x64,0x00,0xFF)},
        {QStringLiteral("Dark Khaki"),          QColor(0xBD,0xB7,0x6B,0xFF)},
        {QStringLiteral("Dark Magenta"),        QColor(0x8B,0x00,0x8B,0xFF)},
        {QStringLiteral("Dark Olive Green"),    QColor(0x55,0x6B,0x2F,0xFF)},
        {QStringLiteral("Dark Orange"),         QColor(0xFF,0x8C,0x00,0xFF)},
        {QStringLiteral("Dark Orchid"),         QColor(0x99,0x32,0xCC,0xFF)},
        {QStringLiteral("Dark Red"),            QColor(0x8B,0x00,0x00,0xFF)},
        {QStringLiteral("Dark Salmon"),         QColor(0xE9,0x96,0x7A,0xFF)},
        {QStringLiteral("Dark Sea Green"),      QColor(0x8F,0xBC,0x8F,0xFF)},
        {QStringLiteral("Dark Slate Blue"),     QColor(0x48,0x3D,0x8B,0xFF)},
        {QStringLiteral("Dark Slate Gray"),     QColor(0x2F,0x4F,0x4F,0xFF)},
        {QStringLiteral("Dark Turquoise"),      QColor(0x00,0xCE,0xD1,0xFF)},
        {QStringLiteral("Dark Violet"),         QColor(0x94,0x00,0xD3,0xFF)},
        {QStringLiteral("Deep Pink"),           QColor(0xFF,0x14,0x93,0xFF)},
        {QStringLiteral("Deep Sky Blue"),       QColor(0x00,0xBF,0xFF,0xFF)},
        {QStringLiteral("Dim Gray"),            QColor(0x69,0x69,0x69,0xFF)},
        {QStringLiteral("Dodger Blue"),         QColor(0x1E,0x90,0xFF,0xFF)},
        {QStringLiteral("Firebrick"),           QColor(0xB2,0x22,0x22,0xFF)},
        {QStringLiteral("Floral White"),        QColor(0xFF,0xFA,0xF0,0xFF)},
        {QStringLiteral("Forest Green"),        QColor(0x22,0x8B,0x22,0xFF)},
        {QStringLiteral("Fuchsia"),             QColor(0xFF,0x01,0xFF,0xFF)},
        {QStringLiteral("Gainsboro"),           QColor(0xDC,0xDC,0xDC,0xFF)},
        {QStringLiteral("Ghost White"),         QColor(0xF8,0xF8,0xFF,0xFF)},
        {QStringLiteral("Gold"),                QColor(0xFF,0xD7,0x00,0xFF)},
        {QStringLiteral("Goldenrod"),           QColor(0xDA,0xA5,0x20,0xFF)},
        {QStringLiteral("Gray"),                QColor(0xBE,0xBE,0xBE,0xFF)},
        {QStringLiteral("Web Gray"),            QColor(0x80,0x80,0x80,0xFF)},
        {QStringLiteral("Green"),               QColor(0x00,0xFF,0x00,0xFF)},
        {QStringLiteral("Web Green"),           QColor(0x00,0x80,0x00,0xFF)},
        {QStringLiteral("Green Yellow"),        QColor(0xAD,0xFF,0x2F,0xFF)},
        {QStringLiteral("Honeydew"),            QColor(0xF0,0xFF,0xF0,0xFF)},
        {QStringLiteral("Hot Pink"),            QColor(0xFF,0x69,0xB4,0xFF)},
        {QStringLiteral("Indian Red"),          QColor(0xCD,0x5C,0x5C,0xFF)},
        {QStringLiteral("Indigo"),              QColor(0x4B,0x00,0x82,0xFF)},
        {QStringLiteral("Ivory"),               QColor(0xFF,0xFF,0xF0,0xFF)},
        {QStringLiteral("Khaki"),               QColor(0xF0,0xE6,0x8C,0xFF)},
        {QStringLiteral("Lavender"),            QColor(0xE6,0xE6,0xFA,0xFF)},
        {QStringLiteral("Lavender Blush"),      QColor(0xFF,0xF0,0xF5,0xFF)},
        {QStringLiteral("Lawn Green"),          QColor(0x7C,0xFC,0x00,0xFF)},
        {QStringLiteral("Lemon Chiffon"),       QColor(0xFF,0xFA,0xCD,0xFF)},
        {QStringLiteral("Light Blue"),          QColor(0xAD,0xD8,0xE6,0xFF)},
        {QStringLiteral("Light Coral"),         QColor(0xF0,0x80,0x80,0xFF)},
        {QStringLiteral("Light Cyan"),          QColor(0xE0,0xFF,0xFF,0xFF)},
        {QStringLiteral("Light Goldenrod"),     QColor(0xFA,0xFA,0xD2,0xFF)},
        {QStringLiteral("Light Gray"),          QColor(0xD3,0xD3,0xD3,0xFF)},
        {QStringLiteral("Light Green"),         QColor(0x90,0xEE,0x90,0xFF)},
        {QStringLiteral("Light Pink"),          QColor(0xFF,0xB6,0xC1,0xFF)},
        {QStringLiteral("Light Salmon"),        QColor(0xFF,0xA0,0x7A,0xFF)},
        {QStringLiteral("Light Sea Green"),     QColor(0x20,0xB2,0xAA,0xFF)},
        {QStringLiteral("Light Sky Blue"),      QColor(0x87,0xCE,0xFA,0xFF)},
        {QStringLiteral("Light Slate Gray"),    QColor(0x77,0x88,0x99,0xFF)},
        {QStringLiteral("Light Steel Blue"),    QColor(0xB0,0xC4,0xDE,0xFF)},
        {QStringLiteral("Light Yellow"),        QColor(0xFF,0xFF,0xE0,0xFF)},
        {QStringLiteral("Lime"),                QColor(0x00,0xFF,0x01,0xFF)},
        {QStringLiteral("Lime Green"),          QColor(0x32,0xCD,0x32,0xFF)},
        {QStringLiteral("Linen"),               QColor(0xFA,0xF0,0xE6,0xFF)},
        {QStringLiteral("Magenta"),             QColor(0xFF,0x00,0xFF,0xFF)},
        {QStringLiteral("Maroon"),              QColor(0xB0,0x30,0x60,0xFF)},
        {QStringLiteral("Web Maroon"),          QColor(0x80,0x00,0x00,0xFF)},
        {QStringLiteral("Medium Aquamarine"),   QColor(0x66,0xCD,0xAA,0xFF)},
        {QStringLiteral("Medium Blue"),         QColor(0x00,0x00,0xCD,0xFF)},
        {QStringLiteral("Medium Orchid"),       QColor(0xBA,0x55,0xD3,0xFF)},
        {QStringLiteral("Medium Purple"),       QColor(0x93,0x70,0xDB,0xFF)},
        {QStringLiteral("Medium Sea Green"),    QColor(0x3C,0xB3,0x71,0xFF)},
        {QStringLiteral("Medium Slate Blue"),   QColor(0x7B,0x68,0xEE,0xFF)},
        {QStringLiteral("Medium Spring Green"), QColor(0x00,0xFA,0x9A,0xFF)},
        {QStringLiteral("Medium Turquoise"),    QColor(0x48,0xD1,0xCC,0xFF)},
        {QStringLiteral("Medium Violet Red"),   QColor(0xC7,0x15,0x85,0xFF)},
        {QStringLiteral("Midnight Blue"),       QColor(0x19,0x19,0x70,0xFF)},
        {QStringLiteral("Mint Cream"),          QColor(0xF5,0xFF,0xFA,0xFF)},
        {QStringLiteral("Misty Rose"),          QColor(0xFF,0xE4,0xE1,0xFF)},
        {QStringLiteral("Moccasin"),            QColor(0xFF,0xE4,0xB5,0xFF)},
        {QStringLiteral("Navajo White"),        QColor(0xFF,0xDE,0xAD,0xFF)},
        {QStringLiteral("Navy Blue"),           QColor(0x00,0x00,0x80,0xFF)},
        {QStringLiteral("Old Lace"),            QColor(0xFD,0xF5,0xE6,0xFF)},
        {QStringLiteral("Olive"),               QColor(0x80,0x80,0x00,0xFF)},
        {QStringLiteral("Olive Drab"),          QColor(0x6B,0x8E,0x23,0xFF)},
        {QStringLiteral("Orange"),              QColor(0xFF,0xA5,0x00,0xFF)},
        {QStringLiteral("Orange Red"),          QColor(0xFF,0x45,0x00,0xFF)},
        {QStringLiteral("Orchid"),              QColor(0xDA,0x70,0xD6,0xFF)},
        {QStringLiteral("Pale Goldenrod"),      QColor(0xEE,0xE8,0xAA,0xFF)},
        {QStringLiteral("Pale Green"),          QColor(0x98,0xFB,0x98,0xFF)},
        {QStringLiteral("Pale Turquoise"),      QColor(0xAF,0xEE,0xEE,0xFF)},
        {QStringLiteral("Pale Violet Red"),     QColor(0xDB,0x70,0x93,0xFF)},
        {QStringLiteral("Papaya Whip"),         QColor(0xFF,0xEF,0xD5,0xFF)},
        {QStringLiteral("Peach Puff"),          QColor(0xFF,0xDA,0xB9,0xFF)},
        {QStringLiteral("Peru"),                QColor(0xCD,0x85,0x3F,0xFF)},
        {QStringLiteral("Pink"),                QColor(0xFF,0xC0,0xCB,0xFF)},
        {QStringLiteral("Plum"),                QColor(0xDD,0xA0,0xDD,0xFF)},
        {QStringLiteral("Powder Blue"),         QColor(0xB0,0xE0,0xE6,0xFF)},
        {QStringLiteral("Purple"),              QColor(0xA0,0x20,0xF0,0xFF)},
        {QStringLiteral("Web Purple"),          QColor(0x80,0x00,0x80,0xFF)},
        {QStringLiteral("Rebecca Purple"),      QColor(0x66,0x33,0x99,0xFF)},
        {QStringLiteral("Red"),                 QColor(0xFF,0x00,0x00,0xFF)},
        {QStringLiteral("Rosy Brown"),          QColor(0xBC,0x8F,0x8F,0xFF)},
        {QStringLiteral("Royal Blue"),          QColor(0x41,0x69,0xE1,0xFF)},
        {QStringLiteral("Saddle Brown"),        QColor(0x8B,0x45,0x13,0xFF)},
        {QStringLiteral("Salmon"),              QColor(0xFA,0x80,0x72,0xFF)},
        {QStringLiteral("Sandy Brown"),         QColor(0xF4,0xA4,0x60,0xFF)},
        {QStringLiteral("Sea Green"),           QColor(0x2E,0x8B,0x57,0xFF)},
        {QStringLiteral("Seashell"),            QColor(0xFF,0xF5,0xEE,0xFF)},
        {QStringLiteral("Sienna"),              QColor(0xA0,0x52,0x2D,0xFF)},
        {QStringLiteral("Silver"),              QColor(0xC0,0xC0,0xC0,0xFF)},
        {QStringLiteral("Sky Blue"),            QColor(0x87,0xCE,0xEB,0xFF)},
        {QStringLiteral("Slate Blue"),          QColor(0x6A,0x5A,0xCD,0xFF)},
        {QStringLiteral("Slate Gray"),          QColor(0x70,0x80,0x90,0xFF)},
        {QStringLiteral("Snow"),                QColor(0xFF,0xFA,0xFA,0xFF)},
        {QStringLiteral("Spring Green"),        QColor(0x00,0xFF,0x7F,0xFF)},
        {QStringLiteral("Steel Blue"),          QColor(0x46,0x82,0xB4,0xFF)},
        {QStringLiteral("Tan"),                 QColor(0xD2,0xB4,0x8C,0xFF)},
        {QStringLiteral("Teal"),                QColor(0x00,0x80,0x80,0xFF)},
        {QStringLiteral("Thistle"),             QColor(0xD8,0xBF,0xD8,0xFF)},
        {QStringLiteral("Tomato"),              QColor(0xFF,0x63,0x47,0xFF)},
        {QStringLiteral("Turquoise"),           QColor(0x40,0xE0,0xD0,0xFF)},
        {QStringLiteral("Violet"),              QColor(0xEE,0x82,0xEE,0xFF)},
        {QStringLiteral("Wheat"),               QColor(0xF5,0xDE,0xB3,0xFF)},
        {QStringLiteral("White"),               QColor(0xFF,0xFF,0xFF,0xFF)},
        {QStringLiteral("White Smoke"),         QColor(0xF5,0xF5,0xF5,0xFF)},
        {QStringLiteral("Yellow"),              QColor(0xFF,0xFF,0x00,0xFF)},
        {QStringLiteral("Yellow Green"),        QColor(0x9A,0xCD,0x32,0xFF)},
        {QStringLiteral("Transparent"),         QColor(0x01,0x01,0x01,0x00)}
    };

    m_palette_names.insert(p2_pal_comment,      QStringLiteral("Comment"));
    m_palette_names.insert(p2_pal_instruction,  QStringLiteral("Instruction"));
    m_palette_names.insert(p2_pal_conditional,  QStringLiteral("Conditional"));
    m_palette_names.insert(p2_pal_wcz_suffix,   QStringLiteral("WCZ_suffix"));
    m_palette_names.insert(p2_pal_section,      QStringLiteral("Section"));
    m_palette_names.insert(p2_pal_modcz_param,  QStringLiteral("MODCZ_parameter"));
    m_palette_names.insert(p2_pal_symbol,       QStringLiteral("Symbol"));
    m_palette_names.insert(p2_pal_locsym,       QStringLiteral("Local_symbol"));
    m_palette_names.insert(p2_pal_expression,   QStringLiteral("Expression"));
    m_palette_names.insert(p2_pal_str_const,    QStringLiteral("String_constant"));
    m_palette_names.insert(p2_pal_bin_const,    QStringLiteral("Binary_constant"));
    m_palette_names.insert(p2_pal_byt_const,    QStringLiteral("Byte_constant"));
    m_palette_names.insert(p2_pal_dec_const,    QStringLiteral("Decimal_constant"));
    m_palette_names.insert(p2_pal_hex_const,    QStringLiteral("Hexadecimal_constant"));
    m_palette_names.insert(p2_pal_real_const,   QStringLiteral("Real_constant"));
    m_palette_names.insert(p2_pal_source,       QStringLiteral("Source"));

    reset_palette();
}

/**
 * @brief Return a list of color names
 * @param sort_by_hue_sat_lum if true, the list returned is sorted by hue, saturation, and luminance
 * @return QStringList with names
 */
QStringList P2Colors::color_names(bool sort_by_hue_sat_lum) const
{
    if (m_color_names.isEmpty())
        setup_tables();
    QStringList list;
    if (sort_by_hue_sat_lum) {
        for (int i = 0; i < m_colors_hue_sat_lum.count(); i++) {
            const QColor& color = m_colors_hue_sat_lum[i];
            QString name = m_named_colors.key(color.rgb());
            list.append(name);
        }
    } else {
        list = m_color_names;
    }
    return list;
}

/**
 * @brief Return the QColor value for a color name
 * @param name of the color (X11 colors)
 * @return QColor value, or black if unknown
 */
QColor P2Colors::color(const QString& name) const
{
    if (m_color_names.isEmpty())
        setup_tables();
    return m_named_colors.value(name, Qt::black);
}

/**
 * @brief Return the color at index %idx
 * @param idx index into the color names
 * @return QColor of the color
 */
QColor P2Colors::color_at(int idx) const
{
    if (m_color_names.isEmpty())
        setup_tables();
    return color(m_color_names.value(idx));
}

/**
 * @brief Return the index for a color name
 * @param name of the color
 * @return index in the names, or -1 if not found
 */
int P2Colors::color_index(const QString& name) const
{
    if (m_color_names.isEmpty())
        setup_tables();
    return m_color_names.indexOf(name);
}

/**
 * @brief Return the name of a color closest to the QColor
 * @param color some QColor
 * @return QString name of the closes color from our table
 */
QString P2Colors::closest(QColor color) const
{
    if (m_color_names.isEmpty())
        setup_tables();

    if (!m_named_colors.key(color).isEmpty())
        return m_named_colors.key(color);

    int i = 0;
    int bestidx = -1;
    qreal bestdist = 99999.0;
    i = 0;
    foreach(const QString& name, m_color_names) {
        QColor known = m_named_colors[name];
        const qreal dr = qAbs(known.redF() - color.redF());
        const qreal dg = qAbs(known.greenF() - color.greenF());
        const qreal db = qAbs(known.blueF() - color.blueF());
        const qreal da = qAbs(known.alphaF() - color.alphaF());
        qreal dist = sqrt(sqrt(sqrt(dr*dr + dg*dg) + db*db) + da*da);
        if (dist < bestdist) {
            bestdist = dist;
            bestidx = i;
        }
        i++;
    }
    Q_ASSERT(bestidx >= 0);
    return m_color_names.value(bestidx);
}

/**
 * @brief Return the technical name of a palette enumeration value %pal
 * @param pal palette index from p2_palette_e
 * @return QString with the technical name
 */
QString P2Colors::palette_name(P2Colors::p2_palette_e pal) const
{
    return m_palette_names.value(pal);
}

/**
 * @brief Return a list of all palette names
 * @return QStringList with the names
 */
QStringList P2Colors::palette_names() const
{
    return m_palette_names.values();
}

/**
 * @brief Return the palette enumeration value for a technical name
 * @param name QString as returned by %palette_name(pal)
 * @return palette enumeration value
 */
P2Colors::p2_palette_e P2Colors::palette_key(const QString& name) const
{
    return m_palette_names.key(name, p2_pal_source);
}

/**
 * @brief Return the current palette
 * @param reset_default if true, reset to the default palette first
 * @return QHash of palette enumeration values and their QColor
 */
const QHash<P2Colors::p2_palette_e, QColor>& P2Colors::palette(bool reset_default)
{
    if (reset_default)
        reset_palette();
    return m_palette;
}

/**
 * @brief Return a palette color for the palette enumeration value %pal
 * @param pal palette index from p2_palette_e
 * @param darker if true, make the color darker
 * @return QColor for the index
 */
QColor P2Colors::palette_color(p2_palette_e pal, bool darker) const
{
    QColor color = m_palette.value(pal, m_default_colors[p2_pal_source]);
    if (darker)
        color = color.darker(120);
    return color;
}

/**
 * @brief Set the entire palette to a new hash
 * @param palette const reference to a QHash of palette enumeration values and their QColor
 */
void P2Colors::set_palette(const QHash<P2Colors::p2_palette_e, QColor>& palette)
{
    m_palette = palette;
}

/**
 * @brief Set a single palette color
 * @param pal palette index from p2_palette_e
 * @param color QColor to set
 */
void P2Colors::set_palette_color(P2Colors::p2_palette_e pal, const QColor& color)
{
    m_palette.insert(pal, color);
}

/**
 * @brief Set a single palette color
 * @param pal palette index from p2_palette_e
 * @param name color name
 */
void P2Colors::set_palette_color(P2Colors::p2_palette_e pal, const QString& name)
{
    m_palette.insert(pal, m_named_colors.value(name));
}

/**
 * @brief Return a palette color for the specified parser token %tok
 * @param tok parser token enumeration value
 * @param darker if true, make the color darker
 * @return QColor from the palette
 */
QColor P2Colors::palette_color(p2_token_e tok, bool darker) const
{
    p2_palette_e pal = p2_pal_source;

    switch (tok) {
    case t_comment:
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
    return palette_color(pal, darker);
}

/**
 * @brief Setup and sort the tables
 */
void P2Colors::setup_tables() const
{
    // m_color_names is sorted lexicographically
    m_color_names = m_named_colors.keys();
    std::sort(m_color_names.begin(), m_color_names.end());

    m_color_values.clear();
    for (int i = 0; i < m_color_names.size(); ++i)
        m_color_values += m_named_colors.value(m_color_names[i]);

    // Sort by base colors and ascending luminance
    m_colors_hue_sat_lum = m_color_values;
    std::sort(m_colors_hue_sat_lum.begin(), m_colors_hue_sat_lum.end(), color_compare);
}

/**
 * @brief Reset the palette to its defaults
 */
void P2Colors::reset_palette()
{
    m_default_colors.insert(p2_pal_source,        m_named_colors.value("Black"));
    m_default_colors.insert(p2_pal_comment,       m_named_colors.value("Light Green"));
    m_default_colors.insert(p2_pal_str_const,     m_named_colors.value("Cadet Blue"));
    m_default_colors.insert(p2_pal_bin_const,     m_named_colors.value("Dark Blue"));
    m_default_colors.insert(p2_pal_byt_const,     m_named_colors.value("Deep Sky Blue"));
    m_default_colors.insert(p2_pal_dec_const,     m_named_colors.value("Sky Blue"));
    m_default_colors.insert(p2_pal_hex_const,     m_named_colors.value("Blue"));
    m_default_colors.insert(p2_pal_real_const,    m_named_colors.value("Powder Blue"));
    m_default_colors.insert(p2_pal_locsym,        m_named_colors.value("Hot Pink"));
    m_default_colors.insert(p2_pal_symbol,        m_named_colors.value("Orange"));
    m_default_colors.insert(p2_pal_expression,    m_named_colors.value("Orange Red"));
    m_default_colors.insert(p2_pal_section,       m_named_colors.value("Cyan"));
    m_default_colors.insert(p2_pal_conditional,   m_named_colors.value("Violet"));
    m_default_colors.insert(p2_pal_instruction,   m_named_colors.value("Dark Cyan"));
    m_default_colors.insert(p2_pal_modcz_param,   m_named_colors.value("Medium Violet Red"));
    m_default_colors.insert(p2_pal_wcz_suffix,    m_named_colors.value("Pale Violet Red"));

    foreach(p2_palette_e pal, m_default_colors.keys())
        m_palette.insert(pal, m_default_colors[pal]);
}

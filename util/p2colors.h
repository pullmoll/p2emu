#pragma once
#include <QVariant>
#include <QColor>
#include <QHash>
#include "p2token.h"

class P2Colors
{
public:
    P2Colors();

    enum p2_palette_e {
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
        p2_pal_modzc_param,
        p2_pal_instruction,
        p2_pal_wcz_suffix,
    };

    QStringList named_colors() const;
    QRgb color(const QString& name);
    QRgb color_at(int idx);
    int color_index(const QString& name);
    QString closest_color(QColor color);

    QColor palette(p2_palette_e pal, bool highlight = false);
    QColor palette(p2_token_e tok, bool highlight = false);

private:
    QHash<QString,QRgb> m_named_colors;
    QHash<p2_palette_e,QColor> m_palette;
    QColor m_default_color_source;
    QColor m_default_color_comment;
    QColor m_default_color_str_const;
    QColor m_default_color_bin_const;
    QColor m_default_color_byt_const;
    QColor m_default_color_dec_const;
    QColor m_default_color_hex_const;
    QColor m_default_color_real_const;
    QColor m_default_color_locsym;
    QColor m_default_color_symbol;
    QColor m_default_color_expression;
    QColor m_default_color_section;
    QColor m_default_color_conditional;
    QColor m_default_color_instruction;
    QColor m_default_color_modzc_param;
    QColor m_default_color_wcz_suffix;

};

Q_DECLARE_METATYPE(P2Colors::p2_palette_e);

extern P2Colors P2Colors;

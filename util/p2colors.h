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

    QStringList color_names(bool sort_by_hue_sat_lum = false) const;
    QColor color(const QString& name) const;
    QColor color_at(int idx) const;
    int color_index(const QString& name) const;
    QString closest(QColor color) const;

    QString palette_name(p2_palette_e id = p2_pal_source) const;
    QStringList palette_names() const;
    p2_palette_e palette_key(const QString& name) const;
    const QHash<p2_palette_e, QColor>& palette(bool reset_default = false);
    QColor palette_color(p2_palette_e pal, bool darker = false) const;
    QColor palette_color(p2_token_e tok, bool darker = false) const;

public slots:
    void set_palette_color(p2_palette_e pal, const QColor& color);
    void set_palette_color(p2_palette_e pal, const QString& name);
    void set_palette(const QHash<p2_palette_e, QColor>& palette_color);

private:
    QHash<QString,QColor> m_named_colors;
    mutable QHash<p2_palette_e,QColor> m_palette;
    mutable QStringList m_color_names;
    mutable QVector<QColor> m_color_values;
    mutable QVector<QColor> m_colors_hue_sat_lum;
    QMap<p2_palette_e,QString> m_palette_names;
    QMap<p2_palette_e,QColor> m_default_colors;
    void setup_tables() const;
    void reset_palette();
};

Q_DECLARE_METATYPE(P2Colors::p2_palette_e);

extern P2Colors Colors;

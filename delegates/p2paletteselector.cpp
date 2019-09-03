#include "p2colors.h"
#include "p2paletteselector.h"
#include <cmath>

P2PaletteSelector::P2PaletteSelector(QWidget *parent)
    : QComboBox(parent)
{
    populateList();
}

QColor P2PaletteSelector::color() const
{
    return qvariant_cast<QColor>(itemData(currentIndex(), Qt::DecorationRole));
}

void P2PaletteSelector::setColor(const QColor &color)
{
    setCurrentIndex(findData(color, Qt::DecorationRole));
}

static bool color_compare(int i1, int i2)
{
    const QColor c1 = QColor(P2Colors.color_at(i1));
    const QColor c2 = QColor(P2Colors.color_at(i2));
    const int val1 = (c1.hue() << 16) | (c1.lightness() << 8) | c1.saturation();
    const int val2 = (c2.hue() << 16) | (c2.lightness() << 8) | c2.saturation();
    return val1 < val2;
}

void P2PaletteSelector::populateList()
{
    QStringList color_names = P2Colors.named_colors();
    QVector<QColor> colors(color_names.size());
    QVector<int> index;
    for (int i = 0; i < color_names.size(); ++i) {
        QColor color = QColor(P2Colors.color(color_names[i]));
        if (!colors.contains(color)) {
            colors[i] = color;
            index.append(i);
        }
    }

    // Sort by base colors and ascending luminance
    std::stable_sort(index.begin(), index.end(), color_compare);

    for (int i = 0; i < index.size(); ++i) {
        const int idx = index[i];
        insertItem(i, color_names[idx]);
        setItemData(i, colors[idx], Qt::DecorationRole);
    }
}

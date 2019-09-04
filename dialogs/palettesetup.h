#pragma once
#include <QDialog>
#include "p2colors.h"

namespace Ui {
class PaletteSetup;
}

class QAbstractButton;

class PaletteSetup : public QDialog
{
    Q_OBJECT

public:
    explicit PaletteSetup(QWidget *parent = nullptr);
    ~PaletteSetup() override;

    void setFont(const QFont& font);

signals:
    void changedPalette();

protected slots:
    void reject() override;

private slots:
    void syntax_changed(int idx = 0);
    void color_changed(int idx);
    void clicked(QAbstractButton* button);
    void apply_palette();
    void original_palette();
    void reset_palette();

private:
    Ui::PaletteSetup *ui;
    void setup_combo_boxes();
    void reinit_combo_boxes();
    QHash<P2Colors::p2_palette_e, QColor> m_original_palette;
    QHash<P2Colors::p2_palette_e, QColor> m_modified_palette;
};

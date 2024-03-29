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
    void restore_default_palette();

private:
    Ui::PaletteSetup *ui;
    void setup_buttons();
    void setup_combo_boxes();
    void reinit_combo_boxes();
    p2_palette_hash_t m_original_palette;
    p2_palette_hash_t m_modified_palette;
};

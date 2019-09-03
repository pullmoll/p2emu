#pragma once
#include <QDialog>

namespace Ui {
class PaletteSetup;
}

class PaletteSetup : public QDialog
{
    Q_OBJECT

public:
    explicit PaletteSetup(QWidget *parent = nullptr);
    ~PaletteSetup();

    void setFont(const QFont& font);

private slots:
    void cellChanged(int row, int column);

private:
    Ui::PaletteSetup *ui;
    void setup_table();
    QVector<QString> m_columns;
    QVector<QVariantList> m_rows;
};

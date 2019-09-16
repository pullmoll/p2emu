#include <QtWidgets>
#include <QTableWidget>
#include "palettesetup.h"
#include "ui_palettesetup.h"

PaletteSetup::PaletteSetup(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PaletteSetup)
    , m_original_palette(Colors.palette_hash())
    , m_modified_palette(Colors.palette_hash())
{
    ui->setupUi(this);
    setup_buttons();
    setup_combo_boxes();
    syntax_changed(0);
}

PaletteSetup::~PaletteSetup()
{
    if (result() == QDialog::Rejected)
        original_palette();
    delete ui;
}

void PaletteSetup::setFont(const QFont& font)
{
    Q_UNUSED(font)
}

void PaletteSetup::reject()
{
    // Ignore
}

void PaletteSetup::syntax_changed(int syntax_row)
{
    QVariant v_pal = ui->cb_syntax->itemData(syntax_row, Qt::UserRole);
    p2_palette_e pal = qvariant_cast<p2_palette_e>(v_pal);
    QColor color = Colors.palette_color(pal);

    const int color_row = ui->cb_colors->findData(color, Qt::DecorationRole);
    ui->cb_colors->setCurrentIndex(color_row);
}

void PaletteSetup::color_changed(int color_row)
{
    QVariant v_col = ui->cb_colors->itemData(color_row, Qt::UserRole);
    p2_color_e col = qvariant_cast<p2_color_e>(v_col);
    QRgb rgba = Colors.rgba(col);

    const int w = 12, h = 20;

    // Create a small image to be drawn in the lbl_color
    QImage img(w,h,QImage::Format_ARGB32);
    img.fill(rgba);
    for (int x = 0; x < w; x++) {
        img.setPixel(x,0,Qt::white);
        img.setPixel(x,h-1,Qt::gray);
    }
    for (int y = 0; y < h; y++) {
        img.setPixel(0,y,Qt::white);
        img.setPixel(w-1,y,Qt::darkGray);
    }

    ui->lbl_color->setPixmap(QPixmap::fromImage(img));
    const int syntax_row = ui->cb_syntax->currentIndex();
    QVariant v_pal = ui->cb_syntax->itemData(syntax_row, Qt::UserRole);
    p2_palette_e pal = qvariant_cast<p2_palette_e>(v_pal);

    m_modified_palette.insert(pal, col);
    Colors.set_palette_color(pal, col);
    emit changedPalette();
}

void PaletteSetup::clicked(QAbstractButton* button)
{
    QDialogButtonBox::StandardButton sb = ui->buttonBox->standardButton(button);
    switch (sb) {
    case QDialogButtonBox::Ok:
        apply_palette();
        QDialog::accept();
        break;
    case QDialogButtonBox::Cancel:
        original_palette();
        QDialog::reject();
        break;
    case QDialogButtonBox::Reset:
        original_palette();
        break;
    case QDialogButtonBox::RestoreDefaults:
        restore_default_palette();
        break;
    default:
        break;
    }
}

void PaletteSetup::apply_palette()
{
    Colors.set_palette(m_modified_palette);
    reinit_combo_boxes();
    emit changedPalette();
}

void PaletteSetup::original_palette()
{
    m_modified_palette = m_original_palette;
    Colors.set_palette(m_modified_palette);
    emit changedPalette();
    reinit_combo_boxes();
}

void PaletteSetup::restore_default_palette()
{
    m_modified_palette = Colors.palette_hash(true);
    Colors.set_palette(m_modified_palette);
    emit changedPalette();
    reinit_combo_boxes();
}

void PaletteSetup::setup_buttons()
{
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(clicked(QAbstractButton*)));
}

void PaletteSetup::setup_combo_boxes()
{
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(clicked(QAbstractButton*)));

    // setup syntax names combo box
    QList<QPair<p2_palette_e,QString>> list = {
        {p2_pal_background,   tr("Background")},
        {p2_pal_comment,      tr("Comment")},
        {p2_pal_instruction,  tr("Instruction")},
        {p2_pal_conditional,  tr("Conditional")},
        {p2_pal_wcz_suffix,   tr("WCZ suffix")},
        {p2_pal_section,      tr("Section")},
        {p2_pal_modcz_param,  tr("MODCZ parameter")},
        {p2_pal_symbol,       tr("Symbol")},
        {p2_pal_locsym,       tr("Local symbol")},
        {p2_pal_expression,   tr("Expression")},
        {p2_pal_str_const,    tr("String constant")},
        {p2_pal_bin_const,    tr("Binary constant")},
        {p2_pal_byt_const,    tr("Byte constant")},
        {p2_pal_dec_const,    tr("Decimal constant")},
        {p2_pal_hex_const,    tr("Hexadecimal constant")},
        {p2_pal_real_const,   tr("Real constant")},
        {p2_pal_source,       tr("Source")},
    };

    ui->cb_syntax->setUpdatesEnabled(false);
    ui->cb_syntax->clear();
    for (int i = 0; i < list.count(); i++) {
        QPair<p2_palette_e,QString> pair = list[i];
        p2_palette_e pal = pair.first;
        QString name = pair.second;
        QString key = Colors.palette_name(pal);

        ui->cb_syntax->addItem(name);
        ui->cb_syntax->setItemData(i, key, Qt::ToolTipRole);
        ui->cb_syntax->setItemData(i, pal, Qt::UserRole);
    }
    connect(ui->cb_syntax, SIGNAL(currentIndexChanged(int)), SLOT(syntax_changed(int)));

    // create a new QTableWidget for the color names combo box
    ui->cb_colors->setUpdatesEnabled(false);
    QStringList colors = Colors.color_names(true);

    QTableWidget *table = new QTableWidget(colors.count(), 2);
    table->setHorizontalHeaderLabels({tr("RGB"), tr("Color")});
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->resize(150, 50);

    for (int i = 0; i < colors.count(); ++i) {
        QString colorname = colors[i];
        p2_color_e col = Colors.color_key(colorname);
        QColor color = Colors.color(col);

        QTableWidgetItem *it_color = new QTableWidgetItem(colorname);
        it_color->setData(Qt::DecorationRole, color);
        it_color->setData(Qt::UserRole, col);

        QTableWidgetItem *it_name = new QTableWidgetItem(colorname);
        it_name->setData(Qt::UserRole, col);

        table->setItem(i, 0, it_color);
        table->setItem(i, 1, it_name);
    }
    table->resizeColumnToContents(0);

    // set the new QTableWidget
    ui->cb_colors->setModel(table->model());
    ui->cb_colors->setView(table);
    connect(ui->cb_colors, SIGNAL(currentIndexChanged(int)), SLOT(color_changed(int)));

    // Not used as dialog, but QWidget in a layout
    // setWindowTitle(tr("Propeller2 Syntax Palette Editor"));
    ui->cb_syntax->updateGeometry();
    ui->cb_syntax->setUpdatesEnabled(true);
    ui->cb_colors->setUpdatesEnabled(true);
}

void PaletteSetup::reinit_combo_boxes()
{
    const int idx = ui->cb_syntax->currentIndex();
    setup_combo_boxes();
    ui->cb_syntax->setCurrentIndex(idx);
}

#include <QtWidgets>
#include <QTableWidget>
#include "palettesetup.h"
#include "ui_palettesetup.h"

PaletteSetup::PaletteSetup(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PaletteSetup)
    , m_original_palette(Colors.palette())
    , m_modified_palette(m_original_palette)
{
    ui->setupUi(this);
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

void PaletteSetup::syntax_changed(int row)
{
    QVariant var = ui->cb_syntax->itemData(row, Qt::UserRole);
    P2Colors::p2_palette_e pal = qvariant_cast<P2Colors::p2_palette_e>(var);
    QColor color(Colors.palette_color(pal, false));

    const int idx = ui->cb_colors->findData(color, Qt::DecorationRole);
    ui->cb_colors->setCurrentIndex(idx);
    color_changed(idx);
}

void PaletteSetup::color_changed(int row)
{
    QVariant v_color = ui->cb_colors->itemData(row, Qt::DecorationRole);
    QColor color = qvariant_cast<QColor>(v_color);

    const int w = 12, h = 12;

    // Create a small image to be drawn in the lbl_color
    QImage img(w,h,QImage::Format_ARGB32);
    img.fill(Qt::white);
    for (int y = 1; y < h-1; y++)
        for (int x = 1; x < w-1; x++)
            img.setPixel(x,y,color.rgba());
    for (int x = 1; x < w; x++)
        img.setPixel(x,h-1,Qt::gray);
    for (int y = 1; y < h; y++)
        img.setPixel(w-1,y,Qt::darkGray);

    ui->lbl_color->setPixmap(QPixmap::fromImage(img));
    Q_UNUSED(row)

    const int idx = ui->cb_syntax->currentIndex();
    QVariant v_pal = ui->cb_syntax->itemData(idx, Qt::UserRole);
    P2Colors::p2_palette_e pal = qvariant_cast<P2Colors::p2_palette_e>(v_pal);
    m_modified_palette.insert(pal, color);
    Colors.set_palette_color(pal, color);
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
    case QDialogButtonBox::Discard:
        reset_palette();
        break;
    case QDialogButtonBox::Reset:
        original_palette();
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

void PaletteSetup::reset_palette()
{
    m_modified_palette = m_original_palette = Colors.palette(true);
    emit changedPalette();
    reinit_combo_boxes();
}

void PaletteSetup::setup_combo_boxes()
{
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(clicked(QAbstractButton*)));

    // setup syntax names combo box
    QMap<P2Colors::p2_palette_e,QString> map = {
        {P2Colors::p2_pal_comment,       tr("Comment")},
        {P2Colors::p2_pal_instruction,   tr("Instruction")},
        {P2Colors::p2_pal_conditional,   tr("Conditional")},
        {P2Colors::p2_pal_wcz_suffix,    tr("WCZ suffix")},
        {P2Colors::p2_pal_section,       tr("Section")},
        {P2Colors::p2_pal_modcz_param,   tr("MODCZ parameter")},
        {P2Colors::p2_pal_symbol,        tr("Symbol")},
        {P2Colors::p2_pal_locsym,        tr("Local symbol")},
        {P2Colors::p2_pal_expression,    tr("Expression")},
        {P2Colors::p2_pal_str_const,     tr("String constant")},
        {P2Colors::p2_pal_bin_const,     tr("Binary constant")},
        {P2Colors::p2_pal_byt_const,     tr("Byte constant")},
        {P2Colors::p2_pal_dec_const,     tr("Decimal constant")},
        {P2Colors::p2_pal_hex_const,     tr("Hexadecimal constant")},
        {P2Colors::p2_pal_real_const,    tr("Real constant")},
        {P2Colors::p2_pal_source,        tr("Source")},
    };

    ui->cb_syntax->setUpdatesEnabled(false);
    ui->cb_syntax->clear();
    foreach(P2Colors::p2_palette_e pal, map.keys()) {
        QString key = Colors.palette_name(pal);
        QString name = map.value(pal);
        int idx = ui->cb_syntax->count();

        ui->cb_syntax->addItem(name);
        ui->cb_syntax->setItemData(idx, key, Qt::ToolTipRole);
        ui->cb_syntax->setItemData(idx, pal, Qt::UserRole);
    }
    connect(ui->cb_syntax, SIGNAL(currentIndexChanged(int)), SLOT(syntax_changed(int)));
    ui->cb_syntax->updateGeometry();
    ui->cb_syntax->setUpdatesEnabled(true);

    // create a new QTableWidget for the color names combo box
    QStringList colors = Colors.color_names(true);
    QTableWidget *table = new QTableWidget(colors.count(), 2);
    table->setHorizontalHeaderLabels({tr("Color"), tr("#RRGGBB")});
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->resize(150, 50);

    for (int i = 0; i < colors.count(); ++i) {
        QString name = colors[i];
        QColor color = Colors.color(name);
        QTableWidgetItem *it_name = new QTableWidgetItem(name);
        it_name->setData(Qt::DecorationRole, color);
        table->setItem(i, 0, it_name);
        QTableWidgetItem *it_color = new QTableWidgetItem(color.name());
        table->setItem(i, 1, it_color);
    }
    table->resizeColumnToContents(0);

    // set the new QTableWidget
    ui->cb_colors->setModel(table->model());
    ui->cb_colors->setView(table);
    connect(ui->cb_colors, SIGNAL(currentIndexChanged(int)), SLOT(color_changed(int)));

    // Not used as dialog, but QWidget in a layout
    // setWindowTitle(tr("Propeller2 Syntax Palette Editor"));
}

void PaletteSetup::reinit_combo_boxes()
{
    const int idx = ui->cb_syntax->currentIndex();
    setup_combo_boxes();
    ui->cb_syntax->setCurrentIndex(idx);
}

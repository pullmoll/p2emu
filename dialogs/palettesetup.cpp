#include <QtWidgets>
#include "p2colors.h"
#include "p2paletteselector.h"
#include "palettesetup.h"
#include "ui_palettesetup.h"

PaletteSetup::PaletteSetup(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PaletteSetup)
    , m_columns()
    , m_rows()
{
    ui->setupUi(this);

    m_columns = {
        tr("Syntax element"),    tr("Color"),               tr("Example")
    };

    m_rows = {
        {P2Colors::p2_pal_source,        tr("Source"),                  QLatin1String("Default color")},
        {P2Colors::p2_pal_comment,       tr("Comment"),                 QLatin1String("' get COG identifier")},
        {P2Colors::p2_pal_str_const,     tr("String constant"),         QLatin1String("\"Hello world\"")},
        {P2Colors::p2_pal_bin_const,     tr("Binary constant"),         QLatin1String("%1010_0111")},
        {P2Colors::p2_pal_byt_const,     tr("Byte constant"),           QLatin1String("%%2")},
        {P2Colors::p2_pal_dec_const,     tr("Decimal constant"),        QLatin1String("5336")},
        {P2Colors::p2_pal_hex_const,     tr("Hexadecimal constant"),    QLatin1String("$7cf8")},
        {P2Colors::p2_pal_real_const,    tr("Real constant"),           QLatin1String("80_000_000.5")},
        {P2Colors::p2_pal_locsym,        tr("Local symbol"),            QLatin1String(".loop")},
        {P2Colors::p2_pal_symbol,        tr("Symbol"),                  QLatin1String("SendText")},
        {P2Colors::p2_pal_expression,    tr("Expression"),              QLatin1String("(val + 17)/4")},
        {P2Colors::p2_pal_section,       tr("Section"),                 QLatin1String("DAT")},
        {P2Colors::p2_pal_conditional,   tr("Conditional"),             QLatin1String("if_c_or_z")},
        {P2Colors::p2_pal_modzc_param,   tr("MODCZ parameter"),         QLatin1String("_c_and_z")},
        {P2Colors::p2_pal_instruction,   tr("Instruction"),             QLatin1String("COGID")},
        {P2Colors::p2_pal_wcz_suffix,    tr("WCZ suffix"),              QLatin1String("WCZ")}
    };

    setup_table();
}

PaletteSetup::~PaletteSetup()
{
    delete ui;
}

void PaletteSetup::setFont(const QFont& font)
{
    for (int i = 0; i < ui->twPalette->rowCount(); i++) {
        for (int column = 0; column < ui->twPalette->columnCount(); column++) {
            QTableWidgetItem* it = ui->twPalette->item(i, column);
            it->setData(Qt::FontRole, font);
        }
    }
}

void PaletteSetup::cellChanged(int row, int column)
{
    if (column != 1)
        return;
    QTableWidgetItem* it = ui->twPalette->item(row, column);
    QColor color = QColor(it->data(Qt::DisplayRole).toString());
    ui->twPalette->item(row, 2)->setData(Qt::ForegroundRole, color);
}

void PaletteSetup::setup_table()
{
    QItemEditorFactory *factory = new QItemEditorFactory;
    QItemEditorCreatorBase *paletteEditorCreator = new QStandardItemEditorCreator<P2PaletteSelector>();
    factory->registerEditor(QVariant::Color, paletteEditorCreator);
    QItemEditorFactory::setDefaultFactory(factory);

    QTableWidget *table = ui->twPalette;
    table->setColumnCount(m_columns.count());
    table->setRowCount(m_rows.count());
    table->setHorizontalHeaderLabels(m_columns.toList());
    table->verticalHeader()->setVisible(false);

    for (int i = 0; i < m_rows.count(); ++i) {
        const QVariantList &row = m_rows.at(i);

        QColor color = P2Colors.palette(qvariant_cast<P2Colors::p2_palette_e>(row[0]), false);
        QTableWidgetItem *nameItem = new QTableWidgetItem(row[1].toString());
        QTableWidgetItem *colorItem = new QTableWidgetItem;
        QTableWidgetItem *fgcolorItem = new QTableWidgetItem(row[2].toString());
        colorItem->setData(Qt::DisplayRole, color);
        fgcolorItem->setData(Qt::ForegroundRole, color);

        table->setItem(i, 0, nameItem);
        table->setItem(i, 1, colorItem);
        table->setItem(i, 2, fgcolorItem);
    }
    table->resizeColumnToContents(0);
    table->resizeColumnToContents(1);
    table->resizeColumnToContents(2);
    table->horizontalHeader()->setStretchLastSection(true);

    setWindowTitle(tr("Propeller2 syntax palette editor"));

    connect(table, SIGNAL(cellChanged(int,int)), SLOT(cellChanged(int,int)));
    setMinimumSize(400, 400);
}

#include "p2asmlisting.h"
#include "ui_p2asmlisting.h"

P2AsmListing::P2AsmListing(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::P2AsmListing)
{
    ui->setupUi(this);
}

P2AsmListing::~P2AsmListing()
{
    delete ui;
}

void P2AsmListing::setListing(const QStringList& listing)
{
    ui->tb_listing->setText(listing.join(QChar::LineFeed));
}

void P2AsmListing::setSymbols(const P2AsmSymTbl& table)
{
    ui->tb_symbols->clear();
    foreach(const P2AsmSymbol& sym, table.symbols()) {
        QString name = sym.name();
        P2Atom val = sym.value<P2Atom>();
        QString line = QString("%1 %2 %3")
                       .arg(name, -40)
                       .arg(val.type_name())
                       .arg(val.to_long(), 8, 16, QChar(0));
    }
}

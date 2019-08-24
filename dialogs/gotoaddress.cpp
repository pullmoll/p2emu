#include "gotoaddress.h"
#include "ui_gotoaddress.h"

GotoAddress::GotoAddress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GotoAddress)
{
    ui->setupUi(this);
    ui->lineEdit->selectAll();
}

GotoAddress::~GotoAddress()
{
    delete ui;
}

QString GotoAddress::address()
{
    return ui->lineEdit->text();
}

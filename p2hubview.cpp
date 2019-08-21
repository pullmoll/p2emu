#include "p2hubview.h"
#include "ui_p2hubview.h"

P2HubView::P2HubView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::P2HubView)
{
    ui->setupUi(this);
}

P2HubView::~P2HubView()
{
    delete ui;
}

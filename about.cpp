#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
}

About::~About()
{
    delete ui;
}

void About::setApplicationName(const QString& name)
{
    ui->label_title->setText(name);
}

void About::setApplicationVersion(const QString& version)
{
    ui->label_version->setText(version);
}

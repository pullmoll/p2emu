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

void About::setApplicationNameVersion(const QString& name, const QString& version)
{
    setWindowTitle(QString("%1 %2").arg(name).arg(version));
    ui->label_title->setText(name);
    ui->label_version->setText(version);
}

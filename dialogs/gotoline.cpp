#include "gotoline.h"
#include "ui_gotoline.h"
#include <QSettings>

GotoLine::GotoLine(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GotoLine)
{
    ui->setupUi(this);
    QSettings s;
    s.beginGroup(objectName());
    int number = s.value(QStringLiteral("number")).toInt();
    s.endGroup();
    set_umber(number);
}

GotoLine::~GotoLine()
{
    delete ui;
}

int GotoLine::number() const
{
    bool ok;
    const int number = ui->le_line->text().toInt(&ok);
    return ok ? number : 1;
}

void GotoLine::set_umber(int value)
{
    ui->le_line->setText(QString::number(value));
    ui->le_line->selectAll();
}

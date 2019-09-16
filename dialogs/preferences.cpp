#include "preferences.h"
#include "ui_preferences.h"

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);
}

Preferences::~Preferences()
{
    delete ui;
}

bool Preferences::pnut() const
{
    return ui->cb_pnut->isChecked();
}

bool Preferences::v33mode() const
{
    return ui->cb_v33mode->isChecked();
}

bool Preferences::file_errors() const
{
    return ui->cb_file_errors->isChecked();
}

const QFont Preferences::font_asm() const
{
    return ui->cb_font_asm->currentFont();
}

const QFont Preferences::font_dasm() const
{
    return ui->cb_font_dasm->currentFont();
}

void Preferences::set_pnut(bool on)
{
    ui->cb_pnut->setChecked(on);
}

void Preferences::set_v33mode(bool on)
{
    ui->cb_v33mode->setChecked(on);
}

void Preferences::set_file_errors(bool on)
{
    ui->cb_file_errors->setChecked(on);
}

void Preferences::set_font_asm(const QFont& font)
{
    ui->cb_font_asm->setCurrentFont(font);
}

void Preferences::set_font_dasm(const QFont& font)
{
    ui->cb_font_dasm->setCurrentFont(font);
}

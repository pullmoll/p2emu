#include "textbrowser.h"
#include "ui_textbrowser.h"

TextBrowser::TextBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::P2AsmListing)
{
    ui->setupUi(this);
}

TextBrowser::~TextBrowser()
{
    delete ui;
}

void TextBrowser::set_list(const QStringList& list)
{
    ui->tb_listing->setText(list.join(QChar::LineFeed));
}

void TextBrowser::set_html(const QStringList& html)
{
    ui->tb_listing->setHtml(html.join(QChar::LineFeed));
}

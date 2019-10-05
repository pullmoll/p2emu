#pragma once
#include <QDialog>
#include <QString>
#include "p2asm.h"

namespace Ui {
class P2AsmListing;
}

class TextBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit TextBrowser(QWidget *parent = nullptr);
    ~TextBrowser();

    void set_list(const QStringList& list);
    void set_html(const QStringList& html);

private:
    Ui::P2AsmListing *ui;
};

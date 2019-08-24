#pragma once
#include <QDialog>
#include <QString>
#include "p2asm.h"

namespace Ui {
class P2AsmListing;
}

class P2AsmListing : public QDialog
{
    Q_OBJECT

public:
    explicit P2AsmListing(QWidget *parent = nullptr);
    ~P2AsmListing();

    void setListing(const QStringList& listing);
    void setSymbols(const P2AsmSymTbl& symbols);
private:
    Ui::P2AsmListing *ui;
};

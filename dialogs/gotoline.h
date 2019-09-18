#pragma once
#include <QDialog>

namespace Ui {
class GotoLine;
}

class GotoLine : public QDialog
{
    Q_OBJECT

public:
    explicit GotoLine(QWidget *parent = nullptr);
    ~GotoLine();

    int number() const;

public slots:
    void set_umber(int value);

private:
    Ui::GotoLine *ui;
};

#pragma once
#include <QDialog>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();

    void setApplicationNameVersion(const QString& name, const QString& version);

private:
    Ui::About *ui;
};

#ifndef GOTOADDRESS_H
#define GOTOADDRESS_H

#include <QDialog>

namespace Ui {
class GotoAddress;
}

class GotoAddress : public QDialog
{
    Q_OBJECT

public:
    explicit GotoAddress(QWidget *parent = nullptr);
    ~GotoAddress();

    QString address();

private:
    Ui::GotoAddress *ui;
};

#endif // GOTOADDRESS_H

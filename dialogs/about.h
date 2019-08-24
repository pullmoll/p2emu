#ifndef ABOUT_H
#define ABOUT_H

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

    void setApplicationName(const QString& name);
    void setApplicationVersion(const QString& version);

private:
    Ui::About *ui;
};

#endif // ABOUT_H

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = nullptr);
    ~Preferences();

    bool pnut() const;
    bool v33mode() const;
    bool file_errors() const;
    const QFont font_asm() const;
    const QFont font_dasm() const;

public slots:
    void set_pnut(bool on = true);
    void set_v33mode(bool on = true);
    void set_file_errors(bool on = true);
    void set_font_asm(const QFont& font);
    void set_font_dasm(const QFont& font);

private:
    Ui::Preferences *ui;
};

#endif // PREFERENCES_H

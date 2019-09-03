#pragma once
#include <QObject>
#include <QWidget>
#include <QComboBox>

class P2PaletteSelector : public QComboBox
{
public:
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor USER true)

public:
    P2PaletteSelector(QWidget *widget = nullptr);

    QColor color() const;
    void setColor(const QColor &color);

private:
    void populateList();
};

#ifndef P2HUBVIEW_H
#define P2HUBVIEW_H

#include <QWidget>

namespace Ui {
class P2HubView;
}

class P2HubView : public QWidget
{
    Q_OBJECT

public:
    explicit P2HubView(QWidget *parent = nullptr);
    ~P2HubView();

private:
    Ui::P2HubView *ui;
};

#endif // P2HUBVIEW_H

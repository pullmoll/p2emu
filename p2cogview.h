#ifndef P2COGVIEW_H
#define P2COGVIEW_H

#include <QWidget>

namespace Ui {
class P2CogView;
}

class P2Cog;
class P2Dasm;
class P2DasmModel;

class P2CogView : public QWidget
{
    Q_OBJECT

public:
    explicit P2CogView(QWidget *parent = nullptr);
    ~P2CogView();

    void updateView();
    void setCog(const P2Cog* cog);

private:
    Ui::P2CogView* ui;
    const P2Cog* m_cog;
    P2Dasm* m_dasm;
    P2DasmModel* m_model;
};

#endif // P2COGVIEW_H

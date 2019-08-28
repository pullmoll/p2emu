#include <QPainter>
#include <QComboBox>
#include <QStandardItemModel>
#include "p2symbolsmodel.h"
#include "p2referencesdelegate.h"

P2ReferencesDelegate::P2ReferencesDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

/**
 * @brief P2ReferencesDelegate::createEditor
 *
 * Returns the editor to be used for editing the data item with the given index.
 * Note that the index contains information about the model being used.
 *
 * The editor's parent widget is specified by parent, and the item options by option.
 * The base implementation returns nullptr. If you want custom editing you will
 * need to reimplement this function.
 *
 * The returned editor widget should have Qt::StrongFocus; otherwise, QMouseEvents
 * received by the widget will propagate to the view.
 *
 * The view's background will shine through unless the editor paints its
 * own background (e.g., with setAutoFillBackground()).
 *
 * @param parent
 * @param option
 * @param index
 * @return
 */
QWidget* P2ReferencesDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QComboBox* cb = new QComboBox(parent);
    cb->setFocusPolicy(Qt::StrongFocus);
    connect(cb, SIGNAL(currentIndexChanged(int)), SLOT(indexChanged(int)));
    return cb;
}

void P2ReferencesDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const P2SymbolsModel* model = qobject_cast<const P2SymbolsModel*>(index.model());
    Q_ASSERT(model);    // assert the model is really P2SymbolsModel
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    if (!cb)
        return;

    cb->setUpdatesEnabled(false);
    cb->clear();
    QVariant data = model->data(index, Qt::EditRole);
    QVariantList list = data.toList();
    QStringList texts;

    // Don't list the definition
    list.removeFirst();
    if (list.isEmpty()) {
        cb->setUpdatesEnabled(true);
        return;
    }

    cb->addItem(tr("%1 refs").arg(list.count()));
    qobject_cast<QStandardItemModel*>(cb->model())->item(0)->setEnabled(false);
    foreach(const QVariant& var, list) {
        const QString text = tr("Line #%1").arg(var.toString());
        QUrl url(QString("%1#%2")
                 .arg(key_tv_asm)
                 .arg(var.toString()));
        url.setFragment(var.toString());
        cb->addItem(text, url);
    }
    cb->setMaxVisibleItems(10);
    cb->setUpdatesEnabled(true);
}

void P2ReferencesDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    if (!cb)
        return;
    // model->setData(index, cb->currentText(), Qt::EditRole);
    emit urlSelected(cb->itemData(cb->currentIndex()).toUrl());
}

void P2ReferencesDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    if (!cb)
        return;
    cb->setGeometry(option.rect);
}

void P2ReferencesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const P2SymbolsModel* model = qobject_cast<const P2SymbolsModel*>(index.model());
    Q_ASSERT(model);    // assert the model is really P2SymbolsModel
    QVariant var = model->data(index, Qt::EditRole);
    QVariantList list = var.toList();
    QStyleOptionViewItem opt(option);
    list.removeFirst();
    if (list.count() < 1) {
        opt.text = tr("%1 refs").arg(tr("no"));
    } else {
        opt.text = tr("%1 refs").arg(list.count());
    }
    painter->drawText(opt.rect, opt.text);
}

void P2ReferencesDelegate::indexChanged(int i) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(sender());
    if (!cb)
        return;
    emit urlSelected(cb->itemData(i).toUrl());
}

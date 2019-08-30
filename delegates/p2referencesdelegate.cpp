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
    const P2SymbolsModel* model = qobject_cast<const P2SymbolsModel*>(index.model());
    Q_ASSERT(model);    // assert the model is really P2SymbolsModel
    Q_UNUSED(option)
    Q_UNUSED(index)
    QComboBox* cb = new QComboBox(parent);
    cb->setFont(qvariant_cast<QFont>(model->data(index, Qt::FontRole)));
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

    if (list.isEmpty()) {
        cb->setUpdatesEnabled(true);
        return;
    }

    cb->addItem(tr("%1 refs").arg(list.count()));
    qobject_cast<QStandardItemModel*>(cb->model())->item(0)->setEnabled(false);
    foreach(const QVariant& var, list) {
        const P2Word& word = qvariant_cast<P2Word>(var);
        const QString text = tr("Line #%1").arg(word.lineno());
        cb->addItem(text, P2Word::url(word));
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
    Q_UNUSED(model)
    Q_UNUSED(index)
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
    QString text;
    int flags = Qt::AlignRight | Qt::AlignVCenter |
                Qt::TextSingleLine | Qt::TextDontClip;

    if (list.count() < 1) {
        text = tr("%1 refs").arg(tr("no"));
    } else {
        text = tr("%1 refs").arg(list.count());
    }
    painter->drawText(opt.rect, flags, text);
}

void P2ReferencesDelegate::indexChanged(int i) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(sender());
    if (!cb)
        return;
    emit urlSelected(cb->itemData(i).toUrl());
}

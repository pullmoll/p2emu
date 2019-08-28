#pragma once
#include <QStyledItemDelegate>
#include <QUrl>

class P2ReferencesDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    P2ReferencesDelegate(QObject* parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void urlSelected(const QUrl& url) const;

private slots:
    void indexChanged(int i) const;
};

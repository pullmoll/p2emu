#pragma once
#include <QAbstractTableModel>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QIcon>

class P2Params;

class P2AsmModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum column_e {
        c_Origin,
        c_Tokens,
        c_Opcode,
        c_Symbols,
        c_Errors,
        c_Source,
    };
    enum format_e {
        f_bin,
        f_hex,
        f_oct
    };

    explicit P2AsmModel(P2Params* params, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    format_e opcode_format() const { return m_opcode_format; }
    QSize sizeHint(column_e column) const { return m_size_normal.value(column); }

public slots:
    void invalidate();
    void setOpcodeFormat(format_e format);
    void updateSizes();

private:
    P2Params* m_params;
    format_e m_opcode_format;
    QFont m_font;
    QFont m_bold;
    QIcon m_error;
    QHash<column_e,QString> m_header;
    QHash<column_e,QSize> m_size_normal;
    QHash<column_e,QSize> m_size_bold;
    QHash<column_e,QRgb> m_background;
    QHash<column_e,int> m_header_alignment;
    QHash<column_e,int> m_text_alignment;
};

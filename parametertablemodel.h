#ifndef PARAMETERTABLEMODEL_H
#define PARAMETERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "parametercodec.h"

class ParameterTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        EnabledColumn = 0,
        KeyColumn,
        TypeColumn,
        ValueColumn,
        DescriptionColumn,
        ColumnCount
    };

    explicit ParameterTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setRows(const QList<ParameterRow> &rows);
    QList<ParameterRow> rows() const;
    void addRow(const ParameterRow &row);
    void removeRow(int row);

signals:
    void rowsChanged(const QList<ParameterRow> &rows);

private:
    void emitRowsChanged();

    QList<ParameterRow> m_rows;
};

#endif // PARAMETERTABLEMODEL_H

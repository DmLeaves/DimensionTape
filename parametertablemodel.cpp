#include "parametertablemodel.h"

ParameterTableModel::ParameterTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ParameterTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rows.size();
}

int ParameterTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant ParameterTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    int row = index.row();
    if (row < 0 || row >= m_rows.size()) {
        return QVariant();
    }

    const ParameterRow &entry = m_rows.at(row);
    int column = index.column();

    if (column == EnabledColumn && role == Qt::CheckStateRole) {
        return entry.enabled ? Qt::Checked : Qt::Unchecked;
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    switch (column) {
    case KeyColumn:
        return entry.key;
    case TypeColumn:
        return ParameterCodec::valueTypeToString(entry.type);
    case ValueColumn:
        return entry.value;
    case DescriptionColumn:
        return entry.description;
    case EnabledColumn:
    default:
        return QVariant();
    }
}

bool ParameterTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    int row = index.row();
    if (row < 0 || row >= m_rows.size()) {
        return false;
    }
    int column = index.column();

    ParameterRow entry = m_rows.at(row);
    bool changed = false;

    if (column == EnabledColumn && role == Qt::CheckStateRole) {
        bool enabled = value.toInt() == Qt::Checked;
        if (entry.enabled != enabled) {
            entry.enabled = enabled;
            changed = true;
        }
    } else if (role == Qt::EditRole) {
        switch (column) {
        case KeyColumn:
            if (entry.key != value.toString()) {
                entry.key = value.toString();
                changed = true;
            }
            break;
        case TypeColumn: {
            ParameterValueType type = ParameterCodec::valueTypeFromString(value.toString());
            if (entry.type != type) {
                entry.type = type;
                changed = true;
            }
            break;
        }
        case ValueColumn:
            if (entry.value != value.toString()) {
                entry.value = value.toString();
                changed = true;
            }
            break;
        case DescriptionColumn:
            if (entry.description != value.toString()) {
                entry.description = value.toString();
                changed = true;
            }
            break;
        default:
            break;
        }
    }

    if (!changed) {
        return false;
    }

    m_rows[row] = entry;

    QVector<int> roles;
    if (column == EnabledColumn) {
        roles << Qt::CheckStateRole;
    } else {
        roles << Qt::DisplayRole << Qt::EditRole;
    }
    emit dataChanged(index, index, roles);
    emitRowsChanged();
    return true;
}

QVariant ParameterTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case EnabledColumn:
            return QString("启用");
        case KeyColumn:
            return QString("键");
        case TypeColumn:
            return QString("类型");
        case ValueColumn:
            return QString("值");
        case DescriptionColumn:
            return QString("说明");
        default:
            return QVariant();
        }
    }
    return QString::number(section + 1);
}

Qt::ItemFlags ParameterTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    switch (index.column()) {
    case EnabledColumn:
        flags |= Qt::ItemIsUserCheckable;
        break;
    case KeyColumn:
    case TypeColumn:
    case ValueColumn:
    case DescriptionColumn:
        flags |= Qt::ItemIsEditable;
        break;
    default:
        break;
    }
    return flags;
}

void ParameterTableModel::setRows(const QList<ParameterRow> &rows)
{
    beginResetModel();
    m_rows = rows;
    endResetModel();
    emitRowsChanged();
}

QList<ParameterRow> ParameterTableModel::rows() const
{
    return m_rows;
}

void ParameterTableModel::addRow(const ParameterRow &row)
{
    int rowIndex = m_rows.size();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    m_rows.append(row);
    endInsertRows();
    emitRowsChanged();
}

void ParameterTableModel::removeRow(int row)
{
    if (row < 0 || row >= m_rows.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_rows.removeAt(row);
    endRemoveRows();
    emitRowsChanged();
}

void ParameterTableModel::emitRowsChanged()
{
    emit rowsChanged(m_rows);
}

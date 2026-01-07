#include "eventlistmodel.h"
#include <QVariant>

EventListModel::EventListModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int EventListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_events.size();
}

int EventListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant EventListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    int row = index.row();
    int column = index.column();
    if (row < 0 || row >= m_events.size()) {
        return QVariant();
    }

    const StickerEvent &event = m_events.at(row);

    if (role == Qt::CheckStateRole && column == EnabledColumn) {
        return event.enabled ? Qt::Checked : Qt::Unchecked;
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    switch (column) {
    case TriggerColumn:
        return mouseTriggersToString(event.trigger);
    case TypeColumn:
        return eventTypeToString(event.type);
    case TargetColumn:
        return event.target;
    case ParametersColumn:
        return event.parametersText();
    case EnabledColumn:
        return QVariant();
    default:
        return QVariant();
    }
}

bool EventListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    int row = index.row();
    int column = index.column();
    if (row < 0 || row >= m_events.size()) {
        return false;
    }

    StickerEvent event = m_events.at(row);
    bool changed = false;

    if (column == EnabledColumn && role == Qt::CheckStateRole) {
        bool enabled = (value.toInt() == Qt::Checked);
        if (event.enabled != enabled) {
            event.enabled = enabled;
            changed = true;
        }
    } else if (role == Qt::EditRole) {
        switch (column) {
        case TriggerColumn: {
            MouseTrigger trigger = stringToMouseTrigger(value.toString());
            if (event.trigger != trigger) {
                event.trigger = trigger;
                changed = true;
            }
            break;
        }
        case TypeColumn: {
            StickerEventType type = stringToEventType(value.toString());
            if (event.type != type) {
                event.type = type;
                changed = true;
            }
            break;
        }
        case TargetColumn:
            if (event.target != value.toString()) {
                event.target = value.toString();
                changed = true;
            }
            break;
        case ParametersColumn:
            if (event.parametersText() != value.toString()) {
                event.setParametersText(value.toString());
                changed = true;
            }
            break;
        case EnabledColumn:
            if (event.enabled != value.toBool()) {
                event.enabled = value.toBool();
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

    m_events[row] = event;

    QVector<int> roles;
    if (column == EnabledColumn) {
        roles << Qt::CheckStateRole;
    } else {
        roles << Qt::DisplayRole << Qt::EditRole;
    }
    emit dataChanged(index, index, roles);
    emitEventsChanged();
    return true;
}

QVariant EventListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case TriggerColumn:
            return QString("触发器");
        case TypeColumn:
            return QString("事件类型");
        case TargetColumn:
            return QString("目标");
        case ParametersColumn:
            return QString("参数");
        case EnabledColumn:
            return QString("启用");
        default:
            return QVariant();
        }
    }
    return QString::number(section + 1);
}

Qt::ItemFlags EventListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    switch (index.column()) {
    case TriggerColumn:
    case TypeColumn:
    case TargetColumn:
    case ParametersColumn:
        flags |= Qt::ItemIsEditable;
        break;
    case EnabledColumn:
        flags |= Qt::ItemIsUserCheckable;
        break;
    default:
        break;
    }
    return flags;
}

void EventListModel::setEvents(const QList<StickerEvent> &events)
{
    beginResetModel();
    m_events = events;
    endResetModel();
}

QList<StickerEvent> EventListModel::events() const
{
    return m_events;
}

void EventListModel::addEvent(const StickerEvent &event)
{
    int row = m_events.size();
    beginInsertRows(QModelIndex(), row, row);
    m_events.append(event);
    endInsertRows();
    emitEventsChanged();
}

void EventListModel::removeEvent(int row)
{
    if (row < 0 || row >= m_events.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_events.removeAt(row);
    endRemoveRows();
    emitEventsChanged();
}

void EventListModel::emitEventsChanged()
{
    emit eventsChanged(m_events);
}

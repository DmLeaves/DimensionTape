#ifndef EVENTLISTMODEL_H
#define EVENTLISTMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "StickerData.h"

class EventListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        TriggerColumn = 0,
        TypeColumn,
        TargetColumn,
        ParametersColumn,
        EnabledColumn,
        ColumnCount
    };

    explicit EventListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setEvents(const QList<StickerEvent> &events);
    QList<StickerEvent> events() const;

    void addEvent(const StickerEvent &event);
    void removeEvent(int row);

signals:
    void eventsChanged(const QList<StickerEvent> &events);

private:
    void emitEventsChanged();

    QList<StickerEvent> m_events;
};

#endif // EVENTLISTMODEL_H

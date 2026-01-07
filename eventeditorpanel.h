#ifndef EVENTEDITORPANEL_H
#define EVENTEDITORPANEL_H

#include <QWidget>
#include "StickerData.h"

class QComboBox;
class QPushButton;
class QTableView;
class EventComboDelegate;
class EventDetailPanel;
class EventListModel;
class EventParameterEditor;

class EventEditorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit EventEditorPanel(QWidget *parent = nullptr);

    void setEvents(const QList<StickerEvent> &events);
    QList<StickerEvent> events() const;
    void setEditingEnabled(bool enabled);
    void clearInputs();

signals:
    void eventsChanged(const QList<StickerEvent> &events);
    void statusMessageRequested(const QString &message, int timeoutMs);

private slots:
    void onAddEventClicked();
    void onRemoveEventClicked();
    void onModelDataChanged();
    void onTypeComboChanged(const QString &text);
    void onSelectionChanged();

private:
    void setupUi();
    void refreshTable();
    void updateDetailForCurrentType();
    void updateDetailForSelectedRow();

    QTableView *m_eventTable = nullptr;
    EventListModel *m_eventModel = nullptr;
    EventComboDelegate *m_triggerDelegate = nullptr;
    EventComboDelegate *m_typeDelegate = nullptr;
    EventDetailPanel *m_detailPanel = nullptr;
    QPushButton *m_addEventBtn = nullptr;
    QPushButton *m_removeEventBtn = nullptr;
    QComboBox *m_triggerComboBox = nullptr;
    QComboBox *m_eventTypeComboBox = nullptr;
    EventParameterEditor *m_parameterEditor = nullptr;
    bool m_editingEnabled = false;
};

#endif // EVENTEDITORPANEL_H

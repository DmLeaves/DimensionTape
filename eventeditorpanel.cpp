#include "eventeditorpanel.h"
#include "eventcombodelegate.h"
#include "eventdetailpanel.h"
#include "eventlistmodel.h"
#include "eventparametereditor.h"
#include "eventtyperegistry.h"
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>

EventEditorPanel::EventEditorPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setEditingEnabled(false);
}

void EventEditorPanel::setEvents(const QList<StickerEvent> &events)
{
    if (m_eventModel) {
        m_eventModel->setEvents(events);
        refreshTable();
        updateDetailForSelectedRow();
    }
}

QList<StickerEvent> EventEditorPanel::events() const
{
    return m_eventModel ? m_eventModel->events() : QList<StickerEvent>();
}

void EventEditorPanel::setEditingEnabled(bool enabled)
{
    m_editingEnabled = enabled;
    if (m_eventTable) {
        m_eventTable->setEnabled(enabled);
    }
    if (m_addEventBtn) {
        m_addEventBtn->setEnabled(enabled);
    }
    if (m_removeEventBtn) {
        m_removeEventBtn->setEnabled(enabled);
    }
    if (m_triggerComboBox) {
        m_triggerComboBox->setEnabled(enabled);
    }
    if (m_eventTypeComboBox) {
        m_eventTypeComboBox->setEnabled(enabled);
    }
    if (m_parameterEditor) {
        m_parameterEditor->setEnabled(enabled);
    }

    if (!enabled && m_detailPanel) {
        m_detailPanel->clear();
    } else if (enabled) {
        updateDetailForSelectedRow();
    }
}

void EventEditorPanel::clearInputs()
{
    if (m_parameterEditor) {
        m_parameterEditor->setValues(QString(), QVariantMap());
    }
}

void EventEditorPanel::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    m_eventTable = new QTableView(splitter);
    m_eventModel = new EventListModel(this);
    m_eventTable->setModel(m_eventModel);
    m_eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_eventTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_eventTable->setAlternatingRowColors(true);
    m_eventTable->horizontalHeader()->setStretchLastSection(true);
    splitter->addWidget(m_eventTable);

    m_detailPanel = new EventDetailPanel(splitter);
    splitter->addWidget(m_detailPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    layout->addWidget(splitter);

    QStringList triggers = {"左键单击", "右键单击", "双击", "滚轮向上", "滚轮向下", "鼠标进入", "鼠标离开"};
    m_triggerDelegate = new EventComboDelegate(triggers, this);
    m_typeDelegate = new EventComboDelegate(EventTypeRegistry::instance().typeDisplayNames(), this);
    m_eventTable->setItemDelegateForColumn(EventListModel::TriggerColumn, m_triggerDelegate);
    m_eventTable->setItemDelegateForColumn(EventListModel::TypeColumn, m_typeDelegate);

    QGroupBox *addEventGroup = new QGroupBox("添加新事件");
    QGridLayout *addLayout = new QGridLayout(addEventGroup);

    addLayout->addWidget(new QLabel("触发器:"), 0, 0);
    m_triggerComboBox = new QComboBox;
    m_triggerComboBox->addItems(triggers);
    addLayout->addWidget(m_triggerComboBox, 0, 1);

    addLayout->addWidget(new QLabel("事件类型:"), 0, 2);
    m_eventTypeComboBox = new QComboBox;
    m_eventTypeComboBox->addItems(EventTypeRegistry::instance().typeDisplayNames());
    addLayout->addWidget(m_eventTypeComboBox, 0, 3);

    m_parameterEditor = new EventParameterEditor(addEventGroup);
    addLayout->addWidget(m_parameterEditor, 1, 0, 1, 4);

    layout->addWidget(addEventGroup);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_addEventBtn = new QPushButton("添加事件");
    m_removeEventBtn = new QPushButton("删除事件");

    buttonLayout->addWidget(m_addEventBtn);
    buttonLayout->addWidget(m_removeEventBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    connect(m_addEventBtn, &QPushButton::clicked, this, &EventEditorPanel::onAddEventClicked);
    connect(m_removeEventBtn, &QPushButton::clicked, this, &EventEditorPanel::onRemoveEventClicked);
    if (m_eventModel) {
        connect(m_eventModel, &EventListModel::eventsChanged,
                this, &EventEditorPanel::eventsChanged);
        connect(m_eventModel, &QAbstractItemModel::dataChanged, this, [this]() {
            onModelDataChanged();
        });
    }
    if (m_eventTable && m_eventTable->selectionModel()) {
        connect(m_eventTable->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, [this]() { onSelectionChanged(); });
    }
    connect(m_eventTypeComboBox, &QComboBox::currentTextChanged,
            this, &EventEditorPanel::onTypeComboChanged);

    onTypeComboChanged(m_eventTypeComboBox->currentText());
}

void EventEditorPanel::refreshTable()
{
    if (!m_eventTable) {
        return;
    }
    m_eventTable->viewport()->update();
}

void EventEditorPanel::onAddEventClicked()
{
    if (!m_editingEnabled) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    StickerEvent event;
    event.trigger = stringToMouseTrigger(m_triggerComboBox->currentText());
    event.type = stringToEventType(m_eventTypeComboBox->currentText());
    event.parameters.clear();
    if (m_parameterEditor) {
        QString error;
        if (!m_parameterEditor->validate(&error)) {
            QMessageBox::warning(this, "警告", error);
            return;
        }
        event.target = m_parameterEditor->target();
        event.parameters = m_parameterEditor->parameters();
    }
    event.enabled = true;

    if (event.target.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入事件目标");
        return;
    }

    if (m_eventModel) {
        m_eventModel->addEvent(event);
    }
    clearInputs();
    emit statusMessageRequested("事件已添加", 2000);
}

void EventEditorPanel::onRemoveEventClicked()
{
    if (!m_editingEnabled) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    int currentRow = m_eventTable ? m_eventTable->currentIndex().row() : -1;
    if (currentRow >= 0 && m_eventModel) {
        m_eventModel->removeEvent(currentRow);
        emit statusMessageRequested("事件已删除", 2000);
        return;
    }

    QMessageBox::warning(this, "警告", "请选择要删除的事件");
}

void EventEditorPanel::onModelDataChanged()
{
    if (!m_editingEnabled) {
        return;
    }
    updateDetailForSelectedRow();
    emit statusMessageRequested("事件已更新", 2000);
}

void EventEditorPanel::onTypeComboChanged(const QString &text)
{
    StickerEventType type = stringToEventType(text);
    EventTypeSpec spec = EventTypeRegistry::instance().specFor(type);
    if (m_parameterEditor) {
        m_parameterEditor->setSpec(spec);
    }
    updateDetailForCurrentType();
}

void EventEditorPanel::onSelectionChanged()
{
    updateDetailForSelectedRow();
}

void EventEditorPanel::updateDetailForCurrentType()
{
    if (!m_detailPanel || !m_eventTypeComboBox) {
        return;
    }
    if (m_eventTable && m_eventTable->selectionModel()
        && m_eventTable->selectionModel()->hasSelection()) {
        return;
    }

    StickerEventType type = stringToEventType(m_eventTypeComboBox->currentText());
    EventTypeSpec spec = EventTypeRegistry::instance().specFor(type);
    m_detailPanel->setSpec(spec);
}

void EventEditorPanel::updateDetailForSelectedRow()
{
    if (!m_detailPanel || !m_eventModel || !m_eventTable || !m_eventTable->selectionModel()) {
        return;
    }

    if (!m_eventTable->selectionModel()->hasSelection()) {
        updateDetailForCurrentType();
        return;
    }

    int row = m_eventTable->selectionModel()->currentIndex().row();
    if (row < 0 || row >= m_eventModel->events().size()) {
        updateDetailForCurrentType();
        return;
    }

    StickerEvent event = m_eventModel->events().at(row);
    EventTypeSpec spec = EventTypeRegistry::instance().specFor(event.type);
    m_detailPanel->setSpec(spec);
}

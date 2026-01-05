#include "StickerRuntime.h"

StickerRuntime::StickerRuntime(QObject *parent)
    : QObject(parent)
{
}

StickerRuntime::~StickerRuntime()
{
    clear();
}

StickerInstance *StickerRuntime::createOrUpdatePrimary(const StickerConfig &config)
{
    if (config.id.isEmpty()) {
        return nullptr;
    }
    return createOrUpdateInstance(config, config.id, config.id, true);
}

StickerInstance *StickerRuntime::createOrUpdateInstance(const StickerConfig &config,
                                                        const QString &instanceId,
                                                        const QString &templateId,
                                                        bool syncToTemplate)
{
    if (instanceId.isEmpty()) {
        return nullptr;
    }
    return ensureInstance(config, instanceId, templateId, syncToTemplate);
}

void StickerRuntime::destroyInstance(const QString &instanceId)
{
    StickerInstance *instance = m_instances.value(instanceId, nullptr);
    if (!instance) {
        return;
    }

    if (instance->widget) {
        instance->widget->disconnect();
        instance->widget->close();
        instance->widget->deleteLater();
        instance->widget = nullptr;
    }

    m_instances.remove(instanceId);
    delete instance;
}

void StickerRuntime::destroyInstancesForTemplate(const QString &templateId)
{
    QList<QString> toRemove;
    for (auto it = m_instances.begin(); it != m_instances.end(); ++it) {
        StickerInstance *instance = it.value();
        if (instance && instance->templateId == templateId) {
            toRemove.append(it.key());
        }
    }
    for (const QString &instanceId : toRemove) {
        destroyInstance(instanceId);
    }
}

void StickerRuntime::clear()
{
    const QList<QString> keys = m_instances.keys();
    for (const QString &key : keys) {
        destroyInstance(key);
    }
}

StickerInstance *StickerRuntime::instance(const QString &instanceId) const
{
    return m_instances.value(instanceId, nullptr);
}

StickerWidget *StickerRuntime::widget(const QString &instanceId) const
{
    StickerInstance *instance = m_instances.value(instanceId, nullptr);
    return instance ? instance->widget : nullptr;
}

QList<StickerInstance*> StickerRuntime::instances() const
{
    return m_instances.values();
}

QList<StickerInstance*> StickerRuntime::instancesForTemplate(const QString &templateId) const
{
    QList<StickerInstance*> result;
    for (StickerInstance *instance : m_instances) {
        if (instance && instance->templateId == templateId) {
            result.append(instance);
        }
    }
    return result;
}

StickerInstance *StickerRuntime::ensureInstance(const StickerConfig &config,
                                                const QString &instanceId,
                                                const QString &templateId,
                                                bool syncToTemplate)
{
    StickerInstance *instance = m_instances.value(instanceId, nullptr);
    if (!instance) {
        instance = new StickerInstance();
        instance->instanceId = instanceId;
        instance->templateId = templateId;
        instance->syncToTemplate = syncToTemplate;
        instance->config = config;
        instance->widget = new StickerWidget(config);
        instance->widget->show();
        connectInstanceSignals(instance);
        m_instances.insert(instanceId, instance);
        instance->config = instance->widget->getConfig();
        return instance;
    }

    instance->templateId = templateId;
    instance->syncToTemplate = syncToTemplate;
    instance->config = config;
    if (instance->widget) {
        instance->widget->updateConfig(config);
        instance->config = instance->widget->getConfig();
    }
    return instance;
}

void StickerRuntime::connectInstanceSignals(StickerInstance *instance)
{
    if (!instance || !instance->widget) {
        return;
    }

    StickerWidget *widget = instance->widget;
    const QString instanceId = instance->instanceId;

    connect(widget, &StickerWidget::configChanged, this,
            [this, instanceId](const StickerConfig &config) {
                StickerInstance *current = m_instances.value(instanceId, nullptr);
                if (!current) {
                    return;
                }
                current->config = config;
                emit instanceConfigChanged(instanceId, config, current->syncToTemplate);
            }, Qt::UniqueConnection);

    connect(widget, &StickerWidget::deleteRequested, this,
            [this, instanceId](const QString &) {
                StickerInstance *current = m_instances.value(instanceId, nullptr);
                if (!current) {
                    return;
                }
                emit instanceDeleteRequested(instanceId, current->templateId);
            }, Qt::UniqueConnection);

    connect(widget, &StickerWidget::editRequested, this,
            [this, instanceId](const QString &) {
                StickerInstance *current = m_instances.value(instanceId, nullptr);
                if (!current) {
                    return;
                }
                emit instanceEditRequested(instanceId, current->templateId);
            }, Qt::UniqueConnection);
}

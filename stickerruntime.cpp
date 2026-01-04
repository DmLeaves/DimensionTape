#include "StickerRuntime.h"

StickerRuntime::StickerRuntime(QObject *parent)
    : QObject(parent)
{
}

StickerRuntime::~StickerRuntime()
{
    clear();
}

StickerWidget *StickerRuntime::createOrUpdate(const StickerConfig &config)
{
    if (config.id.isEmpty()) {
        return nullptr;
    }

    StickerWidget *widget = m_widgets.value(config.id, nullptr);
    if (!widget) {
        widget = new StickerWidget(config);
        widget->show();
        m_widgets.insert(config.id, widget);
        return widget;
    }

    widget->updateConfig(config);
    return widget;
}

void StickerRuntime::destroy(const QString &stickerId)
{
    StickerWidget *widget = m_widgets.value(stickerId, nullptr);
    if (!widget) {
        return;
    }

    widget->disconnect();
    widget->close();
    widget->deleteLater();
    m_widgets.remove(stickerId);
}

void StickerRuntime::clear()
{
    const QList<QString> keys = m_widgets.keys();
    for (const QString &key : keys) {
        destroy(key);
    }
}

bool StickerRuntime::hasWidget(const QString &stickerId) const
{
    return m_widgets.contains(stickerId);
}

StickerWidget *StickerRuntime::widget(const QString &stickerId) const
{
    return m_widgets.value(stickerId, nullptr);
}

QList<StickerWidget*> StickerRuntime::widgets() const
{
    return m_widgets.values();
}

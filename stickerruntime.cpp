#include "StickerRuntime.h"

StickerRuntime::StickerRuntime(QObject *parent)
    : QObject(parent)
    , m_widget(nullptr)
{
}

StickerRuntime::~StickerRuntime()
{
    destroy();
}

void StickerRuntime::createOrUpdate(const StickerConfig &config)
{
    if (!m_widget) {
        m_widget = new StickerWidget(config);
        m_widget->show();
        return;
    }

    m_widget->updateConfig(config);
}

void StickerRuntime::destroy()
{
    if (!m_widget) {
        return;
    }

    m_widget->disconnect();
    m_widget->close();
    m_widget->deleteLater();
    m_widget = nullptr;
}

bool StickerRuntime::hasWidget() const
{
    return m_widget != nullptr;
}

StickerWidget *StickerRuntime::widget() const
{
    return m_widget;
}

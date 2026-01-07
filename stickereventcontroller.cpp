#include "stickereventcontroller.h"

StickerEventController::StickerEventController(QObject *parent)
    : QObject(parent)
    , m_handler(this)
    , m_events(nullptr)
{
    connect(&m_handler, &EventHandler::eventExecuted,
            this, &StickerEventController::eventExecuted);
    connect(&m_handler, &EventHandler::eventFailed,
            this, &StickerEventController::eventFailed);
}

void StickerEventController::setEvents(const QList<StickerEvent> *events)
{
    m_events = events;
}

void StickerEventController::handleTrigger(MouseTrigger trigger)
{
    if (!m_events) {
        return;
    }

    for (const auto &event : *m_events) {
        if (event.trigger == trigger && event.enabled) {
            m_handler.executeEvent(event);
        }
    }
}

void StickerEventController::setAnchorContext(QWidget *widget, const QRect &rect, int pollIntervalMs)
{
    m_handler.setAnchorContext(widget, rect, pollIntervalMs);
}

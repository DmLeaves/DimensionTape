#ifndef STICKEREVENTCONTROLLER_H
#define STICKEREVENTCONTROLLER_H

#include <QObject>
#include "EventHandler.h"
#include "StickerData.h"

class StickerEventController : public QObject
{
    Q_OBJECT

public:
    explicit StickerEventController(QObject *parent = nullptr);

    void setEvents(const QList<StickerEvent> *events);
    void handleTrigger(MouseTrigger trigger);

signals:
    void eventExecuted(const QString &message);
    void eventFailed(const QString &error);

private:
    EventHandler m_handler;
    const QList<StickerEvent> *m_events;
};

#endif // STICKEREVENTCONTROLLER_H

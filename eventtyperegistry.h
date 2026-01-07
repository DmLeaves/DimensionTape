#ifndef EVENTTYPEREGISTRY_H
#define EVENTTYPEREGISTRY_H

#include <QList>
#include <QString>
#include "StickerData.h"

enum class EventFieldValueType {
    Text = 0,
    Multiline,
    Number,
    Boolean
};

enum class EventFieldBinding {
    Target = 0,
    Parameter
};

struct EventFieldSpec {
    QString key;
    QString label;
    QString placeholder;
    QString hint;
    EventFieldValueType valueType = EventFieldValueType::Text;
    EventFieldBinding binding = EventFieldBinding::Parameter;
    bool optional = true;
};

struct EventTypeSpec {
    StickerEventType type = StickerEventType::None;
    QString displayName;
    QString description;
    QList<EventFieldSpec> fields;
    QString example;
};

class EventTypeRegistry
{
public:
    static const EventTypeRegistry &instance();

    EventTypeSpec specFor(StickerEventType type) const;
    QStringList typeDisplayNames() const;

private:
    EventTypeRegistry();
    QList<EventTypeSpec> m_specs;
};

#endif // EVENTTYPEREGISTRY_H

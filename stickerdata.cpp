#include "StickerData.h"
#include <QJsonArray>

QJsonObject StickerEvent::toJson() const
{
    QJsonObject obj;
    obj["type"] = static_cast<int>(type);
    obj["trigger"] = static_cast<int>(trigger);
    obj["target"] = target;
    obj["parameters"] = parameters;
    obj["enabled"] = enabled;
    return obj;
}

void StickerEvent::fromJson(const QJsonObject &json)
{
    type = static_cast<StickerEventType>(json["type"].toInt());
    trigger = static_cast<MouseTrigger>(json["trigger"].toInt());
    target = json["target"].toString();
    parameters = json["parameters"].toString();
    enabled = json["enabled"].toBool(true);
}

QJsonObject StickerConfig::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name;
    obj["imagePath"] = imagePath;
    obj["position"] = QJsonArray{position.x(), position.y()};
    obj["size"] = QJsonArray{size.width(), size.height()};
    obj["isDesktopMode"] = isDesktopMode;
    obj["visible"] = visible;
    obj["opacity"] = opacity;
    obj["allowDrag"] = allowDrag;       // 新增
    obj["clickThrough"] = clickThrough; // 新增

    QJsonArray eventsArray;
    for (const StickerEvent &event : events) {
        eventsArray.append(event.toJson());
    }
    obj["events"] = eventsArray;

    return obj;
}

void StickerConfig::fromJson(const QJsonObject &json)
{
    id = json["id"].toString();
    name = json["name"].toString();
    imagePath = json["imagePath"].toString();

    QJsonArray posArray = json["position"].toArray();
    if (posArray.size() >= 2) {
        position = QPoint(posArray[0].toInt(), posArray[1].toInt());
    }

    QJsonArray sizeArray = json["size"].toArray();
    if (sizeArray.size() >= 2) {
        size = QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
    }

    isDesktopMode = json["isDesktopMode"].toBool(true);
    visible = json["visible"].toBool(true);
    opacity = json["opacity"].toDouble(1.0);
    allowDrag = json["allowDrag"].toBool(true);         // 新增，默认允许拖动
    clickThrough = json["clickThrough"].toBool(false);  // 新增，默认不穿透

    events.clear();
    QJsonArray eventsArray = json["events"].toArray();
    for (const QJsonValue &value : eventsArray) {
        StickerEvent event;
        event.fromJson(value.toObject());
        events.append(event);
    }
}

QString mouseTriggersToString(MouseTrigger trigger)
{
    switch (trigger) {
    case MouseTrigger::LeftClick: return "左键单击";
    case MouseTrigger::RightClick: return "右键单击";
    case MouseTrigger::DoubleClick: return "双击";
    case MouseTrigger::WheelUp: return "滚轮向上";
    case MouseTrigger::WheelDown: return "滚轮向下";
    case MouseTrigger::MouseEnter: return "鼠标进入";
    case MouseTrigger::MouseLeave: return "鼠标离开";
    default: return "无";
    }
}

MouseTrigger stringToMouseTrigger(const QString &str)
{
    if (str == "左键单击") return MouseTrigger::LeftClick;
    if (str == "右键单击") return MouseTrigger::RightClick;
    if (str == "双击") return MouseTrigger::DoubleClick;
    if (str == "滚轮向上") return MouseTrigger::WheelUp;
    if (str == "滚轮向下") return MouseTrigger::WheelDown;
    if (str == "鼠标进入") return MouseTrigger::MouseEnter;
    if (str == "鼠标离开") return MouseTrigger::MouseLeave;
    return MouseTrigger::None;
}

QString eventTypeToString(StickerEventType type)
{
    switch (type) {
    case StickerEventType::OpenProgram: return "打开程序";
    case StickerEventType::OpenFolder: return "打开文件夹";
    case StickerEventType::OpenFile: return "打开文件";
    case StickerEventType::PlaySound: return "播放声音";
    case StickerEventType::ShowMessage: return "显示消息";
    case StickerEventType::CustomScript: return "自定义脚本";
    default: return "无";
    }
}

StickerEventType stringToEventType(const QString &str)
{
    if (str == "打开程序") return StickerEventType::OpenProgram;
    if (str == "打开文件夹") return StickerEventType::OpenFolder;
    if (str == "打开文件") return StickerEventType::OpenFile;
    if (str == "播放声音") return StickerEventType::PlaySound;
    if (str == "显示消息") return StickerEventType::ShowMessage;
    if (str == "自定义脚本") return StickerEventType::CustomScript;
    return StickerEventType::None;
}

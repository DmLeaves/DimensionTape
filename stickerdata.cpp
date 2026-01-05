#include "StickerData.h"
#include <QJsonArray>
#include <QtMath>

StickerTransform::StickerTransform()
    : scaleX(1.0)
    , scaleY(1.0)
    , rotation(0.0)
    , shearX(0.0)
    , shearY(0.0)
{
}

QTransform StickerTransform::toTransform() const
{
    QTransform transform;
    transform.scale(scaleX, scaleY);
    transform.shear(shearX, shearY);
    transform.rotate(rotation);
    return transform;
}

StickerTransform StickerTransform::fromTransform(const QTransform &transform)
{
    StickerTransform params;

    const double a = transform.m11();
    const double b = transform.m12();
    const double c = transform.m21();
    const double d = transform.m22();

    double rotationRad = qAtan2(b, a);
    const double cosr = qCos(rotationRad);
    const double sinr = qSin(rotationRad);

    const double aPrime = a * cosr + b * sinr;
    const double bPrime = -a * sinr + b * cosr;
    const double cPrime = c * cosr + d * sinr;
    const double dPrime = -c * sinr + d * cosr;

    params.scaleX = qFuzzyIsNull(aPrime) ? 1.0 : aPrime;
    params.scaleY = qFuzzyIsNull(dPrime) ? 1.0 : dPrime;
    params.shearX = qFuzzyIsNull(params.scaleX) ? 0.0 : (bPrime / params.scaleX);
    params.shearY = qFuzzyIsNull(params.scaleY) ? 0.0 : (cPrime / params.scaleY);
    params.rotation = qRadiansToDegrees(rotationRad);

    return params;
}

QJsonObject StickerTransform::toJson() const
{
    QJsonObject obj;
    obj["scaleX"] = scaleX;
    obj["scaleY"] = scaleY;
    obj["rotation"] = rotation;
    obj["shearX"] = shearX;
    obj["shearY"] = shearY;
    return obj;
}

void StickerTransform::fromJson(const QJsonObject &json)
{
    scaleX = json["scaleX"].toDouble(1.0);
    scaleY = json["scaleY"].toDouble(1.0);
    rotation = json["rotation"].toDouble(0.0);
    shearX = json["shearX"].toDouble(0.0);
    shearY = json["shearY"].toDouble(0.0);
}

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

StickerFollowConfig::StickerFollowConfig()
    : enabled(false)
    , batchMode(false)
    , filterType(FollowFilterType::WindowClass)
    , anchor(FollowAnchor::LeftTop)
    , offsetMode(FollowOffsetMode::AbsolutePixels)
    , offset(0.0, 0.0)
    , pollIntervalMs(16)
    , hideWhenMinimized(true)
{
}

QJsonObject StickerFollowConfig::toJson() const
{
    QJsonObject obj;
    obj["enabled"] = enabled;
    obj["batchMode"] = batchMode;
    obj["filterType"] = static_cast<int>(filterType);
    obj["filterValue"] = filterValue;
    obj["anchor"] = static_cast<int>(anchor);
    obj["offsetMode"] = static_cast<int>(offsetMode);
    obj["offset"] = QJsonArray{offset.x(), offset.y()};
    obj["pollIntervalMs"] = pollIntervalMs;
    obj["hideWhenMinimized"] = hideWhenMinimized;
    return obj;
}

void StickerFollowConfig::fromJson(const QJsonObject &json)
{
    enabled = json["enabled"].toBool(false);
    batchMode = json["batchMode"].toBool(false);
    filterType = static_cast<FollowFilterType>(
        json["filterType"].toInt(static_cast<int>(FollowFilterType::WindowClass)));
    filterValue = json["filterValue"].toString();
    anchor = static_cast<FollowAnchor>(
        json["anchor"].toInt(static_cast<int>(FollowAnchor::LeftTop)));
    offsetMode = static_cast<FollowOffsetMode>(
        json["offsetMode"].toInt(static_cast<int>(FollowOffsetMode::AbsolutePixels)));

    QJsonArray offsetArray = json["offset"].toArray();
    if (offsetArray.size() >= 2) {
        offset = QPointF(offsetArray[0].toDouble(), offsetArray[1].toDouble());
    }

    pollIntervalMs = json["pollIntervalMs"].toInt(16);
    hideWhenMinimized = json["hideWhenMinimized"].toBool(true);
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
    obj["transform"] = transform.toJson();
    obj["follow"] = follow.toJson();

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
    if (json["transform"].isObject()) {
        transform.fromJson(json["transform"].toObject());
    } else if (json["transform"].isArray()) {
        QJsonArray transformArray = json["transform"].toArray();
        if (transformArray.size() >= 6) {
            QTransform matrix(
                transformArray[0].toDouble(), transformArray[1].toDouble(),
                transformArray[2].toDouble(), transformArray[3].toDouble(),
                transformArray[4].toDouble(), transformArray[5].toDouble()
            );
            transform = StickerTransform::fromTransform(matrix);
        }
    } else {
        transform = StickerTransform();
    }

    if (json["follow"].isObject()) {
        follow.fromJson(json["follow"].toObject());
    } else {
        follow = StickerFollowConfig();
    }

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

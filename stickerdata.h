#ifndef STICKERDATA_H
#define STICKERDATA_H

#include <QString>
#include <QPoint>
#include <QSize>
#include <QJsonObject>
#include <QList>
#include <QTransform>

// 前向声明
struct StickerEvent;

// 贴纸事件类型
enum class StickerEventType {
    None = 0,
    OpenProgram,     // 打开程序
    OpenFolder,      // 打开文件夹
    OpenFile,        // 打开文件
    PlaySound,       // 播放声音
    ShowMessage,     // 显示消息
    CustomScript     // 自定义脚本
};

// 鼠标触发器类型
enum class MouseTrigger {
    None = 0,
    LeftClick,       // 左键单击
    RightClick,      // 右键单击
    DoubleClick,     // 双击
    WheelUp,         // 滚轮向上
    WheelDown,       // 滚轮向下
    MouseEnter,      // 鼠标进入
    MouseLeave       // 鼠标离开
};

// 贴纸事件数据
struct StickerEvent {
    StickerEventType type;
    MouseTrigger trigger;
    QString target;      // 目标路径/命令
    QString parameters;  // 参数
    bool enabled;

    // 构造函数
    StickerEvent()
        : type(StickerEventType::None)
        , trigger(MouseTrigger::None)
        , enabled(true)
    {
    }

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
};

// 贴纸变换参数
struct StickerTransform {
    double scaleX;
    double scaleY;
    double rotation; // 度
    double shearX;
    double shearY;

    StickerTransform();

    QTransform toTransform() const;
    static StickerTransform fromTransform(const QTransform &transform);
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
};

// 贴纸配置数据
struct StickerConfig {
    QString id;              // 唯一标识
    QString name;            // 贴纸名称
    QString imagePath;       // 图片路径
    QPoint position;         // 位置
    QSize size;              // 大小
    bool isDesktopMode;      // 是否为桌面模式
    bool visible;            // 是否可见
    double opacity;          // 透明度
    bool allowDrag;          // 允许拖动
    bool clickThrough;       // 点击穿透
    StickerTransform transform; // 变换参数
    QList<StickerEvent> events; // 事件列表

    // 构造函数
    StickerConfig()
        : isDesktopMode(true)
        , visible(true)
        , opacity(1.0)
        , allowDrag(true)        // 默认允许拖动
        , clickThrough(false)    // 默认不穿透
    {
    }

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
};

// 鼠标触发器转换函数
QString mouseTriggersToString(MouseTrigger trigger);
MouseTrigger stringToMouseTrigger(const QString &str);

// 事件类型转换函数
QString eventTypeToString(StickerEventType type);
StickerEventType stringToEventType(const QString &str);

#endif // STICKERDATA_H

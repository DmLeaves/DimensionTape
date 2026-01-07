#include "eventtyperegistry.h"

namespace {
EventFieldSpec makeField(const QString &key,
                         const QString &label,
                         const QString &placeholder,
                         const QString &hint,
                         EventFieldBinding binding,
                         bool optional,
                         EventFieldValueType valueType = EventFieldValueType::Text)
{
    EventFieldSpec field;
    field.key = key;
    field.label = label;
    field.placeholder = placeholder;
    field.hint = hint;
    field.binding = binding;
    field.valueType = valueType;
    field.optional = optional;
    return field;
}
}

EventTypeRegistry::EventTypeRegistry()
{
    EventTypeSpec openProgram;
    openProgram.type = StickerEventType::OpenProgram;
    openProgram.displayName = "打开程序";
    openProgram.description = "启动一个程序或可执行文件。";
    openProgram.fields.append(
        makeField("program", "程序路径", "例如 C:\\Program Files\\App\\app.exe",
                  "支持可执行文件路径或系统 PATH 中的命令。", EventFieldBinding::Target, false)
    );
    openProgram.fields.append(
        makeField("args", "启动参数", "-arg1 -arg2",
                  "用空格分隔，可留空。", EventFieldBinding::Parameter, true)
    );
    openProgram.example = "示例: 程序路径 C:\\Windows\\notepad.exe";
    m_specs.append(openProgram);

    EventTypeSpec openFolder;
    openFolder.type = StickerEventType::OpenFolder;
    openFolder.displayName = "打开文件夹";
    openFolder.description = "在资源管理器中打开指定文件夹。";
    openFolder.fields.append(
        makeField("folder", "文件夹路径", "例如 C:\\Users\\Name\\Documents",
                  "需要是存在的目录。", EventFieldBinding::Target, false)
    );
    openFolder.example = "示例: 文件夹路径 D:\\Projects";
    m_specs.append(openFolder);

    EventTypeSpec openFile;
    openFile.type = StickerEventType::OpenFile;
    openFile.displayName = "打开文件";
    openFile.description = "使用系统默认程序打开文件。";
    openFile.fields.append(
        makeField("file", "文件路径", "例如 D:\\file.txt",
                  "支持常见文件类型。", EventFieldBinding::Target, false)
    );
    openFile.example = "示例: 文件路径 D:\\readme.txt";
    m_specs.append(openFile);

    EventTypeSpec playSound;
    playSound.type = StickerEventType::PlaySound;
    playSound.displayName = "播放声音";
    playSound.description = "播放一个音频文件。";
    playSound.fields.append(
        makeField("sound", "音频路径", "例如 D:\\sound.mp3",
                  "支持 mp3/wav 等常见格式。", EventFieldBinding::Target, false)
    );
    playSound.example = "示例: 音频路径 D:\\alert.wav";
    m_specs.append(playSound);

    EventTypeSpec showMessage;
    showMessage.type = StickerEventType::ShowMessage;
    showMessage.displayName = "显示消息";
    showMessage.description = "在贴纸附近显示一条消息提示，全部作为正文显示。";
    showMessage.fields.append(
        makeField("message", "正文内容", "输入主要消息内容",
                  "会作为第一行正文显示。", EventFieldBinding::Target, false,
                  EventFieldValueType::Multiline)
    );
    showMessage.fields.append(
        makeField("append", "追加内容", "例如 详细说明",
                  "可选，作为第二行正文显示。", EventFieldBinding::Parameter, true)
    );
    showMessage.example = "示例: 正文 任务完成 / 追加内容 请查收";
    m_specs.append(showMessage);

    EventTypeSpec customScript;
    customScript.type = StickerEventType::CustomScript;
    customScript.displayName = "自定义脚本";
    customScript.description = "执行脚本或可执行文件。";
    customScript.fields.append(
        makeField("script", "脚本路径", "例如 D:\\scripts\\task.ps1",
                  "支持 .bat/.cmd/.ps1 或可执行文件。", EventFieldBinding::Target, false)
    );
    customScript.fields.append(
        makeField("args", "脚本参数", "-foo bar",
                  "用空格分隔，可留空。", EventFieldBinding::Parameter, true)
    );
    customScript.example = "示例: 脚本路径 D:\\scripts\\run.bat";
    m_specs.append(customScript);
}

const EventTypeRegistry &EventTypeRegistry::instance()
{
    static EventTypeRegistry registry;
    return registry;
}

EventTypeSpec EventTypeRegistry::specFor(StickerEventType type) const
{
    for (const EventTypeSpec &spec : m_specs) {
        if (spec.type == type) {
            return spec;
        }
    }
    EventTypeSpec fallback;
    fallback.type = type;
    fallback.displayName = eventTypeToString(type);
    fallback.description = "未定义的事件类型。";
    fallback.fields.append(
        makeField("target", "目标", "", "", EventFieldBinding::Target, true)
    );
    fallback.fields.append(
        makeField("parameters", "参数", "", "", EventFieldBinding::Parameter, true)
    );
    return fallback;
}

QStringList EventTypeRegistry::typeDisplayNames() const
{
    QStringList names;
    names.reserve(m_specs.size());
    for (const EventTypeSpec &spec : m_specs) {
        names.append(spec.displayName);
    }
    return names;
}

#include "EventHandler.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
#include <QMediaPlayer>  // 替换 QSound

EventHandler::EventHandler(QObject *parent)
    : QObject(parent)
{
}

void EventHandler::executeEvent(const StickerEvent &event)
{
    if (!event.enabled) {
        return;
    }

    qDebug() << "执行事件:" << eventTypeToString(event.type)
             << "目标:" << event.target;

    try {
        switch (event.type) {
        case StickerEventType::OpenProgram:
            openProgram(event.target, event.parameters);
            break;
        case StickerEventType::OpenFolder:
            openFolder(event.target);
            break;
        case StickerEventType::OpenFile:
            openFile(event.target);
            break;
        case StickerEventType::PlaySound:
            playSound(event.target);
            break;
        case StickerEventType::ShowMessage:
            showMessage(event.target, event.parameters);
            break;
        case StickerEventType::CustomScript:
            runCustomScript(event.target, event.parameters);
            break;
        default:
            break;
        }

        emit eventExecuted(QString("已执行: %1").arg(eventTypeToString(event.type)));

    } catch (const std::exception &e) {
        QString error = QString("事件执行失败: %1").arg(e.what());
        qDebug() << error;
        emit eventFailed(error);
    }
}

void EventHandler::openProgram(const QString &program, const QString &parameters)
{
    QProcess *process = new QProcess(this);

    if (parameters.isEmpty()) {
        process->startDetached(program);
    } else {
        QStringList args = parameters.split(' ', QString::SkipEmptyParts);
        process->startDetached(program, args);
    }
}

void EventHandler::openFolder(const QString &folderPath)
{
    QFileInfo info(folderPath);
    if (info.exists() && info.isDir()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
    } else {
        throw std::runtime_error("文件夹不存在");
    }
}

void EventHandler::openFile(const QString &filePath)
{
    QFileInfo info(filePath);
    if (info.exists() && info.isFile()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    } else {
        throw std::runtime_error("文件不存在");
    }
}

void EventHandler::playSound(const QString &soundPath)
{
    QFileInfo info(soundPath);
    if (info.exists()) {
        // 使用 QMediaPlayer 替换 QSound
        QMediaPlayer *player = new QMediaPlayer(this);
        player->setMedia(QUrl::fromLocalFile(soundPath));
        player->play();

        // 播放完成后自动删除
        connect(player, QOverload<QMediaPlayer::State>::of(&QMediaPlayer::stateChanged),
                [player](QMediaPlayer::State state) {
                    if (state == QMediaPlayer::StoppedState) {
                        player->deleteLater();
                    }
                });
    } else {
        throw std::runtime_error("音频文件不存在");
    }
}

void EventHandler::showMessage(const QString &message, const QString &title)
{
    QMessageBox::information(nullptr, title.isEmpty() ? "贴纸消息" : title, message);
}

void EventHandler::runCustomScript(const QString &script, const QString &parameters)
{
    QProcess *process = new QProcess(this);

    // 检测脚本类型
    if (script.endsWith(".bat") || script.endsWith(".cmd")) {
        // Windows 批处理文件
        process->startDetached("cmd", QStringList() << "/c" << script << parameters);
    } else if (script.endsWith(".ps1")) {
        // PowerShell 脚本
        process->startDetached("powershell", QStringList() << "-File" << script << parameters);
    } else {
        // 直接执行
        process->startDetached(script, parameters.split(' ', QString::SkipEmptyParts));
    }
}

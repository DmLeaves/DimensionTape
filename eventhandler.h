#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QObject>
#include <QProcess>
#include <QMediaPlayer>  // 替换 QSound
#include <QMessageBox>
#include "StickerData.h"

class EventHandler : public QObject
{
    Q_OBJECT

public:
    explicit EventHandler(QObject *parent = nullptr);

    // 执行事件
    void executeEvent(const StickerEvent &event);

private:
    void openProgram(const QString &program, const QString &parameters = "");
    void openFolder(const QString &folderPath);
    void openFile(const QString &filePath);
    void playSound(const QString &soundPath);
    void showMessage(const QString &message, const QString &title = "贴纸消息");
    void runCustomScript(const QString &script, const QString &parameters = "");

signals:
    void eventExecuted(const QString &message);
    void eventFailed(const QString &error);
};

#endif // EVENTHANDLER_H

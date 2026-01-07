#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QObject>
#include <QProcess>
#include <QMediaPlayer>  // 替换 QSound
#include <QRect>
#include <QPoint>
#include <QPointer>
#include "StickerData.h"

class QWidget;

class EventHandler : public QObject
{
    Q_OBJECT

public:
    explicit EventHandler(QObject *parent = nullptr);

    // 执行事件
    void executeEvent(const StickerEvent &event);
    void setAnchorContext(QWidget *widget, const QRect &rect, int pollIntervalMs = 33);
    void clearAnchorContext();

private:
    void openProgram(const QString &program, const QString &parameters = "");
    void openFolder(const QString &folderPath);
    void openFile(const QString &filePath);
    void playSound(const QString &soundPath);
    void showMessage(const QString &message, const QString &title = "贴纸消息");
    void runCustomScript(const QString &script, const QString &parameters = "");
    QPoint resolveMessagePosition(const QRect &targetRect, const QSize &bubbleSize) const;

signals:
    void eventExecuted(const QString &message);
    void eventFailed(const QString &error);

private:
    QPointer<QWidget> m_anchorWidget;
    QRect m_anchorRect;
    int m_pollIntervalMs = 33;
    bool m_hasAnchorRect = false;
};

#endif // EVENTHANDLER_H

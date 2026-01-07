#include "EventHandler.h"
#include "messagebubblewidget.h"
#include "messagefollowcontroller.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
#include <QMediaPlayer>  // 替换 QSound
#include <QGuiApplication>
#include <QScreen>
#include <QtGlobal>
#include <QVariantMap>

EventHandler::EventHandler(QObject *parent)
    : QObject(parent)
{
}

void EventHandler::setAnchorContext(QWidget *widget, const QRect &rect, int pollIntervalMs)
{
    m_anchorWidget = widget;
    m_anchorRect = rect;
    m_hasAnchorRect = rect.isValid();
    m_pollIntervalMs = qMax(16, pollIntervalMs);
}

void EventHandler::clearAnchorContext()
{
    m_anchorWidget = nullptr;
    m_anchorRect = QRect();
    m_hasAnchorRect = false;
    m_pollIntervalMs = 33;
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
        {
            QString args = event.parameters.value("args").toString();
            if (args.isEmpty()) {
                args = event.parametersText();
            }
            openProgram(event.target, args);
            break;
        }
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
        {
            QString extra = event.parameters.value("append").toString();
            if (extra.isEmpty()) {
                extra = event.parametersText();
            }
            showMessage(event.target, extra);
            break;
        }
        case StickerEventType::CustomScript:
        {
            QString args = event.parameters.value("args").toString();
            if (args.isEmpty()) {
                args = event.parametersText();
            }
            runCustomScript(event.target, args);
            break;
        }
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
    QString text = message;
    const QString extra = title.trimmed();
    if (!extra.isEmpty()) {
        if (!text.isEmpty()) {
            text = QString("%1\n%2").arg(message, extra);
        } else {
            text = extra;
        }
    }

    MessageBubbleWidget *bubble = new MessageBubbleWidget();
    bubble->setAttribute(Qt::WA_DeleteOnClose, true);

    QVariantMap baseConfig;
    baseConfig["autoPosition"] = false;

    bubble->setKind(MessageKind::Toast);
    bubble->setConfig(baseConfig);
    bubble->setText(text);

    QSize bubbleSize = bubble->size();
    QPoint pos;
    QRect anchorRect = m_anchorRect;
    if (!anchorRect.isValid() && m_anchorWidget) {
        anchorRect = m_anchorWidget->frameGeometry();
    }
    if (anchorRect.isValid()) {
        pos = resolveMessagePosition(anchorRect, bubbleSize);
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect geom = screen ? screen->availableGeometry() : QRect(0, 0, 800, 600);
        const int margin = 12;
        pos = QPoint(geom.right() - margin - bubbleSize.width(), geom.top() + margin);
    }

    QVariantMap finalConfig = baseConfig;
    finalConfig.insert("x", pos.x());
    finalConfig.insert("y", pos.y());
    bubble->showMessage(MessageKind::Toast, text, finalConfig);
    MessageFollowController::instance()->trackMessage(bubble, m_anchorWidget.data(), pos, m_pollIntervalMs);
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

QPoint EventHandler::resolveMessagePosition(const QRect &targetRect, const QSize &bubbleSize) const
{
    if (!targetRect.isValid()) {
        return QPoint();
    }

    const int gap = 12;
    const int screenMargin = 12;

    const QPoint center = targetRect.center();
    QScreen *screen = QGuiApplication::screenAt(center);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect screenRect = screen ? screen->availableGeometry() : QRect(0, 0, 800, 600);

    const bool onLeft = center.x() < screenRect.center().x();
    const bool onTop = center.y() < screenRect.center().y();

    QPoint pos;
    if (onLeft && onTop) {
        pos = QPoint(targetRect.right() + gap, targetRect.bottom() + gap);
    } else if (!onLeft && !onTop) {
        pos = QPoint(targetRect.left() - gap - bubbleSize.width(),
                     targetRect.top() - gap - bubbleSize.height());
    } else if (onLeft && !onTop) {
        pos = QPoint(targetRect.right() + gap,
                     targetRect.top() - gap - bubbleSize.height());
    } else {
        pos = QPoint(targetRect.left() - gap - bubbleSize.width(),
                     targetRect.bottom() + gap);
    }

    int minX = screenRect.left() + screenMargin;
    int minY = screenRect.top() + screenMargin;
    int maxX = screenRect.right() - screenMargin - bubbleSize.width() + 1;
    int maxY = screenRect.bottom() - screenMargin - bubbleSize.height() + 1;
    if (maxX < minX) maxX = minX;
    if (maxY < minY) maxY = minY;
    pos.setX(qBound(minX, pos.x(), maxX));
    pos.setY(qBound(minY, pos.y(), maxY));
    return pos;
}

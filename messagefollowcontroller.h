#ifndef MESSAGEFOLLOWCONTROLLER_H
#define MESSAGEFOLLOWCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QPointer>
#include <QPoint>
#include "StickerData.h"
#include "windowattachmentservice.h"
#include "windowrecognitionservice.h"

class MessageBubbleWidget;
class QWidget;
class QTimer;

class MessageFollowController : public QObject
{
    Q_OBJECT

public:
    static MessageFollowController *instance();
    explicit MessageFollowController(QObject *parent = nullptr);

    void trackMessage(MessageBubbleWidget *bubble,
                      QWidget *anchorWidget,
                      const QPoint &initialTopLeft,
                      int pollIntervalMs = 33);
    void untrackMessage(MessageBubbleWidget *bubble);
    void clear();

private slots:
    void refresh();

private:
    struct MessageState {
        QPointer<MessageBubbleWidget> bubble;
        QPointer<QWidget> anchorWidget;
        FollowAnchor anchor = FollowAnchor::LeftTop;
        FollowOffsetMode offsetMode = FollowOffsetMode::AbsolutePixels;
        QPointF offset;
        WindowHandle targetHandle = 0;
        QPoint initialTopLeft;
        int pollIntervalMs = 33;
    };

    static WindowHandle resolveOwnerHandle(QWidget *widget);
    static FollowAnchor chooseAnchorForPosition(const QRectF &windowRect,
                                                const QPoint &topLeft,
                                                const QSize &size);
    void syncTimer();
    int effectiveIntervalMs() const;

    WindowAttachmentService m_attachmentService;
    WindowRecognitionService m_windowService;
    QTimer *m_timer;
    QHash<MessageBubbleWidget*, MessageState> m_messages;
    bool m_refreshing;
};

#endif // MESSAGEFOLLOWCONTROLLER_H

#include "messagefollowcontroller.h"
#include "followlayouthelper.h"
#include "messagebubblewidget.h"
#include <QCoreApplication>
#include <QTimer>
#include <QWidget>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MessageFollowController *MessageFollowController::instance()
{
    static MessageFollowController *s_instance = new MessageFollowController(QCoreApplication::instance());
    return s_instance;
}

MessageFollowController::MessageFollowController(QObject *parent)
    : QObject(parent)
    , m_windowService(this)
    , m_timer(new QTimer(this))
    , m_refreshing(false)
{
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &MessageFollowController::refresh);
}

void MessageFollowController::trackMessage(MessageBubbleWidget *bubble,
                                           QWidget *anchorWidget,
                                           const QPoint &initialTopLeft,
                                           int pollIntervalMs)
{
    if (!bubble) {
        return;
    }

    MessageState state;
    state.bubble = bubble;
    state.anchorWidget = anchorWidget;
    state.initialTopLeft = initialTopLeft;
    state.pollIntervalMs = qMax(16, pollIntervalMs);
    state.targetHandle = resolveOwnerHandle(anchorWidget);

    QSize bubbleSize = bubble->size();

    if (state.targetHandle != 0) {
        WindowInfo info = m_windowService.queryWindow(state.targetHandle);
        if (info.handle != 0 && !info.rect.isNull()) {
            state.anchor = chooseAnchorForPosition(info.rect, initialTopLeft, bubbleSize);
            state.offsetMode = FollowOffsetMode::AbsolutePixels;
            state.offset = FollowLayoutHelper::computeOffsetForPosition(info.rect, bubbleSize,
                                                                        initialTopLeft,
                                                                        state.anchor,
                                                                        state.offsetMode);

            FollowLayoutSpec spec;
            spec.anchor = state.anchor;
            spec.offsetMode = state.offsetMode;
            spec.offset = state.offset;
            QPoint anchoredPos = FollowLayoutHelper::computeAnchoredPosition(info.rect, bubbleSize, spec);
            bubble->move(anchoredPos);
            m_attachmentService.attach(bubble, state.targetHandle);
            m_attachmentService.ensureZOrder(bubble, state.targetHandle);

            m_messages.insert(bubble, state);
            connect(bubble, &QObject::destroyed, this, [this, bubble]() { untrackMessage(bubble); });
            syncTimer();
            return;
        }
        state.targetHandle = 0;
    }

    bubble->move(initialTopLeft);
}

void MessageFollowController::untrackMessage(MessageBubbleWidget *bubble)
{
    if (!bubble) {
        return;
    }
    m_messages.remove(bubble);
    syncTimer();
}

void MessageFollowController::clear()
{
    m_messages.clear();
    if (m_timer->isActive()) {
        m_timer->stop();
    }
}

void MessageFollowController::refresh()
{
    if (m_refreshing) {
        return;
    }

    m_refreshing = true;
    QList<MessageBubbleWidget*> stale;

    for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
        MessageState &state = it.value();
        if (!state.bubble) {
            stale.append(it.key());
            continue;
        }
        if (state.targetHandle == 0) {
            stale.append(it.key());
            continue;
        }

        WindowInfo info = m_windowService.queryWindow(state.targetHandle);
        if (info.handle == 0) {
            stale.append(it.key());
            continue;
        }
        if (info.rect.isNull()) {
            continue;
        }

        FollowLayoutSpec spec;
        spec.anchor = state.anchor;
        spec.offsetMode = state.offsetMode;
        spec.offset = state.offset;
        QPoint topLeft = FollowLayoutHelper::computeAnchoredPosition(info.rect,
                                                                     state.bubble->size(),
                                                                     spec);
        if (state.bubble->pos() != topLeft) {
            state.bubble->move(topLeft);
        }

        m_attachmentService.attach(state.bubble, state.targetHandle);
        m_attachmentService.ensureZOrder(state.bubble, state.targetHandle);
    }

    for (MessageBubbleWidget *bubble : stale) {
        m_messages.remove(bubble);
    }

    m_refreshing = false;
    syncTimer();
}

WindowHandle MessageFollowController::resolveOwnerHandle(QWidget *widget)
{
#ifdef Q_OS_WIN
    if (!widget) {
        return 0;
    }
    HWND self = reinterpret_cast<HWND>(widget->winId());
    if (!IsWindow(self)) {
        return 0;
    }
    HWND owner = reinterpret_cast<HWND>(GetWindowLongPtrW(self, GWLP_HWNDPARENT));
    if (!owner || !IsWindow(owner)) {
        return 0;
    }
    return reinterpret_cast<WindowHandle>(owner);
#else
    Q_UNUSED(widget)
    return 0;
#endif
}

FollowAnchor MessageFollowController::chooseAnchorForPosition(const QRectF &windowRect,
                                                              const QPoint &topLeft,
                                                              const QSize &size)
{
    QPointF center = QPointF(topLeft.x() + size.width() / 2.0,
                             topLeft.y() + size.height() / 2.0);

    bool onLeft = center.x() < windowRect.center().x();
    bool onTop = center.y() < windowRect.center().y();

    if (onLeft && onTop) {
        return FollowAnchor::LeftTop;
    }
    if (onLeft && !onTop) {
        return FollowAnchor::LeftBottom;
    }
    if (!onLeft && onTop) {
        return FollowAnchor::RightTop;
    }
    return FollowAnchor::RightBottom;
}

void MessageFollowController::syncTimer()
{
    int interval = effectiveIntervalMs();
    if (interval <= 0) {
        if (m_timer->isActive()) {
            m_timer->stop();
        }
        return;
    }

    m_timer->setInterval(interval);
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

int MessageFollowController::effectiveIntervalMs() const
{
    int interval = -1;
    for (auto it = m_messages.constBegin(); it != m_messages.constEnd(); ++it) {
        if (!it.value().bubble) {
            continue;
        }
        int candidate = qMax(16, it.value().pollIntervalMs);
        if (interval < 0 || candidate < interval) {
            interval = candidate;
        }
    }
    return interval;
}

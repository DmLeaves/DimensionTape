#include "stickerinteractioncontroller.h"
#include <QtGlobal>
#include <QtMath>
#include <utility>

StickerInteractionController::StickerInteractionController()
    : m_dragging(false)
    , m_rotating(false)
    , m_rotateStartAngle(0.0)
    , m_rotateStartRotation(0.0)
{
}

StickerInteractionController::StickerInteractionController(Callbacks callbacks)
    : m_callbacks(std::move(callbacks))
    , m_dragging(false)
    , m_rotating(false)
    , m_rotateStartAngle(0.0)
    , m_rotateStartRotation(0.0)
{
}

void StickerInteractionController::setCallbacks(Callbacks callbacks)
{
    m_callbacks = std::move(callbacks);
}

bool StickerInteractionController::handleMousePress(QMouseEvent *event, StickerConfig &config, bool allowDrag, bool editMode)
{
    if (event->button() != Qt::LeftButton) {
        return false;
    }

    if (editMode && (event->modifiers() & Qt::ShiftModifier)) {
        m_rotating = true;
        m_dragging = false;
        QRect rect = m_callbacks.widgetRect ? m_callbacks.widgetRect() : QRect();
        m_rotateStartAngle = angleFromCenter(event->pos(), rect);
        m_rotateStartRotation = config.transform.rotation;
        return true;
    }

    if (allowDrag || editMode) {
        m_dragging = true;
        QRect frame = m_callbacks.frameGeometry ? m_callbacks.frameGeometry() : QRect();
        m_dragPosition = event->globalPos() - frame.topLeft();
        return true;
    }

    return false;
}

bool StickerInteractionController::handleMouseMove(QMouseEvent *event, StickerConfig &config, bool allowDrag, bool editMode)
{
    if (m_rotating) {
        QRect rect = m_callbacks.widgetRect ? m_callbacks.widgetRect() : QRect();
        double angle = angleFromCenter(event->pos(), rect);
        double delta = qRadiansToDegrees(angle - m_rotateStartAngle);
        config.transform.rotation = m_rotateStartRotation + delta;
        if (m_callbacks.updateTransformLayout) {
            m_callbacks.updateTransformLayout();
        }
        if (m_callbacks.applyMask) {
            m_callbacks.applyMask();
        }
        if (m_callbacks.requestUpdate) {
            m_callbacks.requestUpdate();
        }
        return true;
    }

    if (m_dragging && (allowDrag || editMode)) {
        QPoint newPos = event->globalPos() - m_dragPosition;
        if (newPos != config.position) {
            if (m_callbacks.moveWindow) {
                m_callbacks.moveWindow(newPos);
            }
            config.position = newPos;
            notifyChanged();
        }
        return true;
    }

    return false;
}

bool StickerInteractionController::handleMouseRelease(QMouseEvent *event, StickerConfig &config)
{
    Q_UNUSED(config)

    if (event->button() != Qt::LeftButton) {
        return false;
    }

    if (m_rotating) {
        m_rotating = false;
        notifyChanged();
        return true;
    }

    if (m_dragging) {
        m_dragging = false;
        if (m_callbacks.requestUpdate) {
            m_callbacks.requestUpdate();
        }
        notifyChanged();
        return true;
    }

    return false;
}

bool StickerInteractionController::handleWheel(QWheelEvent *event, StickerConfig &config, bool editMode)
{
    if (!editMode) {
        return false;
    }

    double delta = event->angleDelta().y() / 120.0;
    if (event->modifiers() & Qt::ControlModifier) {
        config.transform.rotation += delta * 5.0;
    } else {
        double factor = 1.0 + delta * 0.05;
        config.transform.scaleX = qBound(0.1, config.transform.scaleX * factor, 5.0);
        config.transform.scaleY = qBound(0.1, config.transform.scaleY * factor, 5.0);
    }

    if (m_callbacks.updateTransformLayout) {
        m_callbacks.updateTransformLayout();
    }
    if (m_callbacks.applyMask) {
        m_callbacks.applyMask();
    }
    if (m_callbacks.requestUpdate) {
        m_callbacks.requestUpdate();
    }
    notifyChanged();
    return true;
}

void StickerInteractionController::reset()
{
    m_dragging = false;
    m_rotating = false;
}

double StickerInteractionController::angleFromCenter(const QPoint &point, const QRect &rect) const
{
    QPointF center = rect.center();
    QPointF delta = point - center;
    return qAtan2(delta.y(), delta.x());
}

void StickerInteractionController::notifyChanged()
{
    if (m_callbacks.notifyConfigChanged) {
        m_callbacks.notifyConfigChanged();
    }
}

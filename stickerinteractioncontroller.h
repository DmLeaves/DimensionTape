#ifndef STICKERINTERACTIONCONTROLLER_H
#define STICKERINTERACTIONCONTROLLER_H

#include <QPoint>
#include <QRect>
#include <QWheelEvent>
#include <QMouseEvent>
#include <functional>
#include "StickerData.h"

class StickerInteractionController
{
public:
    struct Callbacks {
        std::function<QRect()> widgetRect;
        std::function<QRect()> frameGeometry;
        std::function<void(const QPoint &pos)> moveWindow;
        std::function<void()> updateTransformLayout;
        std::function<void()> applyMask;
        std::function<void()> requestUpdate;
        std::function<void()> notifyConfigChanged;
    };

    StickerInteractionController();
    explicit StickerInteractionController(Callbacks callbacks);

    void setCallbacks(Callbacks callbacks);

    bool handleMousePress(QMouseEvent *event, StickerConfig &config, bool allowDrag, bool editMode);
    bool handleMouseMove(QMouseEvent *event, StickerConfig &config, bool allowDrag, bool editMode);
    bool handleMouseRelease(QMouseEvent *event, StickerConfig &config);
    bool handleWheel(QWheelEvent *event, StickerConfig &config, bool editMode);

    void reset();

private:
    double angleFromCenter(const QPoint &point, const QRect &rect) const;
    void notifyChanged();

    Callbacks m_callbacks;
    bool m_dragging;
    bool m_rotating;
    QPoint m_dragPosition;
    double m_rotateStartAngle;
    double m_rotateStartRotation;
};

#endif // STICKERINTERACTIONCONTROLLER_H

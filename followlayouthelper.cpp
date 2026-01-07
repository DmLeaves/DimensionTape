#include "followlayouthelper.h"
#include <QtMath>

QPointF FollowLayoutHelper::computeOffset(const QRectF &windowRect, const FollowLayoutSpec &spec)
{
    if (spec.offsetMode == FollowOffsetMode::RelativeRatio) {
        return QPointF(windowRect.width() * spec.offset.x(),
                       windowRect.height() * spec.offset.y());
    }
    return spec.offset;
}

QPoint FollowLayoutHelper::computeAnchoredPosition(const QRectF &windowRect,
                                                   const QSize &itemSize,
                                                   const FollowLayoutSpec &spec)
{
    QPointF offset = computeOffset(windowRect, spec);
    QPointF anchorPoint;

    switch (spec.anchor) {
    case FollowAnchor::LeftTop:
        anchorPoint = windowRect.topLeft();
        break;
    case FollowAnchor::LeftBottom:
        anchorPoint = windowRect.bottomLeft();
        break;
    case FollowAnchor::RightTop:
        anchorPoint = windowRect.topRight();
        break;
    case FollowAnchor::RightBottom:
        anchorPoint = windowRect.bottomRight();
        break;
    default:
        anchorPoint = windowRect.topLeft();
        break;
    }

    QPointF target = anchorPoint + offset;
    QPointF topLeft = target;

    if (spec.anchor == FollowAnchor::RightTop || spec.anchor == FollowAnchor::RightBottom) {
        topLeft.setX(target.x() - itemSize.width());
    }
    if (spec.anchor == FollowAnchor::LeftBottom || spec.anchor == FollowAnchor::RightBottom) {
        topLeft.setY(target.y() - itemSize.height());
    }

    return QPoint(qRound(topLeft.x()), qRound(topLeft.y()));
}

QPointF FollowLayoutHelper::computeOffsetForPosition(const QRectF &windowRect,
                                                     const QSize &itemSize,
                                                     const QPoint &itemTopLeft,
                                                     FollowAnchor anchor,
                                                     FollowOffsetMode offsetMode)
{
    QPointF anchorPoint;
    switch (anchor) {
    case FollowAnchor::LeftTop:
        anchorPoint = windowRect.topLeft();
        break;
    case FollowAnchor::LeftBottom:
        anchorPoint = windowRect.bottomLeft();
        break;
    case FollowAnchor::RightTop:
        anchorPoint = windowRect.topRight();
        break;
    case FollowAnchor::RightBottom:
        anchorPoint = windowRect.bottomRight();
        break;
    default:
        anchorPoint = windowRect.topLeft();
        break;
    }

    QPointF offsetPx = QPointF(itemTopLeft) - anchorPoint;
    if (anchor == FollowAnchor::RightTop || anchor == FollowAnchor::RightBottom) {
        offsetPx.setX(offsetPx.x() + itemSize.width());
    }
    if (anchor == FollowAnchor::LeftBottom || anchor == FollowAnchor::RightBottom) {
        offsetPx.setY(offsetPx.y() + itemSize.height());
    }

    if (offsetMode == FollowOffsetMode::RelativeRatio) {
        const double width = windowRect.width();
        const double height = windowRect.height();
        return QPointF(width > 0.0 ? offsetPx.x() / width : 0.0,
                       height > 0.0 ? offsetPx.y() / height : 0.0);
    }

    return offsetPx;
}

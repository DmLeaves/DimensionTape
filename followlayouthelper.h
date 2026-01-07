#ifndef FOLLOWLAYOUTHELPER_H
#define FOLLOWLAYOUTHELPER_H

#include <QPointF>
#include <QRectF>
#include <QSize>
#include "StickerData.h"

struct FollowLayoutSpec {
    FollowAnchor anchor = FollowAnchor::LeftTop;
    FollowOffsetMode offsetMode = FollowOffsetMode::AbsolutePixels;
    QPointF offset;
};

class FollowLayoutHelper
{
public:
    static QPointF computeOffset(const QRectF &windowRect, const FollowLayoutSpec &spec);
    static QPoint computeAnchoredPosition(const QRectF &windowRect,
                                          const QSize &itemSize,
                                          const FollowLayoutSpec &spec);
    static QPointF computeOffsetForPosition(const QRectF &windowRect,
                                            const QSize &itemSize,
                                            const QPoint &itemTopLeft,
                                            FollowAnchor anchor,
                                            FollowOffsetMode offsetMode);
};

#endif // FOLLOWLAYOUTHELPER_H

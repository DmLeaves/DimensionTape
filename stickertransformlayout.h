#ifndef STICKERTRANSFORMLAYOUT_H
#define STICKERTRANSFORMLAYOUT_H

#include <QRectF>
#include <QSize>
#include <QTransform>
#include <QPixmap>
#include "StickerData.h"

struct StickerTransformLayoutResult {
    QRectF baseRect;
    QRectF bounds;
    QSize windowSize;
    QTransform localTransform;
};

class StickerTransformLayout
{
public:
    static bool calculate(const StickerConfig &config,
                          const QSize &baseSize,
                          StickerTransformLayoutResult &out);
    static QTransform buildRenderTransform(const StickerTransformLayoutResult &layout,
                                           const QSize &targetSize);
};

#endif // STICKERTRANSFORMLAYOUT_H

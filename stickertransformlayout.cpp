#include "stickertransformlayout.h"
#include <QtMath>

bool StickerTransformLayout::calculate(const StickerConfig &config,
                                       const QSize &baseSize,
                                       StickerTransformLayoutResult &out)
{
    if (baseSize.isEmpty()) {
        return false;
    }

    out.baseRect = QRectF(-baseSize.width() / 2.0, -baseSize.height() / 2.0,
                          baseSize.width(), baseSize.height());

    out.localTransform = config.transform.toTransform();

    out.bounds = out.localTransform.mapRect(out.baseRect);
    out.windowSize = QSize(
        qMax(1, int(qCeil(out.bounds.width()))),
        qMax(1, int(qCeil(out.bounds.height())))
    );

    return true;
}

QTransform StickerTransformLayout::buildRenderTransform(const StickerTransformLayoutResult &layout,
                                                        const QSize &targetSize)
{
    QPointF targetCenter(targetSize.width() / 2.0, targetSize.height() / 2.0);
    QPointF boundsCenter = layout.bounds.center();
    QPointF delta = targetCenter - boundsCenter;

    QTransform render = layout.localTransform;
    render.setMatrix(render.m11(), render.m12(), render.m13(),
                     render.m21(), render.m22(), render.m23(),
                     render.m31() + delta.x(), render.m32() + delta.y(),
                     render.m33());
    return render;
}

#include "stickerrenderer.h"
#include "stickerimage.h"
#include <QImage>
#include <QPainter>
#include <QtGlobal>

StickerRenderer::StickerRenderer(const StickerImage *image)
    : m_image(image)
{
}

void StickerRenderer::setImage(const StickerImage *image)
{
    m_image = image;
}

bool StickerRenderer::isReady() const
{
    return m_image && !m_image->isNull();
}

bool StickerRenderer::calculateLayout(const StickerConfig &config, StickerTransformLayoutResult &out) const
{
    if (!isReady()) {
        return false;
    }
    return StickerTransformLayout::calculate(config, m_image->baseSize(), out);
}

bool StickerRenderer::paint(QPainter &painter, const StickerConfig &config, const QSize &targetSize) const
{
    if (!isReady()) {
        return false;
    }

    StickerTransformLayoutResult layout;
    if (!calculateLayout(config, layout)) {
        return false;
    }

    QTransform renderTransform = StickerTransformLayout::buildRenderTransform(layout, targetSize);
    painter.save();
    painter.setTransform(renderTransform, true);
    painter.drawPixmap(layout.baseRect, m_image->pixmap(), m_image->sourceRect());
    painter.restore();
    return true;
}

QBitmap StickerRenderer::buildMask(const StickerConfig &config, const QSize &targetSize) const
{
    if (!isReady()) {
        return QBitmap();
    }

    StickerTransformLayoutResult layout;
    if (!calculateLayout(config, layout)) {
        return QBitmap();
    }

    QTransform renderTransform = StickerTransformLayout::buildRenderTransform(layout, targetSize);
    QPixmap maskSource(targetSize);
    maskSource.fill(Qt::transparent);

    QPainter maskPainter(&maskSource);
    maskPainter.setRenderHint(QPainter::Antialiasing);
    maskPainter.setTransform(renderTransform, true);
    maskPainter.drawPixmap(layout.baseRect, m_image->pixmap(), m_image->sourceRect());
    maskPainter.end();

    return createMaskFromPixmap(maskSource);
}

QBitmap StickerRenderer::createMaskFromPixmap(const QPixmap &pixmap) const
{
    if (pixmap.isNull()) {
        return QBitmap();
    }

    QBitmap mask(pixmap.size());
    mask.fill(Qt::color0);

    QPainter maskPainter(&mask);
    maskPainter.setBrush(Qt::color1);
    maskPainter.setPen(Qt::NoPen);

    QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    const int width = image.width();
    const int height = image.height();

    for (int y = 0; y < height; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < width; ++x) {
            if (qAlpha(line[x]) > 50) {
                maskPainter.drawPoint(x, y);
            }
        }
    }

    return mask;
}

#ifndef STICKERRENDERER_H
#define STICKERRENDERER_H

#include <QBitmap>
#include <QSize>
#include "stickertransformlayout.h"

class QPainter;
class StickerImage;

class StickerRenderer
{
public:
    explicit StickerRenderer(const StickerImage *image = nullptr);

    void setImage(const StickerImage *image);
    bool isReady() const;

    bool calculateLayout(const StickerConfig &config, StickerTransformLayoutResult &out) const;
    bool paint(QPainter &painter, const StickerConfig &config, const QSize &targetSize) const;
    QBitmap buildMask(const StickerConfig &config, const QSize &targetSize) const;

private:
    QBitmap createMaskFromPixmap(const QPixmap &pixmap) const;

    const StickerImage *m_image;
};

#endif // STICKERRENDERER_H

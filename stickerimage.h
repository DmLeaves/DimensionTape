#ifndef STICKERIMAGE_H
#define STICKERIMAGE_H

#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QString>

class StickerImage
{
public:
    explicit StickerImage(int maxWindowSize = 600);

    bool loadFromPath(const QString &imagePath);
    void createDefault(int size = 200);

    const QPixmap &pixmap() const;
    bool isNull() const;
    QRect contentRect() const;
    QSize baseSize() const;
    QRectF sourceRect() const;

private:
    QPixmap scalePixmapKeepRatio(const QPixmap &pixmap, int maxSize) const;
    QRect computeContentRect(const QPixmap &pixmap) const;
    QPixmap createDefaultPixmap(int size) const;

    QPixmap m_pixmap;
    QRect m_contentRect;
    int m_maxWindowSize;
};

#endif // STICKERIMAGE_H

#include "stickerimage.h"
#include <QImage>
#include <QPainter>
#include <QRadialGradient>
#include <QtGlobal>

StickerImage::StickerImage(int maxWindowSize)
    : m_maxWindowSize(maxWindowSize)
{
}

bool StickerImage::loadFromPath(const QString &imagePath)
{
    QPixmap originalPixmap(imagePath);
    if (originalPixmap.isNull()) {
        return false;
    }

    m_pixmap = scalePixmapKeepRatio(originalPixmap, m_maxWindowSize);
    m_contentRect = computeContentRect(m_pixmap);
    return true;
}

void StickerImage::createDefault(int size)
{
    m_pixmap = createDefaultPixmap(size);
    m_contentRect = computeContentRect(m_pixmap);
}

const QPixmap &StickerImage::pixmap() const
{
    return m_pixmap;
}

bool StickerImage::isNull() const
{
    return m_pixmap.isNull();
}

QRect StickerImage::contentRect() const
{
    return m_contentRect;
}

QSize StickerImage::baseSize() const
{
    if (m_contentRect.isValid()) {
        return m_contentRect.size();
    }
    return m_pixmap.size();
}

QRectF StickerImage::sourceRect() const
{
    if (m_pixmap.isNull()) {
        return QRectF();
    }
    return m_contentRect.isValid() ? QRectF(m_contentRect) : QRectF(m_pixmap.rect());
}

QPixmap StickerImage::scalePixmapKeepRatio(const QPixmap &pixmap, int maxSize) const
{
    if (pixmap.isNull()) {
        return QPixmap();
    }

    int originalWidth = pixmap.width();
    int originalHeight = pixmap.height();

    if (originalWidth <= maxSize && originalHeight <= maxSize) {
        return pixmap;
    }

    double scale = qMin(double(maxSize) / originalWidth, double(maxSize) / originalHeight);
    int newWidth = int(originalWidth * scale);
    int newHeight = int(originalHeight * scale);

    return pixmap.scaled(newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QRect StickerImage::computeContentRect(const QPixmap &pixmap) const
{
    if (pixmap.isNull()) {
        return QRect();
    }

    QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    int minX = image.width();
    int minY = image.height();
    int maxX = -1;
    int maxY = -1;

    for (int y = 0; y < image.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) > 50) {
                if (x < minX) minX = x;
                if (y < minY) minY = y;
                if (x > maxX) maxX = x;
                if (y > maxY) maxY = y;
            }
        }
    }

    if (maxX < minX || maxY < minY) {
        return QRect(0, 0, image.width(), image.height());
    }

    return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

QPixmap StickerImage::createDefaultPixmap(int size) const
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient(size * 0.4, size * 0.4, size * 0.4);
    gradient.setColorAt(0.0, QColor(100, 150, 255, 255));
    gradient.setColorAt(0.8, QColor(50, 100, 200, 255));
    gradient.setColorAt(1.0, QColor(30, 70, 150, 255));

    painter.setBrush(gradient);
    painter.setPen(QPen(QColor(20, 50, 120), 3));

    const int margin = 10;
    painter.drawEllipse(margin, margin, size - margin * 2, size - margin * 2);

    painter.setBrush(QColor(255, 255, 255, 150));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(size * 0.25, size * 0.25, size * 0.3, size * 0.3);

    painter.setBrush(QColor(150, 200, 255, 200));
    painter.drawEllipse(size * 0.7, size * 0.2, size * 0.1, size * 0.1);
    painter.drawEllipse(size * 0.2, size * 0.7, size * 0.08, size * 0.08);

    painter.end();

    return pixmap;
}

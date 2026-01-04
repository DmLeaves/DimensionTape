#ifndef STICKERRUNTIME_H
#define STICKERRUNTIME_H

#include <QObject>
#include "StickerWidget.h"

class StickerRuntime : public QObject
{
    Q_OBJECT

public:
    explicit StickerRuntime(QObject *parent = nullptr);
    ~StickerRuntime();

    void createOrUpdate(const StickerConfig &config);
    void destroy();
    bool hasWidget() const;
    StickerWidget *widget() const;

private:
    StickerWidget *m_widget;
};

#endif // STICKERRUNTIME_H

#ifndef STICKERRUNTIME_H
#define STICKERRUNTIME_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QString>
#include "StickerWidget.h"

class StickerRuntime : public QObject
{
    Q_OBJECT

public:
    explicit StickerRuntime(QObject *parent = nullptr);
    ~StickerRuntime();

    StickerWidget *createOrUpdate(const StickerConfig &config);
    void destroy(const QString &stickerId);
    void clear();
    bool hasWidget(const QString &stickerId) const;
    StickerWidget *widget(const QString &stickerId) const;
    QList<StickerWidget*> widgets() const;

private:
    QHash<QString, StickerWidget*> m_widgets;
};

#endif // STICKERRUNTIME_H

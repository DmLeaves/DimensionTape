#ifndef STICKERRUNTIME_H
#define STICKERRUNTIME_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QString>
#include "stickerinstance.h"
#include "StickerWidget.h"

class StickerRuntime : public QObject
{
    Q_OBJECT

public:
    explicit StickerRuntime(QObject *parent = nullptr);
    ~StickerRuntime();

    StickerInstance *createOrUpdatePrimary(const StickerConfig &config);
    StickerInstance *createOrUpdateInstance(const StickerConfig &config,
                                            const QString &instanceId,
                                            const QString &templateId,
                                            bool syncToTemplate = true);
    void destroyInstance(const QString &instanceId);
    void destroyInstancesForTemplate(const QString &templateId);
    void clear();
    StickerInstance *instance(const QString &instanceId) const;
    StickerWidget *widget(const QString &instanceId) const;
    QList<StickerInstance*> instances() const;
    QList<StickerInstance*> instancesForTemplate(const QString &templateId) const;

signals:
    void instanceConfigChanged(const QString &instanceId,
                               const StickerConfig &config,
                               bool syncToTemplate);
    void instanceDeleteRequested(const QString &instanceId, const QString &templateId);
    void instanceEditRequested(const QString &instanceId, const QString &templateId);

private:
    StickerInstance *ensureInstance(const StickerConfig &config,
                                    const QString &instanceId,
                                    const QString &templateId,
                                    bool syncToTemplate);
    void connectInstanceSignals(StickerInstance *instance);

    QHash<QString, StickerInstance*> m_instances;
};

#endif // STICKERRUNTIME_H

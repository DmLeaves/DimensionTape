#ifndef STICKERMANAGER_H
#define STICKERMANAGER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QList>
#include "StickerData.h"
#include "stickerrepository.h"
#include "stickerruntime.h"

class StickerManager : public QObject
{
    Q_OBJECT

public:
    static StickerManager* instance();
    ~StickerManager();

    StickerManager(const StickerManager&) = delete;
    StickerManager& operator=(const StickerManager&) = delete;

    // 贴纸管理
    void createStickerInternal(const StickerConfig &config);
    void deleteSticker(const QString &stickerId);
    void editSticker(const QString &stickerId, const StickerConfig &config);
    void editStickerFromMainWindow(const QString &stickerId);
    void cleanup();

    // 配置管理
    bool loadConfig();
    bool saveConfig();
    void loadConfig(const QString &filePath);
    void saveConfig(const QString &filePath);

    // 获取信息
    QList<StickerConfig> getAllConfigs() const;
    StickerWidget* getStickerWidget(const QString &stickerId) const;

public slots:
    void createSticker();
    void onStickerConfigChanged(const StickerConfig &config);
    void onStickerDeleteRequested(const QString &stickerId);
    void onStickerEditRequested(const QString &stickerId);
    void onConfigsRequested();

signals:
    void stickerCreated(const StickerConfig &config);
    void stickerDeleted(const QString &stickerId);
    void stickerConfigChanged(const StickerConfig &config);
    void configLoaded(const QList<StickerConfig> &configs);
    void configSaved();
    void stickerConfigsUpdated(const QList<StickerConfig> &configs);

private slots:
    void onAutoSaveTimer();

private:
    explicit StickerManager(QObject *parent = nullptr);

    bool loadConfigInternal();
    bool saveConfigInternal();
    void destroyStickerInternal();
    void createDefaultSticker();
    void connectWidgetSignals(StickerWidget *widget);
    int findConfigIndex(const QString &stickerId) const;

    StickerRepository m_repository;
    StickerRuntime m_runtime;
    QList<StickerConfig> m_configs;
    mutable QMutex m_mutex;

    QTimer *m_autoSaveTimer;
    bool m_isCleanedUp;
};

#endif // STICKERMANAGER_H

#ifndef STICKERMANAGER_H
#define STICKERMANAGER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include "StickerData.h"
#include "StickerWidget.h"

class StickerManager : public QObject
{
    Q_OBJECT

public:
    explicit StickerManager(QObject *parent = nullptr);
    ~StickerManager();

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
    void initializeDataDirectory();
    QString getConfigFilePath() const;
    void removeStickerWidget(const QString &stickerId);
    void updateStickerWidget(const QString &stickerId, const StickerConfig &config);

    QMap<QString, StickerWidget*> m_stickerWidgets;
    void createDefaultSticker();
    QList<StickerConfig> m_stickerConfigs;
    QTimer *m_autoSaveTimer;
    QString m_configDirectory;
    QString m_defaultConfigFile;
    bool m_isCleanedUp;
};

#endif // STICKERMANAGER_H

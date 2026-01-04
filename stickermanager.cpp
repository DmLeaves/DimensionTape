#include "StickerManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QUuid>

StickerManager::StickerManager(QObject *parent)
    : QObject(parent)
{
    initializeDataDirectory();

    // 设置自动保存定时器
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setSingleShot(false);
    m_autoSaveTimer->setInterval(30000); // 30秒自动保存
    connect(m_autoSaveTimer, &QTimer::timeout, this, &StickerManager::onAutoSaveTimer);
    m_autoSaveTimer->start();
    m_isCleanedUp = false;

    // 启动时加载配置
    loadConfig();

    qDebug() << "贴纸管理器初始化完成";
}

StickerManager::~StickerManager()
{
    cleanup();
    qDebug() << "贴纸管理器销毁完成";
}

void StickerManager::cleanup()
{
    if (m_isCleanedUp) {
        qDebug() << "贴纸管理器已经清理过，跳过重复清理";
        return;
    }

    qDebug() << "开始清理贴纸管理器...";
    m_isCleanedUp = true;

    // 1. 停止自动保存定时器
    if (m_autoSaveTimer) {
        m_autoSaveTimer->stop();
    }

    // 2. 保存当前配置
    saveConfig();

    // 3. 关闭并删除所有贴纸窗口
    qDebug() << "关闭" << m_stickerWidgets.size() << "个贴纸窗口";

    for (auto it = m_stickerWidgets.begin(); it != m_stickerWidgets.end(); ++it) {
        StickerWidget *widget = it.value();
        if (widget) {
            widget->disconnect();
            widget->close();
            widget->deleteLater();
        }
    }
    m_stickerWidgets.clear();
    m_stickerConfigs.clear();

    qDebug() << "贴纸管理器清理完成";
}

void StickerManager::initializeDataDirectory()
{
    m_configDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(m_configDirectory);

    m_defaultConfigFile = QDir(m_configDirectory).filePath("stickers.json");

    qDebug() << "数据目录:" << m_configDirectory;
    qDebug() << "默认配置文件:" << m_defaultConfigFile;
}

QString StickerManager::getConfigFilePath() const
{
    return m_defaultConfigFile;
}

void StickerManager::createSticker()
{
    StickerConfig config;
    config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    config.name = QString("贴纸 %1").arg(m_stickerConfigs.size() + 1);
    config.position = QPoint(100, 100);
    config.size = QSize(200, 200);
    config.isDesktopMode = true;
    config.visible = true;
    config.opacity = 1.0;

    createStickerInternal(config);
}

void StickerManager::deleteSticker(const QString &stickerId)
{
    qDebug() << "删除贴纸:" << stickerId;

    // 移除贴纸窗口
    removeStickerWidget(stickerId);

    // 从配置列表中移除
    for (int i = m_stickerConfigs.size() - 1; i >= 0; --i) {
        if (m_stickerConfigs[i].id == stickerId) {
            m_stickerConfigs.removeAt(i);
            break;
        }
    }

    emit stickerDeleted(stickerId);
    qDebug() << "贴纸删除完成:" << stickerId;
}

void StickerManager::editSticker(const QString &stickerId, const StickerConfig &config)
{
    qDebug() << "编辑贴纸:" << stickerId;

    // 更新配置列表
    for (auto &storedConfig : m_stickerConfigs) {
        if (storedConfig.id == stickerId) {
            storedConfig = config;
            break;
        }
    }

    // 更新贴纸窗口
    updateStickerWidget(stickerId, config);

    emit stickerConfigChanged(config);
    qDebug() << "贴纸编辑完成:" << stickerId;
}


void StickerManager::editStickerFromMainWindow(const QString &stickerId)
{
    qDebug() << "从主窗口编辑贴纸:" << stickerId;

    // 触发信号，让主窗口显示编辑界面
    for (const StickerConfig &config : m_stickerConfigs) {
        if (config.id == stickerId) {
            emit stickerConfigChanged(config);
            break;
        }
    }
}

void StickerManager::removeStickerWidget(const QString &stickerId)
{
    auto it = m_stickerWidgets.find(stickerId);
    if (it != m_stickerWidgets.end()) {
        StickerWidget *widget = it.value();
        widget->close();
        widget->deleteLater();
        m_stickerWidgets.erase(it);
    }
}

void StickerManager::updateStickerWidget(const QString &stickerId, const StickerConfig &config)
{
    auto it = m_stickerWidgets.find(stickerId);
    if (it != m_stickerWidgets.end()) {
        it.value()->updateConfig(config);
    }
}

bool StickerManager::loadConfig()
{
    QString filePath = getConfigFilePath();
    qDebug() << "加载配置:" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "配置文件不存在，使用空配置";
        emit configLoaded(m_stickerConfigs);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "配置文件解析错误:" << error.errorString();
        emit configLoaded(m_stickerConfigs);
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray stickersArray = root["stickers"].toArray();

    if (stickersArray.isEmpty()) {
        qDebug() << "配置文件为空";
        emit configLoaded(m_stickerConfigs);
        return false;
    }

    // 清理现有贴纸
    for (auto it = m_stickerWidgets.begin(); it != m_stickerWidgets.end(); ++it) {
        it.value()->close();
        it.value()->deleteLater();
    }
    m_stickerWidgets.clear();
    m_stickerConfigs.clear();

    // 加载贴纸配置
    for (const QJsonValue &value : stickersArray) {
        StickerConfig config;
        config.fromJson(value.toObject());
        createStickerInternal(config);
    }

    emit configLoaded(m_stickerConfigs);
    qDebug() << "配置加载完成，共" << m_stickerConfigs.size() << "个贴纸";
    return true;
}

void StickerManager::createDefaultSticker()
{
    StickerConfig config;
    config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    config.name = "默认贴纸";
    config.imagePath = ""; // 空路径表示使用默认图像
    config.position = QPoint(200, 200);
    config.size = QSize(200, 200);
    config.isDesktopMode = true;
    config.visible = true;
    config.opacity = 1.0;

    createStickerInternal(config);
    qDebug() << "默认贴纸已创建:" << config.id;
}

void StickerManager::createStickerInternal(const StickerConfig &config)
{
    qDebug() << "创建贴纸:" << config.id;

    // 添加到配置列表
    m_stickerConfigs.append(config);

    // 创建贴纸窗口
    StickerWidget *widget = new StickerWidget(config);
    m_stickerWidgets[config.id] = widget;

    // 连接信号
    connect(widget, &StickerWidget::configChanged, this, &StickerManager::onStickerConfigChanged);
    connect(widget, &StickerWidget::deleteRequested, this, &StickerManager::onStickerDeleteRequested);
    connect(widget, &StickerWidget::editRequested, this, &StickerManager::onStickerEditRequested);

    // 显示窗口
    widget->show();

    // 发送创建信号
    emit stickerCreated(config);
    qDebug() << "贴纸创建完成:" << config.id;
}

void StickerManager::loadConfig(const QString &filePath)
{
    loadConfig(); // 简化版本，使用默认路径
}

bool StickerManager::saveConfig()
{
    QString filePath = getConfigFilePath();
    qDebug() << "保存配置:" << filePath;

    // 更新配置（从窗口获取最新状态）
    for (auto &config : m_stickerConfigs) {
        auto it = m_stickerWidgets.find(config.id);
        if (it != m_stickerWidgets.end()) {
            config = it.value()->getConfig();
        }
    }

    QJsonObject root;
    QJsonArray stickersArray;

    for (const StickerConfig &config : m_stickerConfigs) {
        stickersArray.append(config.toJson());
    }

    root["version"] = "1.0";
    root["stickers"] = stickersArray;

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法写入配置文件:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    emit configSaved();
    qDebug() << "配置保存完成，共" << m_stickerConfigs.size() << "个贴纸";
    return true;
}

void StickerManager::saveConfig(const QString &filePath)
{
    saveConfig(); // 简化版本，使用默认路径
}

QList<StickerConfig> StickerManager::getAllConfigs() const
{
    QList<StickerConfig> configs;

    // 从窗口获取最新配置
    for (const StickerConfig &config : m_stickerConfigs) {
        auto it = m_stickerWidgets.find(config.id);
        if (it != m_stickerWidgets.end()) {
            configs.append(it.value()->getConfig());
        } else {
            configs.append(config);
        }
    }

    return configs;
}

StickerWidget* StickerManager::getStickerWidget(const QString &stickerId) const
{
    auto it = m_stickerWidgets.find(stickerId);
    return (it != m_stickerWidgets.end()) ? it.value() : nullptr;
}

void StickerManager::onStickerConfigChanged(const StickerConfig &config)
{
    // 更新存储的配置
    for (StickerConfig &storedConfig : m_stickerConfigs) {
        if (storedConfig.id == config.id) {
            storedConfig = config;
            break;
        }
    }

    emit stickerConfigChanged(config);
}

void StickerManager::onStickerDeleteRequested(const QString &stickerId)
{
    deleteSticker(stickerId);
}

void StickerManager::onStickerEditRequested(const QString &stickerId)
{
    // 找到对应的配置并发送信号
    for (const StickerConfig &config : m_stickerConfigs) {
        if (config.id == stickerId) {
            emit stickerConfigChanged(config);
            break;
        }
    }
}

void StickerManager::onAutoSaveTimer()
{
    saveConfig();
    qDebug() << "自动保存配置完成";
}

void StickerManager::onConfigsRequested()
{
    emit stickerConfigsUpdated(getAllConfigs());
}

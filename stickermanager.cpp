#include "StickerManager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QThread>
#include <QUuid>

namespace {
bool isOnThread(const QObject *obj)
{
    return QThread::currentThread() == obj->thread();
}
}

StickerManager* StickerManager::instance()
{
    static StickerManager *s_instance = new StickerManager(QCoreApplication::instance());
    return s_instance;
}

StickerManager::StickerManager(QObject *parent)
    : QObject(parent)
    , m_repository()
    , m_runtime(this)
    , m_hasSticker(false)
    , m_autoSaveTimer(new QTimer(this))
    , m_isCleanedUp(false)
{
    m_autoSaveTimer->setSingleShot(false);
    m_autoSaveTimer->setInterval(30000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &StickerManager::onAutoSaveTimer);
    m_autoSaveTimer->start();

    loadConfigInternal();
    updateRuntimeConnections();

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

    if (m_autoSaveTimer) {
        m_autoSaveTimer->stop();
    }

    saveConfigInternal();
    destroyStickerInternal();

    QMutexLocker locker(&m_mutex);
    m_hasSticker = false;

    qDebug() << "贴纸管理器清理完成";
}

void StickerManager::createSticker()
{
    StickerConfig config;
    config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    config.name = QString("贴纸 1");
    config.position = QPoint(100, 100);
    config.size = QSize(200, 200);
    config.isDesktopMode = true;
    config.visible = true;
    config.opacity = 1.0;

    createStickerInternal(config);
}

void StickerManager::createStickerInternal(const StickerConfig &config)
{
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, config]() { createStickerInternal(config); }, Qt::QueuedConnection);
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_config = config;
        m_hasSticker = true;
    }

    m_runtime.createOrUpdate(config);
    updateRuntimeConnections();

    emit stickerCreated(config);
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "贴纸创建完成:" << config.id;
}

void StickerManager::deleteSticker(const QString &stickerId)
{
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, stickerId]() { deleteSticker(stickerId); }, Qt::QueuedConnection);
        return;
    }

    StickerConfig oldConfig;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_hasSticker || m_config.id != stickerId) {
            return;
        }
        oldConfig = m_config;
        m_hasSticker = false;
    }

    destroyStickerInternal();
    m_repository.clear();

    emit stickerDeleted(oldConfig.id);
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "贴纸删除完成:" << stickerId;
}

void StickerManager::editSticker(const QString &stickerId, const StickerConfig &config)
{
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, stickerId, config]() { editSticker(stickerId, config); }, Qt::QueuedConnection);
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!m_hasSticker || m_config.id != stickerId) {
            m_config = config;
            m_hasSticker = true;
        } else {
            m_config = config;
        }
    }

    m_runtime.createOrUpdate(config);
    updateRuntimeConnections();

    emit stickerConfigChanged(config);
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "贴纸编辑完成:" << stickerId;
}

void StickerManager::editStickerFromMainWindow(const QString &stickerId)
{
    QMutexLocker locker(&m_mutex);
    if (m_hasSticker && m_config.id == stickerId) {
        emit stickerConfigChanged(m_config);
    }
}

void StickerManager::destroyStickerInternal()
{
    m_runtime.destroy();
}

void StickerManager::updateRuntimeConnections()
{
    StickerWidget *widget = m_runtime.widget();
    if (!widget) {
        return;
    }

    widget->disconnect(this);
    connect(widget, &StickerWidget::configChanged, this, &StickerManager::onStickerConfigChanged);
    connect(widget, &StickerWidget::deleteRequested, this, &StickerManager::onStickerDeleteRequested);
    connect(widget, &StickerWidget::editRequested, this, &StickerManager::onStickerEditRequested);
}

bool StickerManager::loadConfig()
{
    if (!isOnThread(this)) {
        bool result = false;
        QMetaObject::invokeMethod(this, [this, &result]() { result = loadConfigInternal(); }, Qt::BlockingQueuedConnection);
        return result;
    }
    return loadConfigInternal();
}

bool StickerManager::saveConfig()
{
    if (!isOnThread(this)) {
        bool result = false;
        QMetaObject::invokeMethod(this, [this, &result]() { result = saveConfigInternal(); }, Qt::BlockingQueuedConnection);
        return result;
    }
    return saveConfigInternal();
}

void StickerManager::loadConfig(const QString &filePath)
{
    Q_UNUSED(filePath)
    loadConfig();
}

void StickerManager::saveConfig(const QString &filePath)
{
    Q_UNUSED(filePath)
    saveConfig();
}

bool StickerManager::loadConfigInternal()
{
    StickerConfig config;
    bool hasData = false;
    m_repository.load(config, hasData);

    if (!hasData) {
        {
            QMutexLocker locker(&m_mutex);
            m_hasSticker = false;
        }
        emit stickerConfigsUpdated(getAllConfigs());
        return false;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_config = config;
        m_hasSticker = true;
    }

    m_runtime.createOrUpdate(config);
    updateRuntimeConnections();

    emit configLoaded(getAllConfigs());
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "配置加载完成";
    return true;
}

bool StickerManager::saveConfigInternal()
{
    StickerConfig config;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_hasSticker) {
            return m_repository.clear();
        }
        config = m_config;
    }

    if (!m_repository.save(config)) {
        return false;
    }

    emit configSaved();
    qDebug() << "配置保存完成";
    return true;
}

QList<StickerConfig> StickerManager::getAllConfigs() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_hasSticker) {
        return {};
    }
    return {m_config};
}

StickerWidget* StickerManager::getStickerWidget(const QString &stickerId) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_hasSticker || m_config.id != stickerId) {
        return nullptr;
    }
    return m_runtime.widget();
}

void StickerManager::onStickerConfigChanged(const StickerConfig &config)
{
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, config]() { onStickerConfigChanged(config); }, Qt::QueuedConnection);
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_config = config;
        m_hasSticker = true;
    }

    emit stickerConfigChanged(config);
    emit stickerConfigsUpdated(getAllConfigs());
}

void StickerManager::onStickerDeleteRequested(const QString &stickerId)
{
    deleteSticker(stickerId);
}

void StickerManager::onStickerEditRequested(const QString &stickerId)
{
    editStickerFromMainWindow(stickerId);
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

void StickerManager::createDefaultSticker()
{
    StickerConfig config;
    config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    config.name = "默认贴纸";
    config.imagePath = "";
    config.position = QPoint(200, 200);
    config.size = QSize(200, 200);
    config.isDesktopMode = true;
    config.visible = true;
    config.opacity = 1.0;

    createStickerInternal(config);
}

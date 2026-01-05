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
    , m_followController(&m_runtime, this)
    , m_autoSaveTimer(new QTimer(this))
    , m_isCleanedUp(false)
{
    m_autoSaveTimer->setSingleShot(false);
    m_autoSaveTimer->setInterval(30000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &StickerManager::onAutoSaveTimer);
    m_autoSaveTimer->start();

    connectRuntimeSignals();
    loadConfigInternal();

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

    m_followController.clear();
    saveConfigInternal();
    destroyStickerInternal();

    QMutexLocker locker(&m_mutex);
    m_configs.clear();

    qDebug() << "贴纸管理器清理完成";
}

void StickerManager::createSticker()
{
    StickerConfig config;
    config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    int nextIndex = 1;
    {
        QMutexLocker locker(&m_mutex);
        nextIndex = m_configs.size() + 1;
    }
    config.name = QString("贴纸 %1").arg(nextIndex);
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

    StickerConfig createdConfig = config;
    if (createdConfig.id.isEmpty()) {
        createdConfig.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    StickerInstance *instance = m_runtime.createOrUpdatePrimary(createdConfig);
    if (!instance || !instance->widget) {
        return;
    }

    StickerConfig actualConfig = instance->widget->getConfig();
    bool isNew = false;

    {
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(actualConfig.id);
        if (index >= 0) {
            m_configs[index] = actualConfig;
        } else {
            m_configs.append(actualConfig);
            isNew = true;
        }
    }

    if (isNew) {
        emit stickerCreated(actualConfig);
    } else {
        emit stickerConfigChanged(actualConfig);
    }
    m_followController.updateTemplate(actualConfig);
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "贴纸创建完成:" << actualConfig.id;
}

void StickerManager::deleteSticker(const QString &stickerId)
{
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, stickerId]() { deleteSticker(stickerId); }, Qt::QueuedConnection);
        return;
    }

    StickerConfig oldConfig;
    bool removed = false;
    {
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(stickerId);
        if (index < 0) {
            return;
        }
        oldConfig = m_configs.at(index);
        m_configs.removeAt(index);
        removed = true;
    }

    if (!removed) {
        return;
    }

    m_followController.removeTemplate(stickerId);
    m_runtime.destroyInstancesForTemplate(stickerId);

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

    StickerConfig updatedConfig = config;
    updatedConfig.id = stickerId;

    StickerInstance *instance = m_runtime.createOrUpdatePrimary(updatedConfig);
    if (!instance || !instance->widget) {
        return;
    }

    StickerConfig actualConfig = instance->widget->getConfig();
    bool isNew = false;

    {
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(actualConfig.id);
        if (index >= 0) {
            m_configs[index] = actualConfig;
        } else {
            m_configs.append(actualConfig);
            isNew = true;
        }
    }

    if (isNew) {
        emit stickerCreated(actualConfig);
    }
    emit stickerConfigChanged(actualConfig);
    m_followController.updateTemplate(actualConfig);
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "贴纸编辑完成:" << stickerId;
}

void StickerManager::editStickerFromMainWindow(const QString &stickerId)
{
    QMutexLocker locker(&m_mutex);
    int index = findConfigIndex(stickerId);
    if (index >= 0) {
        emit stickerConfigChanged(m_configs.at(index));
    }
}

void StickerManager::destroyStickerInternal()
{
    m_runtime.clear();
}

void StickerManager::connectRuntimeSignals()
{
    connect(&m_runtime, &StickerRuntime::instanceConfigChanged,
            this, &StickerManager::onInstanceConfigChanged, Qt::UniqueConnection);
    connect(&m_runtime, &StickerRuntime::instanceDeleteRequested,
            this, &StickerManager::onInstanceDeleteRequested, Qt::UniqueConnection);
    connect(&m_runtime, &StickerRuntime::instanceEditRequested,
            this, &StickerManager::onInstanceEditRequested, Qt::UniqueConnection);
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
    QList<StickerConfig> configs;
    bool hasData = false;
    m_repository.load(configs, hasData);

    if (!hasData || configs.isEmpty()) {
        m_runtime.clear();
        m_followController.clear();
        {
            QMutexLocker locker(&m_mutex);
            m_configs.clear();
        }
        emit stickerConfigsUpdated(getAllConfigs());
        return false;
    }

    m_runtime.clear();

    QList<StickerConfig> actualConfigs;
    actualConfigs.reserve(configs.size());
    for (StickerConfig config : configs) {
        if (config.id.isEmpty()) {
            config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        StickerInstance *instance = m_runtime.createOrUpdatePrimary(config);
        if (instance && instance->widget) {
            actualConfigs.append(instance->widget->getConfig());
        }
    }

    {
        QMutexLocker locker(&m_mutex);
        m_configs = actualConfigs;
    }

    m_followController.setTemplates(actualConfigs);
    emit configLoaded(actualConfigs);
    emit stickerConfigsUpdated(actualConfigs);
    qDebug() << "配置加载完成";
    return true;
}

bool StickerManager::saveConfigInternal()
{
    QList<StickerConfig> configs;
    {
        QMutexLocker locker(&m_mutex);
        configs = m_configs;
    }

    if (configs.isEmpty()) {
        return m_repository.clear();
    }

    if (!m_repository.save(configs)) {
        return false;
    }

    emit configSaved();
    qDebug() << "配置保存完成";
    return true;
}

QList<StickerConfig> StickerManager::getAllConfigs() const
{
    QMutexLocker locker(&m_mutex);
    return m_configs;
}

StickerWidget* StickerManager::getStickerWidget(const QString &stickerId) const
{
    QMutexLocker locker(&m_mutex);
    if (findConfigIndex(stickerId) < 0) {
        return nullptr;
    }
    return m_runtime.widget(stickerId);
}

void StickerManager::onInstanceConfigChanged(const QString &instanceId,
                                             const StickerConfig &config,
                                             bool syncToTemplate)
{
    Q_UNUSED(instanceId)
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, instanceId, config, syncToTemplate]() {
            onInstanceConfigChanged(instanceId, config, syncToTemplate);
        }, Qt::QueuedConnection);
        return;
    }

    if (config.id.isEmpty()) {
        return;
    }

    if (!syncToTemplate) {
        emit stickerConfigsUpdated(getAllConfigs());
        return;
    }

    bool isNew = false;
    {
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(config.id);
        if (index >= 0) {
            m_configs[index] = config;
        } else {
            m_configs.append(config);
            isNew = true;
        }
    }

    if (isNew) {
        emit stickerCreated(config);
    }
    emit stickerConfigChanged(config);
    m_followController.updateTemplate(config);
    emit stickerConfigsUpdated(getAllConfigs());
}

void StickerManager::onInstanceDeleteRequested(const QString &instanceId, const QString &templateId)
{
    Q_UNUSED(instanceId)
    deleteSticker(templateId);
}

void StickerManager::onInstanceEditRequested(const QString &instanceId, const QString &templateId)
{
    Q_UNUSED(instanceId)
    editStickerFromMainWindow(templateId);
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

void StickerManager::lockStickerToWindow(const QString &stickerId, qulonglong windowHandle)
{
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, stickerId, windowHandle]() {
            lockStickerToWindow(stickerId, windowHandle);
        }, Qt::QueuedConnection);
        return;
    }

    if (stickerId.isEmpty() || windowHandle == 0) {
        return;
    }

    StickerConfig templateConfig;
    {
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(stickerId);
        if (index < 0) {
            return;
        }
        templateConfig = m_configs.at(index);
    }

    templateConfig.follow.enabled = true;
    m_followController.updateTemplate(templateConfig);

    StickerConfig lockedConfig;
    if (!m_followController.lockToTargetWindow(stickerId,
                                               static_cast<WindowHandle>(windowHandle),
                                               &lockedConfig)) {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(stickerId);
        if (index >= 0) {
            m_configs[index] = lockedConfig;
        }
    }

    emit stickerConfigChanged(lockedConfig);
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "贴纸" << stickerId << "已锁定跟随窗口";
}

void StickerManager::unlockStickerTarget(const QString &stickerId)
{
    if (!isOnThread(this)) {
        QMetaObject::invokeMethod(this, [this, stickerId]() {
            unlockStickerTarget(stickerId);
        }, Qt::QueuedConnection);
        return;
    }

    if (stickerId.isEmpty()) {
        return;
    }

    StickerConfig updatedConfig;
    {
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(stickerId);
        if (index < 0) {
            return;
        }
        updatedConfig = m_configs.at(index);
        updatedConfig.follow.targetProcessName.clear();
        m_configs[index] = updatedConfig;
    }

    StickerInstance *instance = m_runtime.createOrUpdatePrimary(updatedConfig);
    if (instance && instance->widget) {
        updatedConfig = instance->widget->getConfig();
        updatedConfig.follow.targetProcessName.clear();
        QMutexLocker locker(&m_mutex);
        int index = findConfigIndex(stickerId);
        if (index >= 0) {
            m_configs[index] = updatedConfig;
        }
    }

    m_followController.clearTarget(stickerId);
    m_followController.updateTemplate(updatedConfig);
    emit stickerConfigChanged(updatedConfig);
    emit stickerConfigsUpdated(getAllConfigs());
    qDebug() << "贴纸" << stickerId << "已取消锚定窗口";
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

int StickerManager::findConfigIndex(const QString &stickerId) const
{
    for (int i = 0; i < m_configs.size(); ++i) {
        if (m_configs.at(i).id == stickerId) {
            return i;
        }
    }
    return -1;
}

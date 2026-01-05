#include "stickerfollowcontroller.h"
#include "stickerinstance.h"
#include "stickerruntime.h"
#include "stickerwidget.h"
#include <QRegularExpression>
#include <QtMath>

StickerFollowController::StickerFollowController(StickerRuntime *runtime, QObject *parent)
    : QObject(parent)
    , m_runtime(runtime)
    , m_windowService(this)
    , m_timer(new QTimer(this))
    , m_refreshing(false)
{
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &StickerFollowController::refresh);
}

void StickerFollowController::setRuntime(StickerRuntime *runtime)
{
    m_runtime = runtime;
}

void StickerFollowController::setTemplates(const QList<StickerConfig> &configs)
{
    QSet<QString> seen;
    for (const StickerConfig &config : configs) {
        if (config.id.isEmpty()) {
            continue;
        }
        updateTemplate(config);
        seen.insert(config.id);
    }

    QList<QString> existing = m_templates.keys();
    for (const QString &id : existing) {
        if (!seen.contains(id)) {
            removeTemplate(id);
        }
    }

    if (!m_refreshing) {
        refresh();
    }
}

void StickerFollowController::updateTemplate(const StickerConfig &config)
{
    if (config.id.isEmpty()) {
        return;
    }

    TemplateState &state = m_templates[config.id];
    state.config = config;

    updateTemplateVisibility(config);

    if (!config.follow.enabled) {
        removeAllInstances(state);
        state.primaryHandle = 0;
    }

    syncTimer();
    if (!m_refreshing) {
        refresh();
    }
}

void StickerFollowController::removeTemplate(const QString &templateId)
{
    auto it = m_templates.find(templateId);
    if (it == m_templates.end()) {
        return;
    }

    removeAllInstances(it.value());
    m_templates.erase(it);
    syncTimer();
}

void StickerFollowController::clear()
{
    QList<QString> keys = m_templates.keys();
    for (const QString &id : keys) {
        removeTemplate(id);
    }
    if (m_timer->isActive()) {
        m_timer->stop();
    }
}

bool StickerFollowController::isActive() const
{
    return m_timer->isActive();
}

bool StickerFollowController::lockToTargetWindow(const QString &templateId,
                                                 WindowHandle handle,
                                                 StickerConfig *outConfig)
{
    if (!m_runtime || templateId.isEmpty() || handle == 0) {
        return false;
    }

    auto it = m_templates.find(templateId);
    if (it == m_templates.end()) {
        return false;
    }

    WindowInfo info = m_windowService.queryWindow(handle);
    if (info.handle == 0 || info.rect.isNull()) {
        return false;
    }

    TemplateState &state = it.value();
    StickerInstance *instance = m_runtime->instance(templateId);
    QSize stickerSize = resolveStickerSize(state.config, instance);
    QPoint stickerTopLeft = state.config.position;

    if (instance && instance->widget) {
        stickerSize = instance->widget->size();
        stickerTopLeft = instance->widget->frameGeometry().topLeft();
    }

    if (!stickerSize.isEmpty()) {
        state.config.follow.offset = computeOffsetForPosition(info.rect, stickerSize, stickerTopLeft,
                                                              state.config.follow);
    }

    state.primaryHandle = handle;
    updateTemplateVisibility(state.config);
    if (outConfig) {
        *outConfig = state.config;
    }

    syncTimer();
    if (!m_refreshing) {
        refresh();
    }
    return true;
}

void StickerFollowController::refresh()
{
    if (m_refreshing || !m_runtime) {
        return;
    }

    m_refreshing = true;

    QList<WindowInfo> windows;
    bool needEnumeration = false;
    for (auto it = m_templates.constBegin(); it != m_templates.constEnd(); ++it) {
        const StickerFollowConfig &follow = it.value().config.follow;
        if (!follow.enabled) {
            continue;
        }
        if (follow.batchMode || it.value().primaryHandle == 0) {
            needEnumeration = true;
            break;
        }
    }

    if (needEnumeration) {
        windows = m_windowService.listWindows(true);
    }

    for (auto it = m_templates.begin(); it != m_templates.end(); ++it) {
        if (!it.value().config.follow.enabled) {
            continue;
        }
        if (!it.value().config.follow.batchMode && it.value().primaryHandle != 0 && !needEnumeration) {
            WindowInfo info = m_windowService.queryWindow(it.value().primaryHandle);
            QList<WindowInfo> subset;
            if (info.handle != 0 && info.visible) {
                subset.append(info);
            }
            updateInstancesForTemplate(it.value(), subset);
        } else {
            updateInstancesForTemplate(it.value(), windows);
        }
    }

    m_refreshing = false;
}

void StickerFollowController::syncTimer()
{
    int interval = effectiveIntervalMs();
    if (interval <= 0) {
        if (m_timer->isActive()) {
            m_timer->stop();
        }
        return;
    }

    m_timer->setInterval(interval);
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

int StickerFollowController::effectiveIntervalMs() const
{
    int interval = -1;
    for (auto it = m_templates.constBegin(); it != m_templates.constEnd(); ++it) {
        const StickerFollowConfig &follow = it.value().config.follow;
        if (!follow.enabled) {
            continue;
        }
        int candidate = qMax(16, follow.pollIntervalMs);
        if (interval < 0 || candidate < interval) {
            interval = candidate;
        }
    }
    return interval;
}

QList<WindowInfo> StickerFollowController::filterWindows(const StickerFollowConfig &follow,
                                                         const QList<WindowInfo> &windows) const
{
    QList<WindowInfo> result;
    if (!follow.enabled || follow.filterValue.trimmed().isEmpty()) {
        return result;
    }

    for (const WindowInfo &info : windows) {
        if (matchesFilter(follow, info)) {
            result.append(info);
        }
    }
    return result;
}

bool StickerFollowController::matchesFilter(const StickerFollowConfig &follow, const WindowInfo &info) const
{
    switch (follow.filterType) {
    case FollowFilterType::WindowClass:
        return info.className.contains(follow.filterValue, Qt::CaseInsensitive);
    case FollowFilterType::ProcessName:
        return info.processName.contains(follow.filterValue, Qt::CaseInsensitive);
    case FollowFilterType::WindowTitleRegex: {
        QRegularExpression regex(follow.filterValue, QRegularExpression::CaseInsensitiveOption);
        if (!regex.isValid()) {
            return false;
        }
        return regex.match(info.title).hasMatch();
    }
    default:
        break;
    }
    return false;
}

QPoint StickerFollowController::computeAnchoredPosition(const QRectF &windowRect,
                                                        const QSize &stickerSize,
                                                        const StickerFollowConfig &follow) const
{
    QPointF offset = computeOffset(windowRect, follow);
    QPointF anchorPoint;

    switch (follow.anchor) {
    case FollowAnchor::LeftTop:
        anchorPoint = windowRect.topLeft();
        break;
    case FollowAnchor::LeftBottom:
        anchorPoint = windowRect.bottomLeft();
        break;
    case FollowAnchor::RightTop:
        anchorPoint = windowRect.topRight();
        break;
    case FollowAnchor::RightBottom:
        anchorPoint = windowRect.bottomRight();
        break;
    default:
        anchorPoint = windowRect.topLeft();
        break;
    }

    QPointF target = anchorPoint + offset;
    QPointF topLeft = target;

    if (follow.anchor == FollowAnchor::RightTop || follow.anchor == FollowAnchor::RightBottom) {
        topLeft.setX(target.x() - stickerSize.width());
    }
    if (follow.anchor == FollowAnchor::LeftBottom || follow.anchor == FollowAnchor::RightBottom) {
        topLeft.setY(target.y() - stickerSize.height());
    }

    return QPoint(qRound(topLeft.x()), qRound(topLeft.y()));
}

QPointF StickerFollowController::computeOffset(const QRectF &windowRect,
                                               const StickerFollowConfig &follow) const
{
    if (follow.offsetMode == FollowOffsetMode::RelativeRatio) {
        return QPointF(windowRect.width() * follow.offset.x(),
                       windowRect.height() * follow.offset.y());
    }
    return follow.offset;
}

QPointF StickerFollowController::computeOffsetForPosition(const QRectF &windowRect,
                                                          const QSize &stickerSize,
                                                          const QPoint &stickerTopLeft,
                                                          const StickerFollowConfig &follow) const
{
    QPointF anchorPoint;
    switch (follow.anchor) {
    case FollowAnchor::LeftTop:
        anchorPoint = windowRect.topLeft();
        break;
    case FollowAnchor::LeftBottom:
        anchorPoint = windowRect.bottomLeft();
        break;
    case FollowAnchor::RightTop:
        anchorPoint = windowRect.topRight();
        break;
    case FollowAnchor::RightBottom:
        anchorPoint = windowRect.bottomRight();
        break;
    default:
        anchorPoint = windowRect.topLeft();
        break;
    }

    QPointF offsetPx = QPointF(stickerTopLeft) - anchorPoint;
    if (follow.anchor == FollowAnchor::RightTop || follow.anchor == FollowAnchor::RightBottom) {
        offsetPx.setX(offsetPx.x() + stickerSize.width());
    }
    if (follow.anchor == FollowAnchor::LeftBottom || follow.anchor == FollowAnchor::RightBottom) {
        offsetPx.setY(offsetPx.y() + stickerSize.height());
    }

    if (follow.offsetMode == FollowOffsetMode::RelativeRatio) {
        const double width = windowRect.width();
        const double height = windowRect.height();
        return QPointF(width > 0.0 ? offsetPx.x() / width : 0.0,
                       height > 0.0 ? offsetPx.y() / height : 0.0);
    }

    return offsetPx;
}

QSize StickerFollowController::resolveStickerSize(const StickerConfig &config,
                                                  const StickerInstance *instance) const
{
    if (!config.size.isEmpty()) {
        return config.size;
    }
    if (instance && !instance->config.size.isEmpty()) {
        return instance->config.size;
    }
    if (instance && instance->widget) {
        return instance->widget->size();
    }
    return QSize();
}

QString StickerFollowController::makeInstanceId(const QString &templateId, WindowHandle handle) const
{
    return QString("%1@%2").arg(templateId, QString::number(handle, 16));
}

void StickerFollowController::updateInstancesForTemplate(TemplateState &state,
                                                         const QList<WindowInfo> &windows)
{
    const StickerFollowConfig &follow = state.config.follow;
    QList<WindowInfo> matched;
    if (!follow.batchMode && state.primaryHandle != 0) {
        for (const WindowInfo &info : windows) {
            if (info.handle == state.primaryHandle) {
                matched.append(info);
                break;
            }
        }
    }
    if (matched.isEmpty()) {
        matched = filterWindows(follow, windows);
    }
    QSet<WindowHandle> aliveHandles;

    if (matched.isEmpty()) {
        removeStaleInstances(state, aliveHandles);
        state.primaryHandle = 0;
        return;
    }

    if (!follow.batchMode) {
        WindowHandle preferred = state.primaryHandle;
        bool preferredValid = false;
        if (preferred != 0) {
            for (const WindowInfo &info : matched) {
                if (info.handle == preferred) {
                    preferredValid = true;
                    break;
                }
            }
        }

        WindowInfo target = matched.first();
        if (preferredValid) {
            for (const WindowInfo &info : matched) {
                if (info.handle == preferred) {
                    target = info;
                    break;
                }
            }
        }

        state.primaryHandle = target.handle;
        aliveHandles.insert(target.handle);

        QString instanceId = makeInstanceId(state.config.id, target.handle);
        StickerInstance *existing = m_runtime->instance(instanceId);
        QSize stickerSize = resolveStickerSize(state.config, existing);
        StickerConfig instanceConfig = state.config;
        if (!stickerSize.isEmpty()) {
            instanceConfig.size = stickerSize;
        }
        instanceConfig.position = computeAnchoredPosition(target.rect, instanceConfig.size, follow);
        instanceConfig.visible = state.config.visible
            && (!target.minimized || !follow.hideWhenMinimized);

        StickerInstance *instance = m_runtime->createOrUpdateInstance(instanceConfig, instanceId,
                                                                      state.config.id, false);
        if (instance && instance->widget) {
            m_attachmentService.attach(instance->widget, target.handle);
        }
        state.instanceIds[target.handle] = instanceId;
        removeStaleInstances(state, aliveHandles);
        return;
    }

    for (const WindowInfo &info : matched) {
        aliveHandles.insert(info.handle);
        QString instanceId = makeInstanceId(state.config.id, info.handle);
        StickerInstance *existing = m_runtime->instance(instanceId);
        QSize stickerSize = resolveStickerSize(state.config, existing);
        StickerConfig instanceConfig = state.config;
        if (!stickerSize.isEmpty()) {
            instanceConfig.size = stickerSize;
        }
        instanceConfig.position = computeAnchoredPosition(info.rect, instanceConfig.size, follow);
        instanceConfig.visible = state.config.visible
            && (!info.minimized || !follow.hideWhenMinimized);

        StickerInstance *instance = m_runtime->createOrUpdateInstance(instanceConfig, instanceId,
                                                                      state.config.id, false);
        if (instance && instance->widget) {
            m_attachmentService.attach(instance->widget, info.handle);
        }
        state.instanceIds[info.handle] = instanceId;
    }

    removeStaleInstances(state, aliveHandles);
}

void StickerFollowController::updateTemplateVisibility(const StickerConfig &config)
{
    if (!m_runtime) {
        return;
    }

    const TemplateState *state = nullptr;
    auto it = m_templates.constFind(config.id);
    if (it != m_templates.constEnd()) {
        state = &it.value();
    }

    StickerWidget *widget = m_runtime->widget(config.id);
    if (!widget) {
        return;
    }

    bool hideTemplate = false;
    if (config.follow.enabled) {
        if (config.follow.batchMode) {
            hideTemplate = true;
        } else if (state && state->primaryHandle != 0) {
            hideTemplate = true;
        }
    }

    widget->setRuntimeHidden(hideTemplate);
}

void StickerFollowController::removeStaleInstances(TemplateState &state,
                                                   const QSet<WindowHandle> &aliveHandles)
{
    for (auto it = state.instanceIds.begin(); it != state.instanceIds.end(); ) {
        if (aliveHandles.contains(it.key())) {
            ++it;
            continue;
        }
        if (m_runtime) {
            m_runtime->destroyInstance(it.value());
        }
        it = state.instanceIds.erase(it);
    }
}

void StickerFollowController::removeAllInstances(TemplateState &state)
{
    for (auto it = state.instanceIds.begin(); it != state.instanceIds.end(); ++it) {
        if (m_runtime) {
            m_runtime->destroyInstance(it.value());
        }
    }
    state.instanceIds.clear();
}

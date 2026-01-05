#ifndef STICKERFOLLOWCONTROLLER_H
#define STICKERFOLLOWCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QSet>
#include <QTimer>
#include "stickerdata.h"
#include "windowattachmentservice.h"
#include "windowrecognitionservice.h"

class StickerRuntime;
struct StickerInstance;

class StickerFollowController : public QObject
{
    Q_OBJECT

public:
    explicit StickerFollowController(StickerRuntime *runtime = nullptr, QObject *parent = nullptr);

    void setRuntime(StickerRuntime *runtime);
    void setTemplates(const QList<StickerConfig> &configs);
    void updateTemplate(const StickerConfig &config);
    void removeTemplate(const QString &templateId);
    void clear();
    bool isActive() const;
    bool lockToTargetWindow(const QString &templateId, WindowHandle handle, StickerConfig *outConfig = nullptr);

private slots:
    void refresh();

private:
    struct TemplateState {
        StickerConfig config;
        QHash<WindowHandle, QString> instanceIds;
        WindowHandle primaryHandle = 0;
    };

    void syncTimer();
    int effectiveIntervalMs() const;
    QList<WindowInfo> filterWindows(const StickerFollowConfig &follow,
                                    const QList<WindowInfo> &windows) const;
    bool matchesFilter(const StickerFollowConfig &follow, const WindowInfo &info) const;
    QPoint computeAnchoredPosition(const QRectF &windowRect,
                                   const QSize &stickerSize,
                                   const StickerFollowConfig &follow) const;
    QPointF computeOffset(const QRectF &windowRect, const StickerFollowConfig &follow) const;
    QPointF computeOffsetForPosition(const QRectF &windowRect,
                                     const QSize &stickerSize,
                                     const QPoint &stickerTopLeft,
                                     const StickerFollowConfig &follow) const;
    QSize resolveStickerSize(const StickerConfig &config, const StickerInstance *instance) const;
    QString makeInstanceId(const QString &templateId, WindowHandle handle) const;
    void updateInstancesForTemplate(TemplateState &state, const QList<WindowInfo> &windows);
    void updateTemplateVisibility(const StickerConfig &config);
    void removeStaleInstances(TemplateState &state, const QSet<WindowHandle> &aliveHandles);
    void removeAllInstances(TemplateState &state);

    StickerRuntime *m_runtime;
    WindowAttachmentService m_attachmentService;
    WindowRecognitionService m_windowService;
    QTimer *m_timer;
    QHash<QString, TemplateState> m_templates;
    bool m_refreshing;
};

#endif // STICKERFOLLOWCONTROLLER_H

#ifndef STICKERWIDGET_H
#define STICKERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QMenu>
#include <QPropertyAnimation>
#include "StickerData.h"
#include "EventHandler.h"
#include "stickerimage.h"
#include "stickerrenderer.h"

class StickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StickerWidget(const StickerConfig &config, QWidget *parent = nullptr);
    ~StickerWidget();

    // 获取/设置配置
    StickerConfig getConfig() const;
    void updateConfig(const StickerConfig &config);

    // 显示/隐藏控制
    void setVisible(bool visible) override;
    void setOpacity(double opacity);

    // 模式切换
    void setDesktopMode(bool isDesktop);
    void setAllowDrag(bool allowDrag);      // 新增
    void setClickThrough(bool clickThrough); // 新增

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void configChanged(const StickerConfig &config);
    void deleteRequested(const QString &stickerId);
    void editRequested(const QString &stickerId);

private slots:
    void onAnimationTimer();
    void onEditSticker();
    void onDeleteSticker();
    void onToggleMode();
    void onToggleDrag();        // 新增
    void onToggleClickThrough(); // 新增
    void onToggleEditMode();

private:
    enum class ResizeAnchor {
        KeepCenter,
        KeepTopLeft
    };

    void initializeWidget();
    void loadStickerImage(const QString &imagePath);
    void createDefaultSticker();
    void applyMask();
    void updateTransformedWindowSize(ResizeAnchor anchor = ResizeAnchor::KeepCenter);
    void setupContextMenu();
    void updateContextMenuState();  // 添加这个函数声明
    void handleMouseTrigger(MouseTrigger trigger);
    void updateWindowFlags();
    void updateClickThrough(); // 新增
    void setEditMode(bool enabled);
    double angleFromCenter(const QPoint &point) const;

    StickerConfig m_config;
    EventHandler *m_eventHandler;

    StickerImage m_image;
    StickerRenderer m_renderer;
    QPoint m_dragPosition;
    bool m_dragging;
    bool m_initialized;
    double m_animationAngle;
    bool m_editMode;
    bool m_rotating;
    double m_rotateStartAngle;
    double m_rotateStartRotation;

    QTimer *m_animationTimer;
    QPropertyAnimation *m_opacityAnimation;
    QMenu *m_contextMenu;

    // 右键菜单项
    QAction *m_editAction;
    QAction *m_toggleModeAction;
    QAction *m_toggleDragAction;        // 新增
    QAction *m_toggleClickThroughAction; // 新增
    QAction *m_editModeAction;
    QAction *m_deleteAction;
};

#endif // STICKERWIDGET_H

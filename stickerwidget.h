#ifndef STICKERWIDGET_H
#define STICKERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QPropertyAnimation>
#include "StickerData.h"
#include "stickereventcontroller.h"
#include "stickercontextmenucontroller.h"
#include "stickereditcontroller.h"
#include "stickerimage.h"
#include "stickerinteractioncontroller.h"
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
    void updateContextMenuState();
    void handleMouseTrigger(MouseTrigger trigger);
    void updateClickThrough(); // 新增
    void setEditMode(bool enabled);

    StickerConfig m_config;
    StickerImage m_image;
    StickerRenderer m_renderer;
    StickerInteractionController m_interactionController;
    StickerEditController m_editController;
    StickerContextMenuController m_menuController;
    StickerEventController m_eventController;
    bool m_initialized;
    double m_animationAngle;

    QTimer *m_animationTimer;
    QPropertyAnimation *m_opacityAnimation;
};

#endif // STICKERWIDGET_H

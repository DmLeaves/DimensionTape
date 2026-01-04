#include "StickerWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>
#include <QApplication>
#include <QScreen>
#include <QFileInfo>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#include <QtMath>
#include "stickertransformlayout.h"

StickerWidget::StickerWidget(const StickerConfig &config, QWidget *parent)
    : QWidget(parent)
    , m_config(config)
    , m_image(600)
    , m_renderer(&m_image)
    , m_interactionController()
    , m_editController(this, this)
    , m_menuController(this)
    , m_eventController(this)
    , m_initialized(false)
    , m_animationAngle(0)
{
    // 基础设置
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    // 设置窗口大小和位置
    if (!m_config.size.isEmpty()) {
        setFixedSize(m_config.size);
    } else {
        setFixedSize(200, 200);
        m_config.size = QSize(200, 200);
    }

    if (!m_config.position.isNull()) {
        move(m_config.position);
    } else {
        // 默认居中
        if (QScreen *screen = QApplication::primaryScreen()) {
            QRect screenRect = screen->geometry();
            int x = (screenRect.width() - width()) / 2;
            int y = (screenRect.height() - height()) / 2;
            move(x, y);
            m_config.position = QPoint(x, y);
        }
    }

    // 创建动画定时器
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &StickerWidget::onAnimationTimer);
    m_animationTimer->start(50); // 20 FPS

    // 设置透明度动画
    m_opacityAnimation = new QPropertyAnimation(this, "windowOpacity");
    m_opacityAnimation->setDuration(300);
    m_opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // 设置上下文菜单
    setupContextMenu();

    StickerInteractionController::Callbacks interactionCallbacks;
    interactionCallbacks.widgetRect = [this]() { return rect(); };
    interactionCallbacks.frameGeometry = [this]() { return frameGeometry(); };
    interactionCallbacks.moveWindow = [this](const QPoint &pos) { move(pos); };
    interactionCallbacks.updateTransformLayout = [this]() {
        updateTransformedWindowSize(ResizeAnchor::KeepCenter);
    };
    interactionCallbacks.applyMask = [this]() { applyMask(); };
    interactionCallbacks.requestUpdate = [this]() { update(); };
    interactionCallbacks.notifyConfigChanged = [this]() { emit configChanged(m_config); };
    m_interactionController.setCallbacks(std::move(interactionCallbacks));

    connect(&m_editController, &StickerEditController::editModeChanged, this, [this](bool) {
        updateContextMenuState();
        update();
    });

    m_eventController.setEvents(&m_config.events);

    // 连接事件处理器信号
    connect(&m_eventController, &StickerEventController::eventExecuted, [this](const QString &message) {
        qDebug() << "贴纸事件执行成功:" << message;
    });

    connect(&m_eventController, &StickerEventController::eventFailed, [this](const QString &error) {
        qDebug() << "贴纸事件执行失败:" << error;
    });

    // 初始化窗口
    initializeWidget();

    qDebug() << "创建贴纸:" << m_config.id << "位置:" << m_config.position << "大小:" << m_config.size;
}

StickerWidget::~StickerWidget()
{
    if (m_animationTimer) {
        m_animationTimer->stop();
    }
    qDebug() << "销毁贴纸:" << m_config.id;
}

void StickerWidget::initializeWidget()
{
    // 加载贴纸图像
    if (!m_config.imagePath.isEmpty() && QFileInfo::exists(m_config.imagePath)) {
        loadStickerImage(m_config.imagePath);
    } else {
        createDefaultSticker();
    }

    updateTransformedWindowSize(ResizeAnchor::KeepTopLeft);

    // 应用遮罩
    applyMask();

    // 设置窗口模式
    m_editController.applyWindowFlags(m_config.isDesktopMode, m_initialized);

    // 设置点击穿透
    updateClickThrough();

    // 设置透明度
    setOpacity(m_config.opacity);

    // 设置可见性
    setVisible(m_config.visible);

    m_initialized = true;

    qDebug() << "贴纸初始化完成:" << m_config.id;
}

void StickerWidget::loadStickerImage(const QString &imagePath)
{
    qDebug() << "加载贴纸图像:" << imagePath;

    if (!m_image.loadFromPath(imagePath)) {
        qDebug() << "无法加载图像，使用默认贴纸";
        createDefaultSticker();
        return;
    }

    const QPixmap &pixmap = m_image.pixmap();
    if (!pixmap.isNull() && pixmap.size() != size()) {
        setFixedSize(pixmap.size());
        m_config.size = pixmap.size();
    }

    qDebug() << "贴纸图像加载完成，大小:" << pixmap.size();
}

void StickerWidget::createDefaultSticker()
{
    m_image.createDefault();

    qDebug() << "默认贴纸创建完成";
}

void StickerWidget::applyMask()
{
    QBitmap mask = m_renderer.buildMask(m_config, size());
    if (!mask.isNull()) {
        setMask(mask);
    }
}

void StickerWidget::updateTransformedWindowSize(ResizeAnchor anchor)
{
    StickerTransformLayoutResult layout;
    if (!m_renderer.calculateLayout(m_config, layout)) {
        return;
    }

    if (m_editController.isEditMode()) {
        QTransform renderTransform = StickerTransformLayout::buildRenderTransform(layout, layout.windowSize);
        QRectF mapped = renderTransform.mapRect(layout.baseRect);
        QPointF windowCenter(layout.windowSize.width() / 2.0, layout.windowSize.height() / 2.0);
        QPointF centerOffset = mapped.center() - windowCenter;
        qDebug() << "变换调试"
                 << "baseSize" << m_image.baseSize()
                 << "contentRect" << m_image.contentRect()
                 << "windowSize" << layout.windowSize
                 << "bounds" << layout.bounds
                 << "baseRect" << layout.baseRect
                 << "mappedCenter" << mapped.center()
                 << "centerOffset" << centerOffset
                 << "pos" << pos()
                 << "cfgPos" << m_config.position
                 << "scale" << m_config.transform.scaleX << m_config.transform.scaleY
                 << "rot" << m_config.transform.rotation
                 << "shear" << m_config.transform.shearX << m_config.transform.shearY;
    }

    if (layout.windowSize != size()) {
        QPoint oldTopLeft = pos();
        QPoint oldCenter = geometry().center();
        setFixedSize(layout.windowSize);
        QPoint newTopLeft = oldTopLeft;
        if (anchor == ResizeAnchor::KeepCenter) {
            newTopLeft = oldCenter - QPoint(layout.windowSize.width() / 2,
                                            layout.windowSize.height() / 2);
        }
        move(newTopLeft);
    }

    bool configDirty = false;
    if (m_config.position != pos()) {
        m_config.position = pos();
        configDirty = true;
    }
    if (m_config.size != size()) {
        m_config.size = size();
        configDirty = true;
    }
    if (configDirty && m_initialized) {
        emit configChanged(m_config);
    }
}

void StickerWidget::updateClickThrough()
{
    if (m_config.clickThrough) {
        // 启用点击穿透
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        qDebug() << "贴纸" << m_config.id << "启用点击穿透";
    } else {
        // 禁用点击穿透
        setAttribute(Qt::WA_TransparentForMouseEvents, false);
        qDebug() << "贴纸" << m_config.id << "禁用点击穿透";
    }
}

void StickerWidget::paintEvent(QPaintEvent *)
{
    if (!m_initialized || m_image.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制贴纸图片（支持矩阵变换）
    m_renderer.paint(painter, m_config, size());

    // 默认贴纸的动画效果
    if (m_config.imagePath.isEmpty()) {
        int alpha = int(20 + 15 * qSin(m_animationAngle));
        painter.fillRect(rect(), QColor(150, 200, 255, alpha));
    }

    if (m_editController.isEditMode()) {
        QPen borderPen(QColor(0, 180, 0), 2);
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);
        QRect borderRect = rect().adjusted(1, 1, -2, -2);
        painter.drawRect(borderRect);
    }
}

void StickerWidget::mousePressEvent(QMouseEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    bool editMode = m_editController.isEditMode();
    if (m_config.clickThrough && !editMode) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (!editMode) {
            handleMouseTrigger(MouseTrigger::LeftClick);
        }

        if (m_interactionController.handleMousePress(event, m_config, m_config.allowDrag, editMode)) {
            event->accept();
            return;
        }
    } else if (event->button() == Qt::RightButton) {
        if (!editMode) {
            handleMouseTrigger(MouseTrigger::RightClick);
        }
        // 右键不再用于拖动，只触发右键事件
    }

    update();
}

void StickerWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    bool editMode = m_editController.isEditMode();
    if (m_config.clickThrough && !editMode) {
        event->ignore();
        return;
    }

    if (m_interactionController.handleMouseMove(event, m_config, m_config.allowDrag, editMode)) {
        event->accept();
        return;
    }
}

void StickerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    bool editMode = m_editController.isEditMode();
    if (m_config.clickThrough && !editMode) {
        event->ignore();
        return;
    }

    if (m_interactionController.handleMouseRelease(event, m_config)) {
        event->accept();
        return;
    }
}

void StickerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    if (m_config.clickThrough && !m_editController.isEditMode()) {
        event->ignore();
        return;
    }

    if (m_editController.isEditMode()) {
        event->ignore();
        return;
    }

    Q_UNUSED(event)
    handleMouseTrigger(MouseTrigger::DoubleClick);
}

void StickerWidget::wheelEvent(QWheelEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    bool editMode = m_editController.isEditMode();
    if (m_config.clickThrough && !editMode) {
        event->ignore();
        return;
    }

    if (m_interactionController.handleWheel(event, m_config, editMode)) {
        event->accept();
        return;
    }

    if (event->angleDelta().y() > 0) {
        handleMouseTrigger(MouseTrigger::WheelUp);
    } else {
        handleMouseTrigger(MouseTrigger::WheelDown);
    }
}

void StickerWidget::enterEvent(QEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    if (m_config.clickThrough && !m_editController.isEditMode()) {
        event->ignore();
        return;
    }

    Q_UNUSED(event)
    handleMouseTrigger(MouseTrigger::MouseEnter);

    // 鼠标进入时增加透明度
    if (m_opacityAnimation->state() != QAbstractAnimation::Running) {
        m_opacityAnimation->setStartValue(windowOpacity());
        m_opacityAnimation->setEndValue(qMin(1.0, m_config.opacity + 0.2));
        m_opacityAnimation->start();
    }
}

void StickerWidget::leaveEvent(QEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    if (m_config.clickThrough && !m_editController.isEditMode()) {
        event->ignore();
        return;
    }

    Q_UNUSED(event)
    handleMouseTrigger(MouseTrigger::MouseLeave);

    // 恢复原始透明度
    if (m_opacityAnimation->state() != QAbstractAnimation::Running) {
        m_opacityAnimation->setStartValue(windowOpacity());
        m_opacityAnimation->setEndValue(m_config.opacity);
        m_opacityAnimation->start();
    }
}

void StickerWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // 如果启用了点击穿透，不显示右键菜单
    if (m_config.clickThrough && !m_editController.isEditMode()) {
        event->ignore();
        return;
    }

    m_menuController.exec(event->globalPos());
}

void StickerWidget::handleMouseTrigger(MouseTrigger trigger)
{
    m_eventController.handleTrigger(trigger);
}

void StickerWidget::setupContextMenu()
{
    StickerContextMenuController::Callbacks callbacks;
    callbacks.editSticker = [this]() { onEditSticker(); };
    callbacks.toggleEditMode = [this]() { onToggleEditMode(); };
    callbacks.toggleMode = [this]() { onToggleMode(); };
    callbacks.toggleDrag = [this]() { onToggleDrag(); };
    callbacks.toggleClickThrough = [this]() { onToggleClickThrough(); };
    callbacks.deleteSticker = [this]() { onDeleteSticker(); };
    m_menuController.setCallbacks(callbacks);
    updateContextMenuState();
}

void StickerWidget::updateContextMenuState()
{
    m_menuController.updateState(m_config.isDesktopMode,
                                m_config.allowDrag,
                                m_config.clickThrough,
                                m_editController.isEditMode());
}

void StickerWidget::onAnimationTimer()
{
    if (!m_initialized) {
        return;
    }

    m_animationAngle += 0.05;
    if (m_animationAngle >= 2 * M_PI) {
        m_animationAngle = 0;
    }

    // 只有默认贴纸才有动画
    if (m_config.imagePath.isEmpty()) {
        update();
    }
}

void StickerWidget::onEditSticker()
{
    emit editRequested(m_config.id);
}

void StickerWidget::onDeleteSticker()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "删除贴纸",
        QString("确定要删除贴纸 '%1' 吗？").arg(m_config.name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        emit deleteRequested(m_config.id);
    }
}

void StickerWidget::onToggleMode()
{
    m_config.isDesktopMode = !m_config.isDesktopMode;
    m_editController.applyWindowFlags(m_config.isDesktopMode, m_initialized);
    updateContextMenuState();
    emit configChanged(m_config);

    QString mode = m_config.isDesktopMode ? "桌面模式" : "置顶模式";
    qDebug() << "贴纸" << m_config.id << "切换到" << mode;
}

void StickerWidget::onToggleDrag()
{
    m_config.allowDrag = !m_config.allowDrag;
    updateContextMenuState();
    emit configChanged(m_config);

    QString status = m_config.allowDrag ? "允许拖动" : "禁止拖动";
    qDebug() << "贴纸" << m_config.id << status;
}

void StickerWidget::onToggleClickThrough()
{
    m_config.clickThrough = !m_config.clickThrough;
    updateClickThrough();
    updateContextMenuState();
    emit configChanged(m_config);

    QString status = m_config.clickThrough ? "点击穿透已启用" : "点击穿透已禁用";
    qDebug() << "贴纸" << m_config.id << status;
}

void StickerWidget::setEditMode(bool enabled)
{
    if (m_editController.isEditMode() == enabled) {
        return;
    }

    m_interactionController.reset();
    m_editController.setEditMode(enabled, m_config.isDesktopMode, m_initialized);
    updateContextMenuState();
    update();
}

void StickerWidget::onToggleEditMode()
{
    setEditMode(!m_editController.isEditMode());
}

StickerConfig StickerWidget::getConfig() const
{
    StickerConfig config = m_config;
    config.position = pos();
    config.size = size();
    config.visible = isVisible();
    return config;
}

void StickerWidget::updateConfig(const StickerConfig &config)
{
    QSize oldBaseSize = m_config.size;
    m_config = config;
    m_eventController.setEvents(&m_config.events);

    // 更新位置和大小
    move(config.position);
    if (config.size != oldBaseSize) {
        setFixedSize(m_config.size);
    }

    // 更新图像
    if (!config.imagePath.isEmpty() && QFileInfo::exists(config.imagePath)) {
        loadStickerImage(config.imagePath);
    } else {
        createDefaultSticker();
    }

    updateTransformedWindowSize(ResizeAnchor::KeepTopLeft);
    applyMask();

    // 重新应用窗口设置
    m_editController.applyWindowFlags(m_config.isDesktopMode, m_initialized);
    updateClickThrough();
    setOpacity(config.opacity);
    setVisible(config.visible);

    // 更新右键菜单状态
    updateContextMenuState();

    qDebug() << "更新贴纸配置:" << config.id;
}

void StickerWidget::setOpacity(double opacity)
{
    m_config.opacity = qBound(0.1, opacity, 1.0);
    setWindowOpacity(m_config.opacity);
}

void StickerWidget::setDesktopMode(bool isDesktop)
{
    if (m_config.isDesktopMode != isDesktop) {
        m_config.isDesktopMode = isDesktop;
        m_editController.applyWindowFlags(m_config.isDesktopMode, m_initialized);
        updateContextMenuState();
        emit configChanged(m_config);
    }
}

void StickerWidget::setAllowDrag(bool allowDrag)
{
    if (m_config.allowDrag != allowDrag) {
        m_config.allowDrag = allowDrag;
        updateContextMenuState();
        emit configChanged(m_config);
    }
}

void StickerWidget::setClickThrough(bool clickThrough)
{
    if (m_config.clickThrough != clickThrough) {
        m_config.clickThrough = clickThrough;
        updateClickThrough();
        updateContextMenuState();
        emit configChanged(m_config);
    }
}

void StickerWidget::setVisible(bool visible)
{
    m_config.visible = visible;
    QWidget::setVisible(visible);
    emit configChanged(m_config);
}

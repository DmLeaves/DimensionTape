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
    , m_eventHandler(new EventHandler(this))
    , m_image(600)
    , m_renderer(&m_image)
    , m_dragging(false)
    , m_initialized(false)
    , m_animationAngle(0)
    , m_editMode(false)
    , m_rotating(false)
    , m_rotateStartAngle(0.0)
    , m_rotateStartRotation(0.0)
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

    // 连接事件处理器信号
    connect(m_eventHandler, &EventHandler::eventExecuted, [this](const QString &message) {
        qDebug() << "贴纸事件执行成功:" << message;
    });

    connect(m_eventHandler, &EventHandler::eventFailed, [this](const QString &error) {
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
    updateWindowFlags();

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

    if (m_editMode) {
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

void StickerWidget::updateWindowFlags()
{
    if (m_editMode) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_ShowWithoutActivating, false);
        setFocusPolicy(Qt::StrongFocus);
    } else if (m_config.isDesktopMode) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnBottomHint);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
        setFocusPolicy(Qt::NoFocus);
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_ShowWithoutActivating, false);
        setFocusPolicy(Qt::StrongFocus);
    }

    setAttribute(Qt::WA_TranslucentBackground, true);

    if (m_initialized) {
        show(); // 重新显示窗口以应用新的标志
        if (m_editMode) {
            raise();
            activateWindow();
        }
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

    if (m_editMode) {
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
    if (m_config.clickThrough && !m_editMode) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (!m_editMode) {
            handleMouseTrigger(MouseTrigger::LeftClick);
        }

        // 左键拖动（如果允许拖动）
        if (m_editMode && (event->modifiers() & Qt::ShiftModifier)) {
            m_rotating = true;
            m_dragging = false;
            m_rotateStartAngle = angleFromCenter(event->pos());
            m_rotateStartRotation = m_config.transform.rotation;
            event->accept();
            return;
        }

        if (m_config.allowDrag || m_editMode) {
            m_dragging = true;
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        }
    } else if (event->button() == Qt::RightButton) {
        if (!m_editMode) {
            handleMouseTrigger(MouseTrigger::RightClick);
        }
        // 右键不再用于拖动，只触发右键事件
    }

    update();
}

void StickerWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    if (m_config.clickThrough && !m_editMode) {
        event->ignore();
        return;
    }

    if (m_rotating) {
        double angle = angleFromCenter(event->pos());
        double delta = qRadiansToDegrees(angle - m_rotateStartAngle);
        m_config.transform.rotation = m_rotateStartRotation + delta;
        updateTransformedWindowSize(ResizeAnchor::KeepCenter);
        applyMask();
        update();
        return;
    }

    if (m_dragging && (m_config.allowDrag || m_editMode)) {
        QPoint newPos = event->globalPos() - m_dragPosition;
        if (newPos != m_config.position) {
            move(newPos);
            m_config.position = newPos;
            emit configChanged(m_config);
        }
    }
}

void StickerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    if (m_config.clickThrough && !m_editMode) {
        event->ignore();
        return;
    }

    if (m_rotating) {
        m_rotating = false;
        emit configChanged(m_config);
        return;
    }

    if (m_dragging) {
        m_dragging = false;
        update();
        emit configChanged(m_config);
    }
}

void StickerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    if (m_config.clickThrough && !m_editMode) {
        event->ignore();
        return;
    }

    if (m_editMode) {
        event->ignore();
        return;
    }

    Q_UNUSED(event)
    handleMouseTrigger(MouseTrigger::DoubleClick);
}

void StickerWidget::wheelEvent(QWheelEvent *event)
{
    // 如果启用了点击穿透，不处理鼠标事件
    if (m_config.clickThrough && !m_editMode) {
        event->ignore();
        return;
    }

    if (m_editMode && (event->modifiers() & Qt::ControlModifier)) {
        double delta = event->angleDelta().y() / 120.0;
        m_config.transform.rotation += delta * 5.0;
        updateTransformedWindowSize(ResizeAnchor::KeepCenter);
        applyMask();
        update();
        emit configChanged(m_config);
        return;
    }

    if (m_editMode) {
        double delta = event->angleDelta().y() / 120.0;
        double factor = 1.0 + delta * 0.05;
        m_config.transform.scaleX = qBound(0.1, m_config.transform.scaleX * factor, 5.0);
        m_config.transform.scaleY = qBound(0.1, m_config.transform.scaleY * factor, 5.0);
        updateTransformedWindowSize(ResizeAnchor::KeepCenter);
        applyMask();
        update();
        emit configChanged(m_config);
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
    if (m_config.clickThrough && !m_editMode) {
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
    if (m_config.clickThrough && !m_editMode) {
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
    if (m_config.clickThrough && !m_editMode) {
        event->ignore();
        return;
    }

    m_contextMenu->exec(event->globalPos());
}

void StickerWidget::handleMouseTrigger(MouseTrigger trigger)
{
    for (const auto &event : m_config.events) {
        if (event.trigger == trigger && event.enabled) {
            m_eventHandler->executeEvent(event);
        }
    }
}

void StickerWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    m_editAction = m_contextMenu->addAction("编辑贴纸", this, &StickerWidget::onEditSticker);
    m_editModeAction = m_contextMenu->addAction("编辑模式", this, &StickerWidget::onToggleEditMode);

    m_contextMenu->addSeparator();

    m_toggleModeAction = m_contextMenu->addAction("切换模式", this, &StickerWidget::onToggleMode);
    m_toggleDragAction = m_contextMenu->addAction("允许拖动", this, &StickerWidget::onToggleDrag);
    m_toggleClickThroughAction = m_contextMenu->addAction("点击穿透", this, &StickerWidget::onToggleClickThrough);

    m_contextMenu->addSeparator();

    m_deleteAction = m_contextMenu->addAction("删除贴纸", this, &StickerWidget::onDeleteSticker);

    // 设置复选框状态
    m_toggleDragAction->setCheckable(true);
    m_toggleClickThroughAction->setCheckable(true);
    m_editModeAction->setCheckable(true);

    // 更新菜单状态
    updateContextMenuState();
}

void StickerWidget::updateContextMenuState()
{
    if (!m_contextMenu) return;

    // 更新模式切换按钮文本
    QString modeText = m_config.isDesktopMode ? "切换到置顶模式" : "切换到桌面模式";
    m_toggleModeAction->setText(modeText);

    // 更新复选框状态
    m_toggleDragAction->setChecked(m_config.allowDrag);
    m_toggleClickThroughAction->setChecked(m_config.clickThrough);
    m_editModeAction->setChecked(m_editMode);
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
    updateWindowFlags();
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
    if (m_editMode == enabled) {
        return;
    }

    m_editMode = enabled;
    m_rotating = false;
    updateWindowFlags();
    updateContextMenuState();
    update();
}

void StickerWidget::onToggleEditMode()
{
    setEditMode(!m_editMode);
}

double StickerWidget::angleFromCenter(const QPoint &point) const
{
    QPointF center = rect().center();
    QPointF delta = point - center;
    return qAtan2(delta.y(), delta.x());
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
    updateWindowFlags();
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
        updateWindowFlags();
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

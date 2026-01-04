#include "stickercontextmenucontroller.h"
#include <QWidget>
#include <utility>

StickerContextMenuController::StickerContextMenuController(QObject *parent)
    : QObject(parent)
    , m_menu(new QMenu(qobject_cast<QWidget *>(parent)))
    , m_editAction(nullptr)
    , m_editModeAction(nullptr)
    , m_toggleModeAction(nullptr)
    , m_toggleDragAction(nullptr)
    , m_toggleClickThroughAction(nullptr)
    , m_deleteAction(nullptr)
{
    setupMenu();
}

void StickerContextMenuController::setCallbacks(Callbacks callbacks)
{
    m_callbacks = std::move(callbacks);

    connectAction(m_editAction, m_callbacks.editSticker);
    connectAction(m_editModeAction, m_callbacks.toggleEditMode);
    connectAction(m_toggleModeAction, m_callbacks.toggleMode);
    connectAction(m_toggleDragAction, m_callbacks.toggleDrag);
    connectAction(m_toggleClickThroughAction, m_callbacks.toggleClickThrough);
    connectAction(m_deleteAction, m_callbacks.deleteSticker);
}

void StickerContextMenuController::updateState(bool isDesktopMode, bool allowDrag, bool clickThrough, bool editMode)
{
    if (!m_menu) {
        return;
    }

    QString modeText = isDesktopMode ? "切换到置顶模式" : "切换到桌面模式";
    if (m_toggleModeAction) {
        m_toggleModeAction->setText(modeText);
    }
    if (m_toggleDragAction) {
        m_toggleDragAction->setChecked(allowDrag);
    }
    if (m_toggleClickThroughAction) {
        m_toggleClickThroughAction->setChecked(clickThrough);
    }
    if (m_editModeAction) {
        m_editModeAction->setChecked(editMode);
    }
}

void StickerContextMenuController::exec(const QPoint &globalPos)
{
    if (!m_menu) {
        return;
    }
    m_menu->exec(globalPos);
}

void StickerContextMenuController::setupMenu()
{
    m_editAction = m_menu->addAction("编辑贴纸");
    m_editModeAction = m_menu->addAction("编辑模式");

    m_menu->addSeparator();

    m_toggleModeAction = m_menu->addAction("切换模式");
    m_toggleDragAction = m_menu->addAction("允许拖动");
    m_toggleClickThroughAction = m_menu->addAction("点击穿透");

    m_menu->addSeparator();

    m_deleteAction = m_menu->addAction("删除贴纸");

    m_toggleDragAction->setCheckable(true);
    m_toggleClickThroughAction->setCheckable(true);
    m_editModeAction->setCheckable(true);
}

void StickerContextMenuController::connectAction(QAction *action, const std::function<void()> &callback)
{
    if (!action) {
        return;
    }
    action->disconnect(this);
    connect(action, &QAction::triggered, this, [callback]() {
        if (callback) {
            callback();
        }
    });
}

#include "stickereditcontroller.h"

StickerEditController::StickerEditController(QWidget *host, QObject *parent)
    : QObject(parent)
    , m_host(host)
    , m_editMode(false)
{
}

bool StickerEditController::isEditMode() const
{
    return m_editMode;
}

void StickerEditController::setEditMode(bool enabled, bool isDesktopMode, bool initialized)
{
    if (m_editMode == enabled) {
        return;
    }
    m_editMode = enabled;
    applyWindowFlags(isDesktopMode, initialized);
    emit editModeChanged(m_editMode);
}

void StickerEditController::toggleEditMode(bool isDesktopMode, bool initialized)
{
    setEditMode(!m_editMode, isDesktopMode, initialized);
}

void StickerEditController::applyWindowFlags(bool isDesktopMode, bool initialized)
{
    if (!m_host) {
        return;
    }

    if (m_editMode) {
        m_host->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        m_host->setAttribute(Qt::WA_ShowWithoutActivating, false);
        m_host->setFocusPolicy(Qt::StrongFocus);
    } else if (isDesktopMode) {
        m_host->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnBottomHint);
        m_host->setAttribute(Qt::WA_ShowWithoutActivating, true);
        m_host->setFocusPolicy(Qt::NoFocus);
    } else {
        m_host->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        m_host->setAttribute(Qt::WA_ShowWithoutActivating, false);
        m_host->setFocusPolicy(Qt::StrongFocus);
    }

    m_host->setAttribute(Qt::WA_TranslucentBackground, true);

    if (initialized) {
        m_host->show();
        if (m_editMode) {
            m_host->raise();
            m_host->activateWindow();
        }
    }
}

#include "TrayIcon.h"
#include <QApplication>
#include <QStyle>

TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
{
    // 使用系统默认图标，如果没有资源文件
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
    setIcon(icon);
    setToolTip("桌面贴纸管理器");

    setupMenu();

    connect(this, &QSystemTrayIcon::activated,
            this, &TrayIcon::onTrayActivated);
}

void TrayIcon::setupMenu()
{
    m_trayMenu = new QMenu();

    m_showAction = m_trayMenu->addAction("显示主窗口");
    connect(m_showAction, &QAction::triggered, this, &TrayIcon::onShowMainWindow);

    m_trayMenu->addSeparator();

    m_exitAction = m_trayMenu->addAction("退出程序");
    connect(m_exitAction, &QAction::triggered, this, &TrayIcon::onExitApplication);

    setContextMenu(m_trayMenu);
}

void TrayIcon::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        emit showMainWindow();
    }
}

void TrayIcon::onShowMainWindow()
{
    emit showMainWindow();
}

void TrayIcon::onExitApplication()
{
    emit exitApplication();
}

#include "ApplicationManager.h"
#include <QApplication>
#include <QDebug>

ApplicationManager::ApplicationManager(QObject *parent)
    : QObject(parent)
    , m_stickerManager(nullptr)
    , m_trayIcon(nullptr)
    , m_mainWindow(nullptr)
{
}

void ApplicationManager::initialize()
{
    // 创建核心组件
    m_stickerManager = StickerManager::instance();
    m_trayIcon = new TrayIcon(this);
    m_mainWindow = new MainWindow();

    // 建立组件间的连接
    connect(m_trayIcon, &TrayIcon::showMainWindow,
            m_mainWindow, &MainWindow::showAndRaise);
    connect(m_trayIcon, &TrayIcon::exitApplication,
            this, &ApplicationManager::exitApplication);

    // 主窗口和贴纸管理器的连接 - 修复信号连接
    connect(m_mainWindow, &MainWindow::createSticker,
            m_stickerManager, &StickerManager::createStickerInternal);
    connect(m_mainWindow, &MainWindow::deleteSticker,
            m_stickerManager, &StickerManager::deleteSticker);
    connect(m_mainWindow, &MainWindow::editSticker,
            m_stickerManager, &StickerManager::editStickerFromMainWindow);
    connect(m_mainWindow, &MainWindow::editStickerWithConfig,
            m_stickerManager, &StickerManager::editSticker);
    connect(m_mainWindow, &MainWindow::loadStickerConfig,
            m_stickerManager, static_cast<bool(StickerManager::*)()>(&StickerManager::loadConfig));
    connect(m_mainWindow, &MainWindow::saveStickerConfig,
            m_stickerManager, static_cast<bool(StickerManager::*)()>(&StickerManager::saveConfig));
    connect(m_mainWindow, &MainWindow::lockFollowTarget,
            m_stickerManager, &StickerManager::lockStickerToWindow);


    // 贴纸管理器到主窗口的连接
    connect(m_stickerManager, &StickerManager::stickerCreated,
            m_mainWindow, &MainWindow::onStickerCreated);
    connect(m_stickerManager, &StickerManager::stickerDeleted,
            m_mainWindow, &MainWindow::onStickerDeleted);
    connect(m_stickerManager, &StickerManager::stickerConfigChanged,
            m_mainWindow, &MainWindow::onStickerConfigChanged);

    // 配置更新连接
    connect(m_mainWindow, &MainWindow::requestStickerConfigs,
            m_stickerManager, &StickerManager::onConfigsRequested);
    connect(m_stickerManager, &StickerManager::stickerConfigsUpdated,
            m_mainWindow, &MainWindow::onStickerConfigsUpdated);

    // 主窗口退出请求
    connect(m_mainWindow, &MainWindow::exitRequested,
            this, &ApplicationManager::exitApplication);

    // 初始化系统托盘
    m_trayIcon->show();

    // 显示主窗口
    m_mainWindow->show();

    qDebug() << "桌面贴纸管理器启动完成";

    // 延迟加载配置并更新界面
    QTimer::singleShot(100, [this]() {
        if (m_mainWindow && m_stickerManager) {
            QList<StickerConfig> configs = m_stickerManager->getAllConfigs();
            m_mainWindow->onStickerConfigsUpdated(configs);
        }
    });
}

void ApplicationManager::exitApplication()
{
    qDebug() << "开始退出应用程序...";

    // 1. 隐藏系统托盘图标
    if (m_trayIcon) {
        m_trayIcon->hide();
    }

    // 2. 关闭主窗口
    if (m_mainWindow) {
        m_mainWindow->forceClose();
    }

    // 3. 清理所有贴纸
    if (m_stickerManager) {
        m_stickerManager->cleanup();
    }

    qDebug() << "所有组件已清理完成";

    // 4. 退出应用程序
    QApplication::quit();
}

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include "ApplicationManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Desktop Sticker Manager");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("StickerStudio");
    app.setQuitOnLastWindowClosed(false);

    // 创建应用程序数据目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);

    qDebug() << "程序启动，数据目录:" << appDataPath;

    // 创建应用程序管理器
    ApplicationManager appManager;
    appManager.initialize();

    return app.exec();
}

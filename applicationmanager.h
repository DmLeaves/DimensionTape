#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>
#include "MainWindow.h"
#include "TrayIcon.h"
#include "StickerManager.h"

class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationManager(QObject *parent = nullptr);
    void initialize();

public slots:
    void exitApplication();

private:
    StickerManager *m_stickerManager;
    TrayIcon *m_trayIcon;
    MainWindow *m_mainWindow;
};

#endif // APPLICATIONMANAGER_H

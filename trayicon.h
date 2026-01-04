#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit TrayIcon(QObject *parent = nullptr);

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMainWindow();
    void onExitApplication();

signals:
    void showMainWindow();
    void exitApplication();

private:
    void setupMenu();

    QMenu *m_trayMenu;
    QAction *m_showAction;
    QAction *m_exitAction;
};

#endif // TRAYICON_H

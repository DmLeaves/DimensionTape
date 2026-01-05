#ifndef STICKEREDITCONTROLLER_H
#define STICKEREDITCONTROLLER_H

#include <QObject>
#include <QWidget>

class StickerEditController : public QObject
{
    Q_OBJECT

public:
    explicit StickerEditController(QWidget *host, QObject *parent = nullptr);

    bool isEditMode() const;
    void setEditMode(bool enabled, bool isDesktopMode, bool isFollowMode, bool initialized);
    void toggleEditMode(bool isDesktopMode, bool isFollowMode, bool initialized);
    void applyWindowFlags(bool isDesktopMode, bool isFollowMode, bool initialized);

signals:
    void editModeChanged(bool enabled);

private:
    QWidget *m_host;
    bool m_editMode;
};

#endif // STICKEREDITCONTROLLER_H

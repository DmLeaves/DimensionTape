#ifndef STICKERCONTEXTMENUCONTROLLER_H
#define STICKERCONTEXTMENUCONTROLLER_H

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QPoint>
#include <functional>

class StickerContextMenuController : public QObject
{
    Q_OBJECT

public:
    struct Callbacks {
        std::function<void()> editSticker;
        std::function<void()> toggleEditMode;
        std::function<void()> toggleMode;
        std::function<void()> toggleDrag;
        std::function<void()> toggleClickThrough;
        std::function<void()> deleteSticker;
    };

    explicit StickerContextMenuController(QObject *parent = nullptr);

    void setCallbacks(Callbacks callbacks);
    void updateState(bool isDesktopMode, bool allowDrag, bool clickThrough, bool editMode);
    void exec(const QPoint &globalPos);

private:
    void setupMenu();
    void connectAction(QAction *action, const std::function<void()> &callback);

    QMenu *m_menu;
    QAction *m_editAction;
    QAction *m_editModeAction;
    QAction *m_toggleModeAction;
    QAction *m_toggleDragAction;
    QAction *m_toggleClickThroughAction;
    QAction *m_deleteAction;
    Callbacks m_callbacks;
};

#endif // STICKERCONTEXTMENUCONTROLLER_H

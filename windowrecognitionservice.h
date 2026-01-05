#ifndef WINDOWRECOGNITIONSERVICE_H
#define WINDOWRECOGNITIONSERVICE_H

#include <QObject>
#include <QList>
#include <QRectF>
#include <QString>
#include <QtGlobal>

using WindowHandle = quintptr;

struct WindowInfo {
    WindowHandle handle = 0;
    QString title;
    QString className;
    QString processName;
    QRectF rect;
    bool visible = false;
    bool minimized = false;
    bool topMost = false;
};

class WindowRecognitionService : public QObject
{
    Q_OBJECT

public:
    explicit WindowRecognitionService(QObject *parent = nullptr);

    QList<WindowInfo> listWindows(bool visibleOnly = true) const;
    WindowInfo queryWindow(WindowHandle handle) const;
    bool isWindowValid(WindowHandle handle) const;

    static qreal dpiScaleForWindow(WindowHandle handle);
    static QRectF rectPhysicalToLogical(const QRect &rect, WindowHandle handle);
};

#endif // WINDOWRECOGNITIONSERVICE_H
